# CTZ

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

这个项目参考自 [Google marl](https://github.com/google/marl)。

## containers

1、ciel::small_vector<T, BaseCapacity> 不同于 std::vector&lt;T>，它额外增加了第二模板参数 BaseCapacity，内部保存了一个栈数组 buffer[BaseCapacity]，在存储量小于 BASE_CAPACITY 时可以直接存储在 buffer 数组上，只有当大于 BASE_CAPACITY 时才会转为申请堆内存。

（这与 std::string，std::any， std::function 等的优化策略是一样的）

由于本项目对于容器的使用场景很多都是小且定量的，所以这样能有效规避大部分 malloc 调用。

2、ciel::list&lt;T> 与 std::list&lt;T> 的唯一区别是内部多一条 free 链表，保存着元素被抹除后的堆内存，同样是为了规避 malloc 调用。

由于本项目中存储的待办任务会大量的入队出队，这样会有较高的经济性。

经 benchmark 实测，当大量重复的在头尾 push 和 pop，ciel::list&lt;T> 的用时仅有 std::list&lt;T> 的 1/8 倍。

至于为什么不用 std::deque&lt;T> 这种连续空间的数组当队列，个人猜测是因为 std::deque&lt;T> 的内存用量略大。以 libc++ 的实现举例，std::deque&lt;T> 的每块数组长度的计算公式是 4096 / sizeof(T)，也就是说如果存储的是指针，一块数组就会有 512 个存量，很多情况下估计用不上这些内存。所以链表有一定的空间经济性。

## defer

CIEL_DEFER(x) 是一个宏，会把 x 包装进一个 lambda 里，并且生成一个 ciel::finally&lt;F> 对象，它在析构函数里会自动执行之前存入的语句。

可以用于实现类似 RAII 的操作如开关锁：

```cpp
std::mutex mtx;

void f() {
    mtx.lock();
    CIEL_DEFER(mtx.unlock());
    
    // do something...
    
}   // 离开作用域会自动解锁
```

## Task

每个 Task 存储着一个待办的任务（以 std::function<void()> 表示，需要的外部变量统一用值捕获存到里面），以供 scheduler 调度。

注意这里用值捕获而不是捕获引用是因为我们没法保证被捕获的变量的生命周期一定长于这个任务本身，所以复制一份进去更安全。

当然了，项目里的对象内部的功能都是用 std::shared_ptr 实现的，所以复制一份并不会影响这两份对象的交互问题（比如，可以正常地在外面 wait()，在任务里 signal()，内部调用的是同一根指针）。

内部有两种 Flags。当将 flag 设置为 SameThread 时，调度器保证此任务不会交给其它线程。在某些情况下能避免跨线程的消耗。

## Thread

Thread 其实是一个 std::thread 的包装类，它在不支持线程亲和性的平台上行为与 std::thread 完全一致。

而在支持的平台上，它在构造函数中创建完线程后会首先执行 setAffinity() 来进行一些设置，比如为其调用哪些可用的核心。然后再正常执行任务。

```
    // 判断当前平台是否支持线程亲和性。
#if defined(_WIN32) || \
    (defined(__linux__) && !defined(__ANDROID__) && !defined(__BIONIC__)) || \
    defined(__FreeBSD__)
    static constexpr bool supported = true;
#else
    static constexpr bool supported = false;
#endif
```

Thread 还使用到了 Pimpl(Pointer to implementation) 范式，它可以降低编译依赖，提高编译速度，如果把 .cpp 编译成库还可做到商业保密。如果是动态库，还可保持稳定的 ABI。

## thread_local

C/C++ 关键字，意为每个线程会单独持有一份此变量。只能用在静态或全局变量上。

