#ifndef CTZ_WORKER_H_
#define CTZ_WORKER_H_

#include <algorithm>
#include <atomic>
#include <ciel/core/config.hpp>
#include <ciel/experimental/list.hpp>
#include <condition_variable>
#include <ctz/config.hpp>
#include <ctz/fiber.hpp>
#include <mutex>
#include <queue>
#include <thread>

NAMESPACE_CTZ_BEGIN

CIEL_NODISCARD inline unsigned int numLogicalCPUs() noexcept {
    return std::max<unsigned int>(std::thread::hardware_concurrency(), 1);
}

// Worker possess works on a single thread.
class Worker {
public:
    Worker() = default;

    ~Worker() {
        if (thread.joinable()) {
            stop();
        }
    }

    template<class Function>
    void enqueue(Function&& f) noexcept {
        {
            const std::lock_guard<std::mutex> lg(mutex);
            queuedTasks.push(std::forward<Function>(f));
        }

        cv.notify_one();
    }

    template<class Function, class... Args>
    void enqueue(Function&& f, Args&&... args) noexcept {
        {
            const std::lock_guard<std::mutex> lg(mutex);
            queuedTasks.push(std::bind(std::forward<Function>(f), std::forward<Args>(args)...));
        }

        cv.notify_one();
    }

    void enqueue(std::unique_ptr<Fiber>&& resumedTask) noexcept {
        {
            const std::lock_guard<std::mutex> lg(mutex);
            queuedFibers.push(std::move(resumedTask));
        }

        cv.notify_one();
    }

private:
    friend class Fiber;
    friend class Scheduler;
    friend class ConditionVariable;

    void start() noexcept;

    // stop would only be called when no work left.
    void stop() noexcept {
        stop_flag.store(true, std::memory_order_relaxed);
        cv.notify_one();
        thread.join();
    }

    void switchToFiber(std::unique_ptr<Fiber>&& to) noexcept {
        auto from    = std::move(currentFiber);
        currentFiber = std::move(to);
        from->switchTo(currentFiber.get());
    }

    void run() noexcept;

    CIEL_NODISCARD bool stealWork(std::function<void()>&) noexcept;

    CIEL_NODISCARD bool stealFromThis(std::function<void()>& out) noexcept {
        if (!queuedTasks.empty()) {
            const std::lock_guard<std::mutex> lg(mutex);

            if CIEL_LIKELY (!queuedTasks.empty()) {
                out = std::move(queuedTasks.front());
                queuedTasks.pop();

                return true;
            }
        }

        return false;
    }

    static thread_local Worker* current;

    std::unique_ptr<Fiber> mainFiber{nullptr};
    std::unique_ptr<Fiber> currentFiber{nullptr};    // Since we need to tell the task who's its fiber...
    std::thread thread;
    std::queue<std::unique_ptr<Fiber>> queuedFibers; // produced by enqueue(std::unique_ptr<Fiber>&&), consumed by run()
    std::queue<std::function<void()>> queuedTasks;   // produced by enqueue(std::function<void()>), consumed by run()
    std::mutex mutex;                                // Guarding queuedFibers and queuedTasks.
    std::condition_variable cv;
    std::atomic<bool> stop_flag{false};

}; // class Worker

NAMESPACE_CTZ_END

#endif // CTZ_WORKER_H_
