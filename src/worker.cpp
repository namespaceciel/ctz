#include <ctz/scheduler.h>
#include <ctz/worker.h>

#include <unistd.h>

NAMESPACE_CTZ_BEGIN

// numLogicalCPUs()
#if defined(_WIN32)
unsigned int numLogicalCPUs() noexcept {
    unsigned int count = 0;
    const auto& groups = getProcessorGroups();

    for (size_t groupIdx = 0; groupIdx < groups.count; ++groupIdx) {
        const auto& group = groups.groups[groupIdx];
        count += group.count;
    }

    return count;
}

#else
unsigned int numLogicalCPUs() noexcept {
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
    queuedTasks.enqueue(newTask);

    //    CIEL_UNUSED(barrier.arrive());
    cv.notify_one();
}

void Worker::enqueue(std::function<void()>&& newTask) {
    queuedTasks.enqueue(std::move(newTask));

    //    CIEL_UNUSED(barrier.arrive());
    cv.notify_one();
}

void Worker::enqueue(std::unique_ptr<Fiber>&& resumedTask) {
    queuedFibers.enqueue(std::move(resumedTask));

    //    CIEL_UNUSED(barrier.arrive());
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

bool Worker::takeTask(std::function<void()>& taskToBeDone) noexcept {
    if (queuedTasks.try_dequeue(taskToBeDone)) {
        return true;
    }

    for (int i = 0; i < 256; ++i) {
        nop(); nop(); nop(); nop(); nop(); nop(); nop(); nop();
        nop(); nop(); nop(); nop(); nop(); nop(); nop(); nop();
        nop(); nop(); nop(); nop(); nop(); nop(); nop(); nop();
        nop(); nop(); nop(); nop(); nop(); nop(); nop(); nop();

        if (queuedTasks.try_dequeue(taskToBeDone)) {
            return true;
        }
    }

    return false;
}

void Worker::switchToFiber(std::unique_ptr<Fiber>&& to) noexcept {
    auto from = std::move(currentFiber);
    currentFiber = std::move(to);
    from->switchTo(currentFiber.get());
}

[[noreturn]] void Worker::run() noexcept {
    while (true) {
        // Firstly complete all fibers
        std::unique_ptr<Fiber> nextFiber;
        if (queuedFibers.try_dequeue(nextFiber)) {
            switchToFiber(std::move(nextFiber));   // And this fiber is done here.
        }

        // Secondly complete all tasks
        std::function<void()> taskToBeDone;
        if (takeTask(taskToBeDone)) {
            taskToBeDone();
            --scheduler->workNum;
            continue;
        }

        // No works, block itself
//        barrier.wait(0);
        std::unique_lock<std::mutex> ul(mutex);
        cv.wait(ul);
    }
}

NAMESPACE_CTZ_END