#ifndef CTZ_WORKER_H_
#define CTZ_WORKER_H_

#include <ctz/config.h>
#include <ctz/fiber.h>

#include <ciel/list.hpp>

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

NAMESPACE_CTZ_BEGIN

CIEL_NODISCARD unsigned int
numLogicalCPUs() noexcept;

class Scheduler;

// Worker possess works on a single thread.
class Worker {
public:
    explicit Worker();

    ~Worker();

    void
    enqueue(const std::function<void()>&);

    void
    enqueue(std::function<void()>&&);

    void
    enqueue(std::unique_ptr<Fiber>&&);

private:
    friend class Fiber;
    friend class Scheduler;
    friend class ConditionVariable;

    void
    start();

    void
    stop() noexcept;

    void
    switchToFiber(std::unique_ptr<Fiber>&&) noexcept;

    [[noreturn]] void
    run() noexcept;

    CIEL_NODISCARD bool
    stealWork(std::function<void()>&) noexcept;

    CIEL_NODISCARD bool
    stealFromThis(std::function<void()>&) noexcept;

    static thread_local Worker* current;

    Scheduler* const scheduler;
    std::unique_ptr<Fiber> mainFiber{nullptr};
    std::unique_ptr<Fiber> currentFiber{nullptr};    // Since we need to tell the task who's its fiber...
    std::thread thread;
    std::queue<std::unique_ptr<Fiber>> queuedFibers; // produced by enqueue(std::unique_ptr<Fiber>&&), consumed by run()
    std::queue<std::function<void()>> queuedTasks;   // produced by enqueue(std::function<void()>), consumed by run()
    std::mutex mutex;                                // Guarding queuedFibers and queuedTasks.
    std::condition_variable cv;

}; // class Worker

NAMESPACE_CTZ_END

#endif // CTZ_WORKER_H_
