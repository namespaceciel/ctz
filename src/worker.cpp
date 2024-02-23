#include <ctz/scheduler.h>
#include <ctz/worker.h>

#include <unistd.h>

NAMESPACE_CTZ_BEGIN

// numLogicalCPUs()
#if defined(_WIN32)
CIEL_NODISCARD unsigned int numLogicalCPUs() noexcept {
    unsigned int count = 0;
    const auto& groups = getProcessorGroups();

    for (size_t groupIdx = 0; groupIdx < groups.count; ++groupIdx) {
        const auto& group = groups.groups[groupIdx];
        count += group.count;
    }

    return count;
}

#else
CIEL_NODISCARD unsigned int numLogicalCPUs() noexcept {
    return static_cast<unsigned int>(sysconf(_SC_NPROCESSORS_ONLN));
}

#endif

thread_local Worker* Worker::current = nullptr;

Worker::Worker()
    : scheduler(Scheduler::get()),
      mainFiber(Fiber::createFromCurrentThread(this)),
      currentFiber(Fiber::create(this, scheduler->config.fiberStackSize, [this] {
          run();
      })) {}

Worker::~Worker() {
    if (thread.joinable()) {
        stop();
    }
}

void Worker::enqueue(const std::function<void()>& newTask) {
    {
        std::lock_guard<std::mutex> lg(mutex);
        queuedTasks.push(newTask);
    }

    cv.notify_one();
}

void Worker::enqueue(std::function<void()>&& newTask) {
    {
        std::lock_guard<std::mutex> lg(mutex);
        queuedTasks.push(std::move(newTask));
    }

    cv.notify_one();
}

void Worker::enqueue(std::unique_ptr<Fiber>&& resumedTask) {
    {
        std::lock_guard<std::mutex> lg(mutex);
        queuedFibers.push(std::move(resumedTask));
    }

    cv.notify_one();
}

void Worker::start() {
    CTZ_ASSERT(!thread.joinable(), "Worker::start() on a joinable thread");

    thread = std::thread([this] {
        current = this;     // bind thread_local worker* to this thread

        mainFiber->switchTo(currentFiber.get());
    });
}

void Worker::stop() noexcept {
    enqueue([this] {
        switchToFiber(std::move(mainFiber));    // mainFiber is done here...
    });

    thread.join();
}

void Worker::switchToFiber(std::unique_ptr<Fiber>&& to) noexcept {
    auto from = std::move(currentFiber);
    currentFiber = std::move(to);
    from->switchTo(currentFiber.get());
}

[[noreturn]] void Worker::run() noexcept {
    while (true) {
        // Firstly complete all fibers.
        if (!queuedFibers.empty()) {        // Fiber can't be stolen.
            mutex.lock();

            std::unique_ptr<Fiber> nextFiber = std::move(queuedFibers.front());
            queuedFibers.pop();

            mutex.unlock();

            switchToFiber(std::move(nextFiber));   // And this fiber is done here.
        }

        // Secondly complete all tasks.
        if (!queuedTasks.empty()) {
            mutex.lock();

            if (!queuedTasks.empty()) {
                std::function<void()> taskToBeDone = std::move(queuedTasks.front());
                queuedTasks.pop();

                mutex.unlock();

                taskToBeDone();
                --scheduler->workNum;
                continue;

            } else {     // Newly enqueued task being stolen.
                mutex.unlock();
            }
        }

        // steal
        std::function<void()> out;
        if (stealWork(out)) {
            out();
            --scheduler->workNum;
            continue;
        }

        // No works, block itself
        std::unique_lock<std::mutex> ul(mutex);
        cv.wait(ul, [this] { return !queuedFibers.empty() || !queuedTasks.empty(); });
    }
}

CIEL_NODISCARD bool Worker::stealWork(std::function<void()>& out) noexcept {
    for (size_t i = 0; i < scheduler->workers.size(); ++i) {
        if (this == scheduler->workers[i].get()) {
            continue;
        }

        if (scheduler->workers[i]->stealFromThis(out)) {
            return true;
        }
    }

    return false;
}

CIEL_NODISCARD bool Worker::stealFromThis(std::function<void()>& out) noexcept {
    // Since switching to mainFiber will be the last task pushed into queue when shutting down,
    // we can't tell their differences, so we don't steal one-size queue.
    if (queuedTasks.size() > 1) {
        std::lock_guard<std::mutex> lg(mutex);

        if (queuedTasks.size() > 1) {
            out = std::move(queuedTasks.front());
            queuedTasks.pop();

            return true;
        }
    }

    return false;
}

NAMESPACE_CTZ_END