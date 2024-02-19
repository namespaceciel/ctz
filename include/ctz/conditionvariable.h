#ifndef CTZ_CONDITIONVARIABLE_H_
#define CTZ_CONDITIONVARIABLE_H_

#include <ctz/config.h>
#include <ctz/scheduler.h>

#include <memory>
#include <mutex>
#include <vector>

NAMESPACE_CTZ_BEGIN

class ConditionVariable {
public:
    ConditionVariable() noexcept = default;

    void notify_one();

    void notify_all();

    template<class Predicate>
    void wait(std::unique_lock<std::mutex>& ul, Predicate&& pred) {

        Worker* curWorker = Worker::current;

        if (curWorker) {
            while (!pred()) {
                auto* cur = curWorker->currentFiber.get();

                mutex.lock();
                waitingTasks.emplace_back(std::move(curWorker->currentFiber));
                mutex.unlock();

                ul.unlock();

                CTZ_ASSERT(!curWorker->currentFiber, "curWorker->currentFiber is supposed to be moved above");

                curWorker->currentFiber = Fiber::create(curWorker, curWorker->scheduler->config.fiberStackSize, [curWorker] {
                    curWorker->run();
                });
                cur->switchTo(curWorker->currentFiber.get());

                ul.lock();
            }

        } else {
            cv.wait(ul, std::forward<Predicate>(pred));
        }
    }

    ConditionVariable(const ConditionVariable&) = delete;
    ConditionVariable(ConditionVariable&&) = delete;
    ConditionVariable& operator=(const ConditionVariable&) = delete;
    ConditionVariable& operator=(ConditionVariable&&) = delete;

private:
    std::vector<std::unique_ptr<Fiber>> waitingTasks;
    std::mutex mutex;   // Guarding waitingTasks.
    std::condition_variable cv;     // Managing tasks outside of scheduler.

};  // class ConditionVariable

NAMESPACE_CTZ_END

#endif // CTZ_CONDITIONVARIABLE_H_