#include <ctz/scheduler.h>
#include <ctz/worker.h>

#include <luaopener/luaopener.h>

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
      scheduleFiber(Fiber::create(this, scheduler->config.fiberStackSize, [this] {
          while (true) {
              takeTask();

              switchToFiber(currentTask.get());
          }
      })),
      currentFiber(mainFiber.get()) {}

Worker::~Worker() {
    if (thread.joinable()) {
        stop();
    }
}

void Worker::enqueue(const std::function<void()>& newTask) {
    std::function<void()> decoratedTask = [=] {
        newTask();
        --scheduler->workNum;
        switchToFiber(scheduleFiber.get());
    };

    {
        std::lock_guard<std::mutex> lg(mutex);
        queueTasks.push(Fiber::create(this, scheduler->config.fiberStackSize, std::move(decoratedTask)));
    }

    cv.notify_one();
}

void Worker::enqueue(std::unique_ptr<Fiber>&& resumedTask) {
    {
        std::lock_guard<std::mutex> lg(mutex);
        queueTasks.push(std::move(resumedTask));
    }

    cv.notify_one();
}

void Worker::start() {
    CTZ_ASSERT(!thread.joinable(), "Worker::start() on a joinable thread");

    thread = std::thread([this] {
        current = this;

        switchToFiber(scheduleFiber.get());
    });
}

void Worker::stop() noexcept {
    {
        std::lock_guard<std::mutex> lg(mutex);
        queueTasks.push(Fiber::create(this, scheduler->config.fiberStackSize, [=] {
            switchToFiber(mainFiber.get());
        }));
    }

    cv.notify_one();

    thread.join();
}

void Worker::takeTask() noexcept {
    while (true) {
        // find work
        {
            if (!queueTasks.empty()) {
                std::lock_guard<std::mutex> lg(mutex);

                currentTask = std::move(queueTasks.front());
                queueTasks.pop();
                return;
            }
        }

        for (int i = 0; i < 256; ++i) {
            nop(); nop(); nop(); nop(); nop(); nop(); nop(); nop();
            nop(); nop(); nop(); nop(); nop(); nop(); nop(); nop();
            nop(); nop(); nop(); nop(); nop(); nop(); nop(); nop();
            nop(); nop(); nop(); nop(); nop(); nop(); nop(); nop();

            if (!queueTasks.empty()) {
                std::lock_guard<std::mutex> lg(mutex);

                currentTask = std::move(queueTasks.front());
                queueTasks.pop();
                return;
            }
        }

        // not found
        std::unique_lock<std::mutex> ul(mutex);
        cv.wait(ul);
    }
}

void Worker::switchToFiber(Fiber* to) noexcept {
    auto from = currentFiber;
    currentFiber = to;
    from->switchTo(to);
}

NAMESPACE_CTZ_END