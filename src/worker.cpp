#include <ctz/scheduler.h>
#include <ctz/worker.h>

NAMESPACE_CTZ_BEGIN

thread_local Worker* Worker::current{nullptr};

Worker::Worker(Scheduler* scheduler, Mode mode, uint32_t id)
    : id(id),
      mode(mode),
      scheduler(scheduler),
      work(),
      idleFibers() {}

void Worker::start() {
    switch (mode) {
        case Mode::MultiThreaded: {     // 创建新线程。
            auto& affinityPolicy = scheduler->cfg.workerThread.affinityPolicy;
            auto affinity = affinityPolicy->get(id);
            thread = Thread(std::move(affinity), [=] {
                if (const auto& initFunc = scheduler->cfg.workerThread.initializer) {   // 如果有需要先做的事。
                    initFunc(id);
                }

                Scheduler::setBound(scheduler);

                Worker::current = this;
                mainFiber = Fiber::createFromCurrentThread(0);
                currentFiber = mainFiber.get();

                {
                    std::unique_lock<std::mutex> ul(work.mutex);
                    run();
                }
                mainFiber.reset();
                Worker::current = nullptr;
            });
            break;
        }
        case Mode::SingleThreaded: {    // 创建一个纤程绑在 currentFiber 上。不会主动运行任务，直到调用 suspend()。
            Worker::current = this;
            mainFiber = Fiber::createFromCurrentThread(0);
            currentFiber = mainFiber.get();
            break;
        }
        default:
            ciel::unreachable();
    }
}

void Worker::stop() {
    switch (mode) {
        case Mode::MultiThreaded: {
            enqueue(Task([this] { shutdown = true; }, Task::Flags::SameThread));
            thread.join();
            break;
        }
        case Mode::SingleThreaded: {
            std::unique_lock<std::mutex> ul(work.mutex);
            shutdown = true;
            runUntilShutdown();
            Worker::current = nullptr;
            break;
        }
        default:
            ciel::unreachable();
    }
}

// TODO: waitLock 传入时已经上锁了
bool Worker::wait(std::mutex& waitLock, const TimePoint* timeout, const Predicate& pred) {
    while (!pred()) {
        // Lock the work mutex to call suspend().
        work.mutex.lock();

        // Unlock the wait mutex with the work mutex lock held.
        // Order is important here as we need to ensure that the fiber is not
        // enqueued (via Fiber::notify()) between the waitLock.unlock() and fiber
        // switch, otherwise the Fiber::notify() call may be ignored and the fiber
        // is never woken.
        waitLock.unlock();

        // suspend the fiber.
        suspend(timeout);

        // Fiber resumed. We don't need the work mutex locked any more.
        work.mutex.unlock();

        // Re-lock to either return due to timeout, or call pred().
        waitLock.lock();

        // Check timeout.
        if (timeout != nullptr && std::chrono::system_clock::now() >= *timeout) {
            return false;
        }

        // Spurious wake up. Spin again.
    }
    return true;
}

bool Worker::wait(const TimePoint* timeout) {
    {
        std::unique_lock<std::mutex> ul(work.mutex);
        suspend(timeout);
    }
    return timeout == nullptr || std::chrono::system_clock::now() < *timeout;
}

void Worker::suspend(const TimePoint* timeout) {
    // Current fiber is yielding as it is blocked.
    if (timeout != nullptr) {
        setFiberState(currentFiber, Fiber::State::Waiting);
        work.waiting.add(*timeout, currentFiber);
    } else {
        setFiberState(currentFiber, Fiber::State::Yielded);
    }

    // First wait until there's something else this worker can do.
    waitForWork();

    ++work.numBlockedFibers;

    if (!work.fibers.empty()) {
        // There's another fiber that has become unblocked, resume that.
        --work.num;

        auto to = std::move(work.fibers.front());
        work.fibers.pop_front();

        CTZ_ASSERT(to->state == Fiber::State::Queued, "");
        switchToFiber(to);
    } else if (!idleFibers.empty()) {
        // There's an old fiber we can reuse, resume that.
        auto it = idleFibers.begin();
        auto to = std::move(*it);
        idleFibers.erase(it);

        CTZ_ASSERT(to->state == Fiber::State::Idle, "");
        switchToFiber(to);
    } else {
        // Tasks to process and no existing fibers to resume.
        // Spawn a new fiber.
        switchToFiber(createWorkerFiber());
    }

    --work.numBlockedFibers;

    setFiberState(currentFiber, Fiber::State::Running);
}

void Worker::enqueue(Fiber* fiber) {
    bool notify = false;
    {
        std::unique_lock<std::mutex> ul(work.mutex);

        switch (fiber->state) {
            case Fiber::State::Running:
            case Fiber::State::Queued:
                return;  // Nothing to do here - task is already queued or running.
            case Fiber::State::Waiting:
                work.waiting.erase(fiber);
                break;
            case Fiber::State::Idle:
            case Fiber::State::Yielded:
                break;
        }
        notify = work.notifyAdded;
        work.fibers.push_back(fiber);

        CTZ_ASSERT(!work.waiting.contains(fiber), "fiber is unexpectedly in the waiting list");

        setFiberState(fiber, Fiber::State::Queued);
        ++work.num;
    }

    if (notify) {
        work.added.notify_one();
    }
}

void Worker::enqueue(Task&& task) {
    work.mutex.lock();
    enqueueAndUnlock(std::move(task));
}

bool Worker::tryLock() {
    return work.mutex.try_lock();
}

void Worker::enqueueAndUnlock(Task&& task) {
    auto notify = work.notifyAdded;
    work.tasks.push_back(std::move(task));
    ++work.num;
    work.mutex.unlock();
    if (notify) {
        work.added.notify_one();
    }
}

