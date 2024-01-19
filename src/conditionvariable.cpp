#include <ctz/conditionvariable.h>
#include <ctz/config.h>
#include <memory>
#include <mutex>
#include <utility>

NAMESPACE_CTZ_BEGIN

void ConditionVariable::notify_one() {
    {
        const std::lock_guard<std::mutex> lg(mutex);

        if (!waitingTasks.empty()) {
            auto& resumedTask = waitingTasks.back();
            resumedTask->worker->enqueue(std::move(resumedTask));

            waitingTasks.pop_back();
        }
    }

    cv.notify_one();
}

void ConditionVariable::notify_all() {
    {
        const std::lock_guard<std::mutex> lg(mutex);

        for (std::unique_ptr<Fiber>& resumedTask : waitingTasks) {
            resumedTask->worker->enqueue(std::move(resumedTask));
        }

        waitingTasks.clear();
    }

    cv.notify_all();
}

NAMESPACE_CTZ_END
