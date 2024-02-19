#include <ctz/scheduler.h>

NAMESPACE_CTZ_BEGIN

Scheduler* Scheduler::bound = nullptr;

// SchedulerConfig
SchedulerConfig SchedulerConfig::allCores() noexcept {
    return SchedulerConfig().setWorkerThreadCount(numLogicalCPUs());
}

SchedulerConfig& SchedulerConfig::setFiberStackSize(const size_t size) noexcept {
    fiberStackSize = size;
    return *this;
}

SchedulerConfig& SchedulerConfig::setWorkerThreadCount(const size_t count) noexcept {
    threadCount = count;
    return *this;
}

// Scheduler
Scheduler::Scheduler(const SchedulerConfig& cfg) noexcept
    : config(cfg) {}

void Scheduler::bind() noexcept {
    setBound(this);

    workers.reserve(config.threadCount);

    for (size_t i = 0; i < config.threadCount; ++i) {
        workers.emplace_back(new Worker);
    }

    for (auto& t : workers) {
        t->start();
    }
}

void Scheduler::setBound(Scheduler* scheduler) noexcept {
    bound = scheduler;
}

Scheduler* Scheduler::get() noexcept {
    return bound;
}

void Scheduler::unbind() noexcept {
    CTZ_ASSERT(get() == this, "unbind a scheduler that's not yours");
    CTZ_ASSERT(get() != nullptr, "no scheduler bound");

    // See TasksInTasks test, we need to ensure workers is valid before works left.
    while (workNum) {}

    for (auto& t : workers) {
        t->stop();
    }

    workers.clear();
    setBound(nullptr);
}

void Scheduler::enqueue(const std::function<void()>& newTask) {
    CTZ_ASSERT(!workers.empty(), "Scheduler::enqueue on empty scheduler");

    ++workNum;

    workers[index++ % workers.size()]->enqueue(newTask);

    if (index >= workers.size()) {
        index = 0;
    }
}

void Scheduler::enqueue(std::function<void()>&& newTask) {
    CTZ_ASSERT(!workers.empty(), "Scheduler::enqueue on empty scheduler");

    ++workNum;

    workers[index++ % workers.size()]->enqueue(std::move(newTask));

    if (index >= workers.size()) {
        index = 0;
    }
}

NAMESPACE_CTZ_END