void Worker::runUntilShutdown() {
    while (!shutdown || work.num > 0 || work.numBlockedFibers > 0U) {
        waitForWork();
        runUntilIdle();
    }
}

bool Worker::steal(Task& out) {
    if (work.num.load() == 0) {
        return false;
    }

    if (!work.mutex.try_lock()) {
        return false;
    }

    if (work.tasks.empty() || work.tasks.front().is(Task::Flags::SameThread)) {
        work.mutex.unlock();
        return false;
    }

    --work.num;
    out = std::move(work.tasks.front());
    work.tasks.pop_front();
    work.mutex.unlock();
    return true;
}

Worker* Worker::getCurrent() noexcept {
    return current;
}

Fiber* Worker::getCurrentFiber() const noexcept {
    return currentFiber;
}

void Worker::run() {
    if (mode == Mode::MultiThreaded) {
        // This is the entry point for a multi-threaded worker.
        // Start with a regular condition-variable wait for work. This avoids
        // starting the thread with a spinForWorkAndLock().
        work.wait([this]() {
            return work.num > 0 || work.waiting || shutdown;
        });
    }

    CTZ_ASSERT(currentFiber->state == Fiber::State::Running, "");

    runUntilShutdown();
    switchToFiber(mainFiber.get());
}

Fiber* Worker::createWorkerFiber() {
    auto fiberId = static_cast<uint32_t>(workerFibers.size() + 1);

    auto fiber = Fiber::create(fiberId, scheduler->cfg.fiberStackSize, [&]() { run(); });
    auto ptr = fiber.get();
    workerFibers.emplace_back(std::move(fiber));
    return ptr;
}

void Worker::switchToFiber(Fiber* to) {
    CTZ_ASSERT(to == mainFiber.get() || idleFibers.count(to) == 0, "switching to idle fiber");

    auto from = currentFiber;
    currentFiber = to;
    from->switchTo(to);
}

void Worker::runUntilIdle() {
    CTZ_ASSERT(currentFiber->state == Fiber::State::Running, "");
    CTZ_ASSERT(work.num == work.fibers.size() + work.tasks.size(), "work.num out of sync");

    while (!work.fibers.empty() || !work.tasks.empty()) {
        // Note: we cannot take and store on the stack more than a single fiber
        // or task at a time, as the Fiber may yield and these items may get
        // held on suspended fiber stack.

        while (!work.fibers.empty()) {
            --work.num;

            auto fiber = std::move(work.fibers.front());
            work.fibers.pop_front();

            // Sanity checks,
            CTZ_ASSERT(idleFibers.count(fiber) == 0, "dequeued fiber is idle");
            CTZ_ASSERT(fiber != currentFiber, "dequeued fiber is currently running");
            CTZ_ASSERT(fiber->state == Fiber::State::Queued, "");

            setFiberState(currentFiber, Fiber::State::Idle);
            auto added = idleFibers.emplace(currentFiber).second;
            (void)added;

            CTZ_ASSERT(added, "fiber already idle");

            switchToFiber(fiber);
            setFiberState(currentFiber, Fiber::State::Running);
        }

        if (!work.tasks.empty()) {
            --work.num;

            auto task = std::move(work.tasks.front());
            work.tasks.pop_front();

            work.mutex.unlock();

            // Run the task.
            task();

            // std::function<> can carry arguments with complex destructors.
            // Ensure these are destructed outside of the lock.
            task = Task();

            work.mutex.lock();
        }
    }
}

void Worker::waitForWork() {
    CTZ_ASSERT(work.num == work.fibers.size() + work.tasks.size(), "work.num out of sync");

    if (work.num > 0) {
        return;
    }

    if (mode == Mode::MultiThreaded) {
        scheduler->onBeginSpinning(id);
        work.mutex.unlock();
        spinForWorkAndLock();
    }

    work.wait([this]() {
        return work.num > 0 || (shutdown && work.numBlockedFibers == 0U);
    });

    if (work.waiting) {
        enqueueFiberTimeouts();
    }
}

void Worker::spinForWorkAndLock() {
    Task stolen;

    constexpr auto duration = std::chrono::milliseconds(1);
    auto start = std::chrono::high_resolution_clock::now();
    while (std::chrono::high_resolution_clock::now() - start < duration) {
        for (int i = 0; i < 256; ++i) {     // Empirically picked magic number!
            nop(); nop(); nop(); nop(); nop(); nop(); nop(); nop();
            nop(); nop(); nop(); nop(); nop(); nop(); nop(); nop();
            nop(); nop(); nop(); nop(); nop(); nop(); nop(); nop();
            nop(); nop(); nop(); nop(); nop(); nop(); nop(); nop();

            if (work.num > 0) {
                work.mutex.lock();
                if (work.num > 0) {
                    return;
                }
                else {
                    // Our new task was stolen by another worker. Keep spinning.
                    work.mutex.unlock();
                }
            }
        }

        if (scheduler->stealWork(this, rng(), stolen)) {
            work.mutex.lock();
            work.tasks.emplace_back(std::move(stolen));
            ++work.num;
            return;
        }

        std::this_thread::yield();
    }
    work.mutex.lock();
}

void Worker::enqueueFiberTimeouts() {
    auto now = std::chrono::system_clock::now();
    while (auto fiber = work.waiting.take(now)) {
        setFiberState(fiber, Fiber::State::Queued);
        work.fibers.push_back(fiber);
        ++work.num;
    }
}

void Worker::setFiberState(Fiber* fiber, Fiber::State to) const noexcept {
    fiber->state = to;
}

NAMESPACE_CTZ_END