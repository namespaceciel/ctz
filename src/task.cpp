#include <ctz/task.h>

NAMESPACE_CTZ_BEGIN

Task::Task() noexcept = default;

Task::Task(const Task& o) = default;

Task::Task(Task&& o) noexcept = default;

Task::Task(const Function& f, const Flags flags /* = Flags::None */)
    : func(f), flag(flags) {}

Task::Task(Function&& f, const Flags flags /* = Flags::None */) noexcept
    : func(std::move(f)), flag(flags) {}

Task& Task::operator=(const Task& other) = default;

Task& Task::operator=(Task&& other) noexcept = default;

Task& Task::operator=(const Function& f) {
    func = f;
    flag = Flags::None;
    return *this;
}

Task& Task::operator=(Function&& f) noexcept {
    func = std::move(f);
    flag = Flags::None;
    return *this;
}

Task::operator bool() const noexcept {
    return func.operator bool();
}

void Task::operator()() const {
    func();
}

bool Task::is(const Flags other) const noexcept {
    return flag == other;
}

NAMESPACE_CTZ_END