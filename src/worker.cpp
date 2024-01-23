#include <ctz/scheduler.h>
#include <ctz/worker.h>

NAMESPACE_CTZ_BEGIN

thread_local Worker* Worker::current{nullptr};

void Worker::start() {
    switch (mode) {
        case Mode::MultiThreaded: {     // 创建新线程。
            auto& affinityPolicy = scheduler->cfg.workerThread.affinityPolicy;
            auto affinity = affinityPolicy->get(id);
            thread = Thread(std::move(affinity), [=] {
                if (const auto& initFunc = scheduler->cfg.workerThread.initializer) {   // 如果有需要先做的事。
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
        }
        case Mode::SingleThreaded: {    // 创建一个纤程绑在 currentFiber 上。不会主动运行任务，直到调用 suspend()。
            Worker::current = this;
            mainFiber = Fiber::createFromCurrentThread(0);
            currentFiber = mainFiber.get();
            break;
        }
        default:
            ciel::unreachable();
    }
}

NAMESPACE_CTZ_END