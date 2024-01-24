// 这里包含着 worker.h 和 fiber.h 的类内函数模板实现。
// 因为函数模板不能分离实现到 .cpp 中，而 Work 和 Fiber 中各有依赖着对方完整定义的函数模板，所以单纯的前向声明无法解决问题。
// 这里的解决办法是在两个头文件最后 include 此补丁文件，并且判断只有定义过两个头文件的宏才可 include。

#ifdef CTZ_FIBER_H_
#ifdef CTZ_WORKER_H_

NAMESPACE_CTZ_BEGIN

template<class F>
void Work::wait(F&& f) {
    notifyAdded = true;

    if (waiting) {  // 有正在等待的纤程
        std::unique_lock<std::mutex> lock(mutex, std::adopt_lock);
        added.wait_until(lock, waiting.next(), std::forward<F>(f));
        lock.release();

    } else {
        std::unique_lock<std::mutex> lock(mutex, std::adopt_lock);
        added.wait(lock, std::forward<F>(f));
        lock.release();
    }

    notifyAdded = false;
}

template<class Clock, class Duration>
bool Fiber::wait(std::mutex& mtx, const std::chrono::time_point<Clock, Duration>& timeout, const Predicate& pred) {
    using ToDuration = typename TimePoint::duration;
    using ToClock = typename TimePoint::clock;
    auto tp = std::chrono::time_point_cast<ToDuration, ToClock>(timeout);
    return worker->wait(mtx, &tp, pred);
}

template<class Clock, class Duration>
bool Fiber::wait(const std::chrono::time_point<Clock, Duration>& timeout) {
    using ToDuration = typename TimePoint::duration;
    using ToClock = typename TimePoint::clock;
    auto tp = std::chrono::time_point_cast<ToDuration, ToClock>(timeout);
    return worker->wait(&tp);
}

NAMESPACE_CTZ_END

#endif
#endif