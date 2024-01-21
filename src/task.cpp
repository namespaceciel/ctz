#include <ctz/task.h>

NAMESPACE_CTZ_BEGIN

Task::Task() noexcept = default;

Task::Task(const Task& o) = default;

Task::Task(Task&& o) noexcept = default;

Task::Task(const Function& f, const Flags flags /* = Flags::None */)
    : f_(f), flag_(flags) {}

Task::Task(Function&& f, const Flags flags /* = Flags::None */) noexcept
    : f_(std::move(f)), flag_(flags) {}

Task& Task::operator=(const Task& other) = default;

Task& Task::operator=(Task&& other) noexcept = default;

Task& Task::operator=(const Function& f) {
    f_ = f;
    flag_ = Flags::None;
    return *this;
}

Task& Task::operator=(Function&& f) noexcept {
    f_ = std::move(f);
    flag_ = Flags::None;
    return *this;
}

Task::operator bool() const noexcept {
    return f_.operator bool();
}

void Task::operator()() const {
    f_();
}

bool Task::is(const Flags flag) const noexcept {
    return flag_ == flag;
}

NAMESPACE_CTZ_END