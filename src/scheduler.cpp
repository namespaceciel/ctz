#include <ctz/scheduler.h>

NAMESPACE_CTZ_BEGIN

void nop() noexcept {
#if defined(_WIN32)
    __nop();
#else
    __asm__ __volatile__("nop");
#endif
}

// 用于下面的 Scheduler::Scheduler(const SchedulerConfig& config)
SchedulerConfig setConfigDefaults(const SchedulerConfig& cfgIn) {
    SchedulerConfig cfg{cfgIn};

    if (cfg.workerThread.count > 0 && !cfg.workerThread.affinityPolicy) {
        cfg.workerThread.affinityPolicy = Policy::anyOf(Affinity::all());
    }

    return cfg;
}

thread_local Scheduler* Scheduler::bound{nullptr};

// SchedulerConfig
SchedulerConfig SchedulerConfig::allCores() {
    return SchedulerConfig().setWorkerThreadCount(Thread::numLogicalCPUs());
}


SchedulerConfig& SchedulerConfig::setFiberStackSize(size_t size) {
    fiberStackSize = size;
    return *this;
}

SchedulerConfig& SchedulerConfig::setWorkerThreadCount(int count) {
    workerThread.count = count;
    return *this;
}

SchedulerConfig& SchedulerConfig::setWorkerThreadInitializer(const SchedulerConfig::ThreadInitializer& init) {
    workerThread.initializer = init;
    return *this;
}

SchedulerConfig& SchedulerConfig::setWorkerThreadAffinityPolicy(const std::shared_ptr<ctz::Policy>& policy) {
    workerThread.affinityPolicy = policy;
    return *this;
}

// Scheduler
Scheduler::Scheduler(const SchedulerConfig& config)
    : cfg(setConfigDefaults(config)) {

    for (int i = 0; i < cfg.workerThread.count; ++i) {
        spinningWorkers[i] = -1;
        workerThreads[i] = new Worker(this, Worker::Mode::MultiThreaded, i);
    }

    // TODO: 为什么要全部创建完再启动
    for (int i = 0; i < cfg.workerThread.count; ++i) {
        workerThreads[i]->start();
    }
}

Scheduler::~Scheduler() {
    {
        // 等待所有单线程工人解绑。
        std::unique_lock<std::mutex> ul(singleThreadedWorkers.mutex);
        singleThreadedWorkers.unbind.wait(ul, [this]() {
            return singleThreadedWorkers.byTid.empty();
        });
    }

    // 等所有任务 stop()（执行完）。
    for (int i = cfg.workerThread.count - 1; i > -1; --i) {
        workerThreads[i]->stop();
    }

    for (int i = cfg.workerThread.count - 1; i > -1; --i) {
        delete workerThreads[i];
    }
}

Scheduler* Scheduler::get() noexcept {
    return bound;
}


void Scheduler::bind() {
    CTZ_ASSERT(get() == nullptr, "Scheduler already bound");

    setBound(this);
    {
        std::unique_lock<std::mutex> ul(singleThreadedWorkers.mutex);

        auto worker = std::unique_ptr<Worker>(new Worker{this, Worker::Mode::SingleThreaded, static_cast<uint32_t>(-1)});
        worker->start();
        auto tid = std::this_thread::get_id();
        singleThreadedWorkers.byTid.emplace(tid, std::move(worker));
    }
}

void Scheduler::unbind() noexcept {
    CTZ_ASSERT(get() != nullptr, "No scheduler bound");

    auto worker = Worker::getCurrent();
    worker->stop();
    {
        std::unique_lock<std::mutex> ul(get()->singleThreadedWorkers.mutex);

        auto tid = std::this_thread::get_id();
        auto& workers = get()->singleThreadedWorkers.byTid;
        auto it = workers.find(tid);

        CTZ_ASSERT(it != workers.end(), "singleThreadedWorker not found");
        CTZ_ASSERT(it->second.get() == worker, "worker is not bound?");

        workers.erase(it);
        if (workers.empty()) {
            get()->singleThreadedWorkers.unbind.notify_one();
        }
    }
    setBound(nullptr);
}

void Scheduler::enqueue(Task&& task) {
    if (task.is(Task::Flags::SameThread)) {
        Worker::getCurrent()->enqueue(std::move(task));
        return;
    }

    if (cfg.workerThread.count > 0) {
        while (true) {
            // 优先找最近开始 spin 的
            auto i = --nextSpinningWorkerIdx % cfg.workerThread.count;
            auto idx = spinningWorkers[i].exchange(-1);

            if (idx < 0) {
                // 时间片轮转？找不到就下一个。
                idx = nextEnqueueIndex++ % cfg.workerThread.count;
            }

            auto worker = workerThreads[idx];
            if (worker->tryLock()) {
                worker->enqueueAndUnlock(std::move(task));
                return;
            }
        }

    } else {
        auto worker = Worker::getCurrent();
        CTZ_ASSERT(worker != nullptr, "singleThreadedWorker not found. Did you forget to call Scheduler::bind()?");

        worker->enqueue(std::move(task));
    }
}

const SchedulerConfig& Scheduler::config() const noexcept {
    return cfg;
}

bool Scheduler::stealWork(Worker* thief, uint64_t from, Task& out) {
    if (cfg.workerThread.count > 0) {
        auto thread = workerThreads[from % cfg.workerThread.count];
        if (thread != thief) {
            if (thread->steal(out)) {
                return true;
            }
        }
    }

    return false;
}

void Scheduler::onBeginSpinning(int workerId) {
    auto idx = nextSpinningWorkerIdx++ % cfg.workerThread.count;
    spinningWorkers[idx] = workerId;
}

void Scheduler::setBound(Scheduler* scheduler) noexcept {
    bound = scheduler;
}

NAMESPACE_CTZ_END