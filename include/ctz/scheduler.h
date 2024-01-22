#ifndef CTZ_SCHEDULER_H_
#define CTZ_SCHEDULER_H_

#include <ctz/config.h>
#include <ctz/fiber.h>
#include <ctz/thread.h>
#include <ctz/worker.h>

#include <cstddef>
#include <functional>
#include <memory>

NAMESPACE_CTZ_BEGIN

// SchedulerConfig 有 Scheduler 的详细参数，可以设置好以后传给 Scheduler 的构造函数。
struct SchedulerConfig {

    using ThreadInitializer = std::function<void(int workerId)>;

    static constexpr size_t DefaultFiberStackSize = 1024 * 1024;

    // 每个工人线程的设置。
    struct WorkerThread {
        // 工人线程数
        int count = 0;

        // 设置一个在线程创建好到执行工作前需要做的事
        ThreadInitializer initializer;

        // 线程亲和性策略
        std::shared_ptr<ctz::Policy> affinityPolicy;
    };

    WorkerThread workerThread;

    // 每个纤程的栈大小。
    size_t fiberStackSize = DefaultFiberStackSize;

    // 使用所有可用核心。
    static SchedulerConfig allCores();

    // 返回自己以供链式调用。
    SchedulerConfig& setFiberStackSize(size_t);

    SchedulerConfig& setWorkerThreadCount(int);

    SchedulerConfig& setWorkerThreadInitializer(const ThreadInitializer&);

    SchedulerConfig& setWorkerThreadAffinityPolicy(const std::shared_ptr<ctz::Policy>&);

};  // struct SchedulerConfig

// 每个 Scheduler 可以用 bind() 函数绑定到一或多个线程上。
// 当绑定成功，对应的线程就可以调用 ctz::schedule() 来将待处理的任务入队执行。
// Scheduler 默认初始为单线程模式，可以通过调用 SchedulerConfig::setWorkerThreadCount() 生成专用工作线程。
class Scheduler {
public:
    Scheduler(const SchedulerConfig&);

    ~Scheduler();

    static Scheduler* get() noexcept;

    void bind();

    static void unbind() noexcept;

    void enqueue(Task&&);

    const SchedulerConfig& config() const noexcept;

    Scheduler(const Scheduler&) = delete;

    Scheduler(Scheduler&&) = delete;

    Scheduler& operator=(const Scheduler&) = delete;

    Scheduler& operator=(Scheduler&&) = delete;

private:
    friend class Worker;
    
    static constexpr size_t MaxWorkerThreads = 256;

    bool stealWork(Worker* thief, uint64_t from, Task& out);

    void onBeginSpinning(int workerId);

    static void setBound(Scheduler*) noexcept;

    static thread_local Scheduler* bound;

    const SchedulerConfig cfg;

    std::array<std::atomic<int>, MaxWorkerThreads> spinningWorkers;
    std::atomic<unsigned int> nextSpinningWorkerIdx = {0x8000000};

    std::atomic<unsigned int> nextEnqueueIndex = {0};
    std::array<Worker*, MaxWorkerThreads> workerThreads{};

    SingleThreadedWorkers singleThreadedWorkers;

};  // class Scheduler

NAMESPACE_CTZ_END

#endif // CTZ_SCHEDULER_H_