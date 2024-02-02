#include <ctz/scheduler.h>

#include <luaopener/luaopener.h>

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
Scheduler::Scheduler(const SchedulerConfig& cfg)
    : config(cfg) {}

void Scheduler::bind() noexcept {
    setBound(this);

    workers.reserve(config.threadCount);

    for (size_t i = 0; i < config.threadCount; ++i) {
        workers.emplace_back(new Worker());
    }

    for (auto& t : workers) {
        t->start();
    }
}

void Scheduler::setBound(Scheduler* scheduler) {
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

    workers.clear();
    setBound(nullptr);
}

void Scheduler::enqueue(const std::function<void()>& newTask) {
    CTZ_ASSERT(!workers.empty(), "Scheduler::enqueue on empty scheduler");

    ++workNum;

    std::lock_guard<std::mutex> lg(mutex);

    workers[index]->enqueue(newTask);

    if (++index == workers.size()) {
        index = 0;
    }
}

void schedule(const std::string& file, const std::string& name) {
    std::function<void()> newTask = [=] {
        ciel::LuaOpener opener;
        opener.loadFile(file);

        opener[name].call();

        opener.closeFile();
        remove(file.c_str());
    };

    schedule(newTask);
}

void schedule(const std::function<void()>& newTask) {
    auto current = Scheduler::get();

    CTZ_ASSERT(current != nullptr, "schedule when no scheduler bound");

    current->enqueue(newTask);
}

NAMESPACE_CTZ_END