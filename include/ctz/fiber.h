#ifndef CTZ_FIBER_H_
#define CTZ_FIBER_H_

#include <ctz/config.h>

#include <functional>
#include <memory>
#include <set>

#if defined(_WIN32)
#include <ctz/osfiber_windows.h>
#else
#include <ctz/osfiber_asm.h>
#endif

NAMESPACE_CTZ_BEGIN

class Worker;

// Fiber 是 OSFiber 的封装，OSFiber 的实现全是一些平台特异的代码。
class Fiber {
public:
    using TimePoint = std::chrono::system_clock::time_point;
    using Predicate = std::function<bool()>;

    // 获取当前执行的纤程，如果没绑定到 Scheduler 上则为 nullptr。
    static Fiber* current() noexcept;

    // 暂停当前纤程直到 pred 为 true 时被 notify() 唤醒。若为 false 则无效，等待下一次 notify()。
    // 传入的 mtx 必须已经上锁！！！返回时会让 mtx 保持上锁状态。
    // wait() 只能被当前执行的纤程调用。
    void wait(std::mutex& mtx, const Predicate& pred);

    // 同上，但是 timeout 到时间会自动唤醒。
    template<class Clock, class Duration>
    bool wait(std::mutex& mtx, const std::chrono::time_point<Clock, Duration>& timeout, const Predicate& pred);

    // 只建议在同线程调用 wait() 时 notify() 时使用。
    // 它在跨线程场景中可能会导致死锁：当一个线程的 wait() 调用完但还未挂起时，另一个线程已经调用了 notify()，那么这次信号无法被接收到。
    void wait();

    // 同上。
    template<class Clock, class Duration>
    bool wait(const std::chrono::time_point<Clock, Duration>& timeout);

    void notify();

    // Fiber 的线程特异标识符。
    const uint32_t id;

private:
    friend class Scheduler;
    friend class Worker;

    enum class State {
        // 空闲并被收集在 Worker::idleFibers 里
        Idle,

        // 永久阻塞
        Yielded,

        // 带着倒计时的阻塞，被收集在 Worker::Work::waiting 里
        Waiting,

        // 排队等待执行，被收集在 Worker::Work::fibers 里
        Queued,

        // 正在运行
        Running,

    };  // enum class State

    Fiber(std::unique_ptr<OSFiber>&&, uint32_t id);

    // 必须在正在执行的 Fiber 上调用，将执行权转到另一个给定的纤程。
    void switchTo(Fiber*);

    static std::unique_ptr<Fiber> create(uint32_t id, size_t stackSize, const std::function<void()>& func);

    static std::unique_ptr<Fiber> createFromCurrentThread(uint32_t id);

    const std::unique_ptr<OSFiber> impl;
    Worker* const worker;
    State state = State::Running;  // 线程安全由 worker->work.mutex 保护。

};  // class Fiber

// 一个 Fiber 容器，持有所有等待倒计时的 Fiber。
struct WaitingFibers {
    using TimePoint = std::chrono::system_clock::time_point;
    
    WaitingFibers() noexcept;
    
    // 还有没有等待的 Fiber。
    explicit operator bool() const noexcept;

    // fibers 为空返回 nullptr。
    // 由于 timeouts 是以时间升序存储，所以拿到第一个时间，如果没到时间就返回 nullptr。
    // 到时间了就返回。
    Fiber* take(const TimePoint& timeout) noexcept;

    TimePoint next() const noexcept;

    void add(const TimePoint& timeout, Fiber* fiber);

    void erase(Fiber* fiber) noexcept;

    bool contains(Fiber* fiber) const noexcept;

private:
    struct Timeout {
        TimePoint timepoint;
        Fiber* fiber;
        
        bool operator<(const Timeout& other) const noexcept;

    };  // struct Timeout

    std::set<Timeout> timeouts;
    std::unordered_map<Fiber*, TimePoint> fibers;

};  // struct WaitingFibers

NAMESPACE_CTZ_END

#include <ctz/worker_fiber_patch.inl>

#endif // CTZ_FIBER_H_