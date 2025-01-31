#ifndef CTZ_CONDITIONVARIABLE_H_
#define CTZ_CONDITIONVARIABLE_H_

#include <ciel/core/exchange.hpp>
#include <ciel/vector.hpp>
#include <ctz/config.h>
#include <ctz/scheduler.h>
#include <memory>
#include <mutex>

NAMESPACE_CTZ_BEGIN

class ConditionVariable {
public:
    ConditionVariable()                                    = default;
    ConditionVariable(const ConditionVariable&)            = delete;
    ConditionVariable& operator=(const ConditionVariable&) = delete;

    void notify_one();

    void notify_all();

    template<class Predicate>
    void wait(std::unique_lock<std::mutex>& ul, Predicate&& pred) {
        Worker* curWorker = Worker::current;

        if (curWorker) {
            while (!pred()) {
                auto* cur = curWorker->currentFiber.get();

                mutex.lock();
                waitingTasks.emplace_back(
                    ciel::exchange(curWorker->currentFiber,
                                   Fiber::create(curWorker, curWorker->scheduler->config.fiberStackSize, [curWorker] {
                                       curWorker->run();
                                   })));
                mutex.unlock();

                ul.unlock();
                cur->switchTo(curWorker->currentFiber.get());
                ul.lock();
            }

        } else {
            cv.wait(ul, std::forward<Predicate>(pred));
        }
    }

private:
    ciel::vector<std::unique_ptr<Fiber>> waitingTasks;
    std::mutex mutex;           // Guarding waitingTasks.
    std::condition_variable cv; // Managing tasks outside of scheduler.

}; // class ConditionVariable

NAMESPACE_CTZ_END

#endif // CTZ_CONDITIONVARIABLE_H_
