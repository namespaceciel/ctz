#ifndef CTZ_SCHEDULER_H_
#define CTZ_SCHEDULER_H_

#include <atomic>
#include <ciel/core/finally.hpp>
#include <ciel/core/message.hpp>
#include <ciel/vector.hpp>
#include <cstddef>
#include <ctz/config.h>
#include <ctz/worker.h>
#include <memory>

NAMESPACE_CTZ_BEGIN

struct SchedulerConfig {
    static constexpr size_t DefaultFiberStackSize = 1024 * 1024;

    size_t fiberStackSize{DefaultFiberStackSize};

    size_t threadCount{0};

    // Use all available cores.
    CIEL_NODISCARD static SchedulerConfig allCores() noexcept;

    SchedulerConfig& setFiberStackSize(size_t) noexcept;

    SchedulerConfig& setWorkerThreadCount(size_t) noexcept;

}; // struct SchedulerConfig

class Scheduler {
public:
    Scheduler(const SchedulerConfig&) noexcept;

    Scheduler(const Scheduler&)            = delete;
    Scheduler& operator=(const Scheduler&) = delete;

    void bind() noexcept;

    static void setBound(Scheduler*) noexcept;

    CIEL_NODISCARD static Scheduler* get() noexcept;

    void unbind() noexcept;

    template<class... Args>
    void enqueue(Args&&... args) {
        CIEL_ASSERT_M(!workers.empty(), "Scheduler::enqueue on empty scheduler");

        workNum.fetch_add(1, std::memory_order_relaxed);
        workers[index.fetch_add(1, std::memory_order_relaxed) % workers.size()]->enqueue(std::forward<Args>(args)...);
    }

    static Scheduler* bound;

    const SchedulerConfig config;

private:
    friend class Worker;

    ciel::vector<std::unique_ptr<Worker>> workers;
    std::atomic<size_t> workNum{0};
    std::atomic<size_t> index{0};

}; // class Scheduler

template<class... Args>
void schedule(Args&&... args) {
    auto current = Scheduler::get();

    CIEL_ASSERT_M(current != nullptr, "schedule when no scheduler bound");

    current->enqueue(std::forward<Args>(args)...);
}

NAMESPACE_CTZ_END

#endif // CTZ_SCHEDULER_H_
