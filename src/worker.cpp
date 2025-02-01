#include <ciel/core/config.hpp>
#include <ciel/core/message.hpp>
#include <cstddef>
#include <ctz/config.hpp>
#include <ctz/fiber.hpp>
#include <ctz/scheduler.hpp>
#include <ctz/worker.hpp>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>

NAMESPACE_CTZ_BEGIN

thread_local Worker* Worker::current = nullptr;

void Worker::start() noexcept {
    CIEL_ASSERT_M(!thread.joinable(), "Worker::start() on a joinable thread");

    thread = std::thread([this] {
        current = this; // bind thread_local worker* to this thread

        mainFiber      = Fiber::createFromCurrentThread(this);
        Fiber::current = mainFiber.get();
        currentFiber   = Fiber::create(this, Scheduler::get().config.fiberStackSize, [this] {
            run();
        });

        mainFiber->switchTo(currentFiber.get());
    });
}

void Worker::run() noexcept {
    auto& scheduler = Scheduler::get();

    while (!stop_flag.load(std::memory_order_relaxed)) {
        // Firstly complete all fibers.
        if (!queuedFibers.empty()) { // Fiber can't be stolen.
            mutex.lock();

            std::unique_ptr<Fiber> nextFiber = std::move(queuedFibers.front());
            queuedFibers.pop();

            mutex.unlock();

            switchToFiber(std::move(nextFiber)); // And this fiber is done here.
        }

        // Secondly complete all tasks.
        if (!queuedTasks.empty()) {
            mutex.lock();

            if (!queuedTasks.empty()) {
                const std::function<void()> taskToBeDone = std::move(queuedTasks.front());
                queuedTasks.pop();

                mutex.unlock();

                taskToBeDone();
                scheduler.workNum.fetch_sub(1, std::memory_order_relaxed);
                continue;
            }
            // Newly enqueued task being stolen.
            mutex.unlock();
        }

        if (stop_flag.load(std::memory_order_relaxed)) {
            break;
        }

        // steal
        std::function<void()> out;
        if (stealWork(out)) {
            out();
            scheduler.workNum.fetch_sub(1, std::memory_order_relaxed);
            continue;
        }

        // No works, block itself
        std::unique_lock<std::mutex> ul(mutex);
        cv.wait(ul, [this] {
            return !queuedFibers.empty() || !queuedTasks.empty() || stop_flag.load(std::memory_order_relaxed);
        });
    }

    switchToFiber(std::move(mainFiber)); // mainFiber is done here...
}

CIEL_NODISCARD bool Worker::stealWork(std::function<void()>& out) noexcept {
    for (std::unique_ptr<Worker>& worker : Scheduler::get().workers) {
        if (this == worker.get()) {
            continue;
        }

        if (worker->stealFromThis(out)) {
            return true;
        }
    }

    return false;
}

NAMESPACE_CTZ_END
