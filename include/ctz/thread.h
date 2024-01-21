#ifndef CTZ_THREAD_H_
#define CTZ_THREAD_H_

#include <ctz/config.h>

#include <ciel/small_vector.hpp>

#include <cstddef>
#include <functional>

NAMESPACE_CTZ_BEGIN

// 表示某个核心
struct Core {

    struct Windows {
        uint8_t group;  // 组号
        uint8_t index;  // 组内核心号
    };

    struct Pthread {
        uint16_t index;  // 核心号
    };

    union {
        Windows windows;
        Pthread pthread;
    };

    bool operator==(Core) const noexcept;

    bool operator<(Core) const noexcept;

};  // struct Core

// 线程亲和性决定了一个线程可以在哪些处理器核心上执行任务。
struct Affinity {
    
    // 判断当前平台是否支持线程亲和性。
#if defined(_WIN32) || \
    (defined(__linux__) && !defined(__ANDROID__) && !defined(__BIONIC__)) || \
    defined(__FreeBSD__)
    static constexpr bool supported = true;
#else
    static constexpr bool supported = false;
#endif

    Affinity() noexcept;

    Affinity(Affinity&&) noexcept;

    Affinity(const Affinity&);

    Affinity& operator=(Affinity&&) noexcept;

    Affinity(std::initializer_list<Core>);

    Affinity(const ciel::small_vector<Core, 32>&);

    // 返回一个让所有核心都可用的亲和性策略。
    static Affinity all();

    // 返回可用核心数。
    size_t count() const noexcept;

    // 返回第 i 个核心。
    Core operator[](size_t) const noexcept;

    // 将给定的策略内的核心加入当前策略中。
    // 返回自己以便链式调用。
    Affinity& add(const Affinity&);

    // 移除给定的策略内的核心。
    // 返回自己以便链式调用。
    Affinity& remove(const Affinity&);

private:
    ciel::small_vector<Core, 32> cores;

};  // struct Affinity

// 纯虚类接口，anyOf 与 oneOf 函数内会创建一个继承本身并实现 get(ThreadID) 的对象，然后返回
class Policy {
public:
virtual ~Policy() = default;

// 根据给定线程 ID 返回策略
virtual Affinity get(uint32_t) const = 0;

// 返回的 Policy 的 get(ThreadID) 会返回所有的可用核心
// Windows 要求每个线程只能关联到一组亲和策略，所以只能返回同一组的全部可用核心
static std::shared_ptr<Policy> anyOf(Affinity&&);

// 返回的 Policy 的 get(ThreadID) 会返回一个可用核心，计算公式为：
// affinity[ThreadID % affinity.count()]
static std::shared_ptr<Policy> oneOf(Affinity&&);

};  // class Policy

class Thread {
public:
    using Function = std::function<void()>;

    Thread() noexcept;

    Thread(Thread&&) noexcept;

    Thread& operator=(Thread&&) noexcept;

    Thread(Affinity&& affinity, Function&& func);

    ~Thread();

    // 阻塞，等待线程执行完。
    void join();

    // 返回可用的逻辑 CPU 核心数。
    static unsigned int numLogicalCPUs();

    Thread(const Thread&) = delete;
    Thread& operator=(const Thread&) = delete;

private:
    class Impl;

    Impl* impl = nullptr;

};  // class Thread

NAMESPACE_CTZ_END

#endif // CTZ_THREAD_H_