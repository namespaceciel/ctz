#ifndef CTZ_SCHEDULER_H_
#define CTZ_SCHEDULER_H_

#include <ctz/config.h>
#include <ctz/worker.h>

#include <ciel/finally.hpp> // CIEL_DEFER

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
    CIEL_NODISCARD static SchedulerConfig
    allCores() noexcept;

    SchedulerConfig&
    setFiberStackSize(const size_t) noexcept;
    SchedulerConfig&
    setWorkerThreadCount(const size_t) noexcept;

}; // struct SchedulerConfig

class Scheduler {
public:
    Scheduler(const SchedulerConfig&) noexcept;

    void
    bind() noexcept;

    static void
    setBound(Scheduler*) noexcept;

    CIEL_NODISCARD static Scheduler*
    get() noexcept;

    void
    unbind() noexcept;

    void
    enqueue(const std::function<void()>&);

    void
    enqueue(std::function<void()>&&);

    Scheduler(const Scheduler&) = delete;
    Scheduler(Scheduler&&)      = delete;
    // clang-format off
    Scheduler& operator=(const Scheduler&) = delete;
    Scheduler& operator=(Scheduler&&)      = delete;
    // clang-format on

    static Scheduler* bound;

    const SchedulerConfig config;

private:
    friend class Worker;

    std::vector<std::unique_ptr<Worker>> workers;
    std::atomic<size_t> workNum{0};
    std::atomic<size_t> index{0};

}; // class Scheduler

template<class Function>
void
schedule(Function&& f) {
    auto current = Scheduler::get();

    CTZ_ASSERT(current != nullptr, "schedule when no scheduler bound");

    current->enqueue(std::forward<Function>(f));
}

template<class Function, class... Args>
void
schedule(Function&& f, Args&&... args) {
    auto current = Scheduler::get();

    CTZ_ASSERT(current != nullptr, "schedule when no scheduler bound");

    current->enqueue(std::bind(std::forward<Function>(f), std::forward<Args>(args)...));
}

NAMESPACE_CTZ_END

#endif // CTZ_SCHEDULER_H_
