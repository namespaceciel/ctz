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

Worker::~Worker() {
    if (thread.joinable()) {
        stop();
    }
}

void Worker::enqueue(std::unique_ptr<Fiber>&& resumedTask) {
    {
        const std::lock_guard<std::mutex> lg(mutex);
        queuedFibers.push(std::move(resumedTask));
    }

    cv.notify_one();
}

void Worker::start() {
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

void Worker::stop() noexcept {
    enqueue([this] {
        switchToFiber(std::move(mainFiber)); // mainFiber is done here...
    });

    thread.join();
}

void Worker::switchToFiber(std::unique_ptr<Fiber>&& to) noexcept {
    auto from    = std::move(currentFiber);
    currentFiber = std::move(to);
    from->switchTo(currentFiber.get());
}

[[noreturn]] void Worker::run() noexcept {
    auto& scheduler = Scheduler::get();
    while (true) {
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
            return !queuedFibers.empty() || !queuedTasks.empty();
        });
    }
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

CIEL_NODISCARD bool Worker::stealFromThis(std::function<void()>& out) noexcept {
    // Since switching to mainFiber will be the last task pushed into queue when shutting down,
    // we can't tell their differences, so we don't steal one-size queue.
    if (queuedTasks.size() > 1) {
        const std::lock_guard<std::mutex> lg(mutex);

        if (queuedTasks.size() > 1) {
            out = std::move(queuedTasks.front());
            queuedTasks.pop();

            return true;
        }
    }

    return false;
}

NAMESPACE_CTZ_END
