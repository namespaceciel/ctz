#ifndef CTZ_WORKER_H_
#define CTZ_WORKER_H_

#include <ctz/config.h>
#include <ctz/fiber.h>
#include <ctz/task.h>
#include <ctz/thread.h>

#include <ciel/small_vector.hpp>

#include <chrono>
#include <cstdint>
#include <deque>
#include <functional>
#include <mutex>
#include <thread>
#include <unordered_set>

NAMESPACE_CTZ_BEGIN

class Scheduler;
class Fiber;
class WaitingFibers;

// Work 存着 Worker 上排队的任务和纤程
struct Work {

    std::atomic<uint64_t> num = {0};  // tasks.size() + fibers.size()
    uint64_t numBlockedFibers = 0;

    std::deque<Task> tasks;
    std::deque<Fiber*> fibers;
    WaitingFibers waiting;

    bool notifyAdded = true;    // TODO: = true; 是干嘛的
    std::condition_variable added;
    std::mutex mutex;

    // TODO: 调用时 mtx 貌似必须上锁？
    // 这里的 F 是一个谓词。
    template<class F>
    void wait(F&& f);

};  // struct Work

// https://en.wikipedia.org/wiki/Xorshift
class FastRnd {
public:
    uint64_t operator()() noexcept {
        x ^= x << 13;
        x ^= x >> 7;
        x ^= x << 17;
        return x;
    }

private:
    uint64_t x = std::chrono::system_clock::now().time_since_epoch().count();

};  // class FastRnd

// Worker 在单一线程上执行一批任务。
class Worker {
public:
    using TimePoint = std::chrono::system_clock::time_point;
    using Predicate = std::function<bool()>;
    
    enum class Mode {
        // 生成一个后台线程办事。
        MultiThreaded,

        // 自己办事。
        SingleThreaded

    };  // enum class Mode

    Worker(Scheduler* scheduler, Mode mode, uint32_t id);

    // start() begins execution of the worker.
    void start();

    // stop() ceases execution of the worker, blocking until all pending
    // tasks have fully finished.
    void stop();

    // wait() suspends execution of the current task until the predicate pred
    // returns true or the optional timeout is reached.
    // See Fiber::wait() for more information.
    bool wait(std::mutex& waitLock, const TimePoint* timeout, const Predicate& pred);

    // wait() suspends execution of the current task until the fiber is
    // notified, or the optional timeout is reached.
    // See Fiber::wait() for more information.
    bool wait(const TimePoint* timeout);

    // suspend() suspends the currently executing Fiber until the fiber is
    // woken with a call to enqueue(Fiber*), or automatically sometime after the
    // optional timeout.
    void suspend(const TimePoint* timeout);

    // enqueue(Fiber*) enqueues resuming of a suspended fiber.
    void enqueue(Fiber* fiber);

    // enqueue(Task&&) enqueues a new, unstarted task.
    void enqueue(Task&& task);

    // tryLock() attempts to lock the worker for task enqueuing.
    // If the lock was successful then true is returned, and the caller must
    // call enqueueAndUnlock().
    bool tryLock();

    // enqueueAndUnlock() enqueues the task and unlocks the worker.
    // Must only be called after a call to tryLock() which returned true.
    // _Releases_lock_(work.mutex)
    void enqueueAndUnlock(Task&& task);

    // runUntilShutdown() processes all tasks and fibers until there are no more
    // and shutdown is true, upon runUntilShutdown() returns.
    void runUntilShutdown();

    // steal() attempts to steal a Task from the worker for another worker.
    // Returns true if a task was taken and assigned to out, otherwise false.
    bool steal(Task& out);

    // getCurrent() returns the Worker currently bound to the current
    // thread.
    static Worker* getCurrent() noexcept;

    // getCurrentFiber() returns the Fiber currently being executed.
    Fiber* getCurrentFiber() const noexcept;

    // Unique identifier of the Worker.
    const uint32_t id;

private:
    // run() is the task processing function for the worker.
    // run() processes tasks until stop() is called.
    void run();

    // createWorkerFiber() creates a new fiber that when executed calls
    // run().
    Fiber* createWorkerFiber();

    // switchToFiber() switches execution to the given fiber. The fiber
    // must belong to this worker.
    void switchToFiber(Fiber*);

    // runUntilIdle() executes all pending tasks and then returns.
    void runUntilIdle();

    // waitForWork() blocks until new work is available, potentially calling
    // spinForWork().
    void waitForWork();

    // spinForWorkAndLock() attempts to steal work from another Worker, and keeps
    // the thread awake for a short duration. This reduces overheads of
    // frequently putting the thread to sleep and re-waking. It locks the mutex
    // before returning so that a stolen task cannot be re-stolen by other workers.
    void spinForWorkAndLock();

    // enqueueFiberTimeouts() enqueues all the fibers that have finished
    // waiting.
    void enqueueFiberTimeouts();

    void setFiberState(Fiber* fiber, Fiber::State to) const noexcept;

    // The current worker bound to the current thread.
    static thread_local Worker* current;

    const Mode mode;
    Scheduler* const scheduler;
    std::unique_ptr<Fiber> mainFiber;
    Fiber* currentFiber = nullptr;
    Thread thread;
    Work work;
    std::unordered_set<Fiber*> idleFibers;  // 空闲纤程。
    ciel::small_vector<std::unique_ptr<Fiber>, 16> workerFibers;  // 被这个工人创建的所有纤程。
    FastRnd rng;
    bool shutdown = false;

};  // class Worker

struct SingleThreadedWorkers {

    using WorkerByTid = std::unordered_map<std::thread::id, std::unique_ptr<Worker>>;
    
    WorkerByTid byTid;

    std::mutex mutex;
    std::condition_variable unbind;

};  // struct SingleThreadedWorkers

NAMESPACE_CTZ_END

#include <ctz/worker_fiber_patch.inl>

#endif // CTZ_WORKER_H_