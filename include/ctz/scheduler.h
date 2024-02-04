#ifndef CTZ_SCHEDULER_H_
#define CTZ_SCHEDULER_H_

#include <ctz/config.h>
#include <ctz/worker.h>

#include <ciel/finally.hpp>     // CIEL_DEFER

#include <atomic>
#include <cstddef>
#include <memory>
#include <vector>

NAMESPACE_CTZ_BEGIN

struct SchedulerConfig {

    static constexpr size_t DefaultFiberStackSize = 1024 * 1024;

    size_t fiberStackSize = DefaultFiberStackSize;

    size_t threadCount{0};

    // Use all available cores.
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

template<class Function, class... Args>
void schedule(Function&& f, Args&&... args) {
    auto current = Scheduler::get();

    CTZ_ASSERT(current != nullptr, "schedule when no scheduler bound");

    current->enqueue(std::function<void()>(std::bind(std::forward<Function>(f), std::forward<Args>(args)...)));
}

NAMESPACE_CTZ_END

#endif // CTZ_SCHEDULER_H_