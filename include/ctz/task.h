#ifndef CTZ_TASK_H
#define CTZ_TASK_H

#include <ctz/config.h>

#include <functional>

NAMESPACE_CTZ_BEGIN

// 每个 Task 存储着一个任务（以 std::function<void()> 表示，需要的外部变量统一用值捕获存到里面），以供 scheduler 调度
class Task {
public:
    using Function = std::function<void()>;

    enum class Flags {
        None = 0,

        // 当将 flags_ 设置为 SameThread 时，调度器保证此任务不会交给其它线程。
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

    // 检查 f_ 是否处于合法状态
    explicit operator bool() const noexcept;

    // 执行 f_
    void operator()() const;

    bool is(Flags) const noexcept;

private:
    Function f_;
    Flags flag_;

};  // class Task

NAMESPACE_CTZ_END

#endif // CTZ_TASK_H