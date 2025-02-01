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

    size_t fiberStackSize = DefaultFiberStackSize;
    size_t threadCount    = 1;

    // Use all available cores.
    CIEL_NODISCARD static SchedulerConfig allCores() noexcept {
        SchedulerConfig res;
        res.threadCount = ctz::numLogicalCPUs();
        return res;
    }

}; // struct SchedulerConfig

class Scheduler {
public:
    Scheduler(const Scheduler&)            = delete;
    Scheduler& operator=(const Scheduler&) = delete;

    static void start(const SchedulerConfig cfg) noexcept {
        auto& self = get();

        CIEL_ASSERT(!is_running());
        CIEL_ASSERT(cfg.threadCount > 0);
        CIEL_ASSERT_M(cfg.fiberStackSize >= 16 * 1024, "Stack sizes less than 16KB may cause issues on some platforms");

        self.config = cfg;
        self.workers.reserve(self.config.threadCount);

        for (size_t i = 0; i < self.config.threadCount; ++i) {
            self.workers.unchecked_emplace_back(new Worker);
        }

        for (auto& t : self.workers) {
            t->start();
        }
    }

    static void stop() noexcept {
        auto& self = get();

        CIEL_ASSERT(is_running());

        while (self.workNum.load(std::memory_order_relaxed) > 0) {}

        for (auto& t : self.workers) {
            t->stop();
        }

        self.workers.clear();
    }

    template<class... Args>
    static void schedule(Args&&... args) noexcept {
        auto& self = get();

        CIEL_ASSERT_M(is_running(), "Scheduler has not started yet");

        self.workNum.fetch_add(1, std::memory_order_relaxed);
        self.workers[self.index.fetch_add(1, std::memory_order_relaxed) % self.workers.size()]->enqueue(
            std::forward<Args>(args)...);
    }

    CIEL_NODISCARD static bool is_running() noexcept {
        return !get().workers.empty();
    }

private:
    Scheduler() = default;

    CIEL_NODISCARD static Scheduler& get() noexcept {
        static Scheduler res;
        return res;
    }

    friend class Worker;
    friend class ConditionVariable;

    ciel::vector<std::unique_ptr<Worker>> workers;
    std::atomic<size_t> workNum{0};
    std::atomic<size_t> index{0};
    SchedulerConfig config;

}; // class Scheduler

NAMESPACE_CTZ_END

#endif // CTZ_SCHEDULER_H_
