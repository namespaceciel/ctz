#ifndef CTZ_TASK_H
#define CTZ_TASK_H

#include <ctz/config.h>

#include <functional>

NAMESPACE_CTZ_BEGIN

// 每个 Task 存储着一个任务（以 std::function<void()> 表示，需要的外部变量统一用值捕获存到里面），以供 scheduler 调度。
// 如果没有 Flags 那它跟 std::function<void()> 无异。
class Task {
public:
    using Function = std::function<void()>;

    enum class Flags {
        None = 0,

        // 当将 flags 设置为 SameThread 时，调度器保证此任务不会交给其它线程。
        // TODO: 没看懂有什么用
        SameThread = 1,

    };  // enum class Flags

    Task() noexcept;

    Task(const Task&);

    Task(Task&&) noexcept;

    explicit Task(const Function&, Flags = Flags::None);

    explicit Task(Function&&, Flags = Flags::None) noexcept;

    Task& operator=(const Task&);

    Task& operator=(Task&&) noexcept;

    Task& operator=(const Function&);

    Task& operator=(Function&&) noexcept;

    // 检查 func 是否处于合法状态
    explicit operator bool() const noexcept;

    // 执行 func
    void operator()() const;

    bool is(Flags) const noexcept;

private:
    Function func;
    Flags flag;

};  // class Task

NAMESPACE_CTZ_END

#endif // CTZ_TASK_H