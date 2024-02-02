#ifndef CTZ_CONDITIONVARIABLE_H_
#define CTZ_CONDITIONVARIABLE_H_

#include <ctz/config.h>
#include <ctz/scheduler.h>

#include <atomic>
#include <list>
#include <mutex>

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
                mutex.lock();
                waitingTasks.emplace_back(std::move(curWorker->currentTask));
                mutex.unlock();

                ul.unlock();

                curWorker->switchToFiber(curWorker->scheduleFiber.get());

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
    std::list<std::unique_ptr<Fiber>> waitingTasks;
    std::mutex mutex;   // Guarding waitingTasks.
    std::condition_variable cv;     // Managing tasks outside of scheduler.

};  // class ConditionVariable

NAMESPACE_CTZ_END

#endif // CTZ_CONDITIONVARIABLE_H_