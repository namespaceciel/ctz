#ifndef CTZ_SCHEDULER_H_
#define CTZ_SCHEDULER_H_

#include <ctz/config.h>
#include <ctz/worker.h>

#include <ciel/finally.hpp>

#include <atomic>
#include <cstddef>
#include <memory>
#include <vector>

NAMESPACE_CTZ_BEGIN

// SchedulerConfig 有 Scheduler 的详细参数，可以设置好以后传给 Scheduler 的构造函数。
// 一般情况下直接用 SchedulerConfig::allCores() 即可。
struct SchedulerConfig {

    static constexpr size_t DefaultFiberStackSize = 1024 * 1024;

    // 每个纤程的栈大小。
    size_t fiberStackSize = DefaultFiberStackSize;

    // 需要几个线程
    size_t threadCount{0};

    // 使用所有可用核心。
    static SchedulerConfig allCores() noexcept;

    SchedulerConfig& setFiberStackSize(const size_t) noexcept;
    SchedulerConfig& setWorkerThreadCount(const size_t) noexcept;

};  // struct SchedulerConfig

class Scheduler {
public:
    Scheduler(const SchedulerConfig&);

    void bind() noexcept;

    static void setBound(Scheduler*);

    static Scheduler* get() noexcept;

    void unbind() noexcept;

    void enqueue(const std::function<void()>&);

    Scheduler(const Scheduler&) = delete;
    Scheduler(Scheduler&&) = delete;
    Scheduler& operator=(const Scheduler&) = delete;
    Scheduler& operator=(Scheduler&&) = delete;

    static Scheduler* bound;

    const SchedulerConfig config;

private:
    friend class Worker;

    std::vector<std::unique_ptr<Worker>> workers;
    std::atomic<size_t> workNum{0};
    size_t index{0};
    std::mutex mutex;   // Guard enqueue() from different threads.

};  // class Scheduler

void schedule(const std::string&, const std::string&);

void schedule(const std::function<void()>&);

NAMESPACE_CTZ_END

#endif // CTZ_SCHEDULER_H_