#include <ctz/worker.h>
#include <ctz/scheduler.h>

NAMESPACE_CTZ_BEGIN

thread_local Worker* Worker::current{nullptr};

void Worker::start() {
    switch (mode) {
    case Mode::MultiThreaded:
        auto& affinityPolicy = scheduler->cfg.workerThread.affinityPolicy;
        auto affinity = affinityPolicy->get(id);
        thread = Thread(std::move(affinity), [=] {

        if (auto const& initFunc = scheduler->cfg.workerThread.initializer) {
            initFunc(id);
        }

        Scheduler::setBound(scheduler);
        Worker::current = this;
        mainFiber = Fiber::createFromCurrentThread(0);
        currentFiber = mainFiber.get();
        {

            std::unique_lock<std::mutex> ul(work.mutex);
            run();
        }
        mainFiber.reset();
        Worker::current = nullptr;
        });
        break;
    
    case Mode::SingleThreaded:
        Worker::current = this;
        mainFiber = Fiber::createFromCurrentThread(0);
        currentFiber = mainFiber.get();
        break;
    
    default:
        ciel::unreachable();
    }
}

NAMESPACE_CTZ_END