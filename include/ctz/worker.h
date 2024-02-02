#ifndef CTZ_WORKER_H_
#define CTZ_WORKER_H_

#include <ctz/config.h>
#include <ctz/fiber.h>

#include <ciel/list.hpp>

#include <mutex>
#include <queue>
#include <thread>

NAMESPACE_CTZ_BEGIN

unsigned int numLogicalCPUs() noexcept;

class Scheduler;

// Worker possess works on a single thread.
class Worker {
public:
    explicit Worker();

    ~Worker();

    void enqueue(const std::function<void()>&);

    void enqueue(std::unique_ptr<Fiber>&&);

private:
    friend class Fiber;
    friend class Scheduler;
    friend class ConditionVariable;

    void start();

    void stop() noexcept;

    void takeTask() noexcept;

    void switchToFiber(Fiber*) noexcept;

    static thread_local Worker* current;

    Scheduler* const scheduler;
    std::unique_ptr<Fiber> currentTask{nullptr};
    std::unique_ptr<Fiber> mainFiber;
    std::unique_ptr<Fiber> scheduleFiber;
    Fiber* currentFiber;    // Since we need to tell the task who's its fiber...
    std::thread thread;
    std::queue<std::unique_ptr<Fiber>> queueTasks;
    std::condition_variable cv;     // Block when no works left.
    std::mutex mutex;      // Guarding queueTasks from enqueue(), taskTask().

};  // class Worker

NAMESPACE_CTZ_END

#endif // CTZ_WORKER_H_