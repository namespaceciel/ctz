#include <ctz/fiber.h>

NAMESPACE_CTZ_BEGIN

WaitingFibers::WaitingFibers() noexcept = default;

WaitingFibers::operator bool() const noexcept {
    return !fibers.empty();
}

Fiber* WaitingFibers::take(const TimePoint& timeout) noexcept {
    if (fibers.empty()) {
        return nullptr;
    }

    auto it = timeouts.begin();
    if (timeout < it->timepoint) {
        return nullptr;
    }

    auto fiber = it->fiber;
    timeouts.erase(it);
    const bool deleted = (fibers.erase(fiber) != 0);

    (void)deleted;
    CTZ_ASSERT(deleted, "WaitingFibers::take() maps out of sync");

    return fiber;
}

WaitingFibers::TimePoint WaitingFibers::next() const noexcept {
    CTZ_ASSERT(!fibers.empty(), "WaitingFibers::next() called when there are no waiting fibers");
    return timeouts.begin()->timepoint;
}

void WaitingFibers::add(const TimePoint& timeout, Fiber* fiber) {
    timeouts.emplace(Timeout{timeout, fiber});

    const bool added = fibers.emplace(fiber, timeout).second;
    (void)added;
    CTZ_ASSERT(added, "WaitingFibers::add() fiber already waiting");
}

void WaitingFibers::erase(Fiber* fiber) noexcept {
    auto it = fibers.find(fiber);

    if (it != fibers.end()) {
        auto timeout = it->second;

        const bool erased = (timeouts.erase(Timeout{timeout, fiber}) != 0);
        (void)erased;
        CTZ_ASSERT(erased, "WaitingFibers::erase() maps out of sync");

        fibers.erase(it);
    }
}

bool WaitingFibers::contains(Fiber* fiber) const noexcept {
    return fibers.find(fiber) != fibers.end();
}

bool WaitingFibers::Timeout::operator<(const Timeout& other) const noexcept {
    if (timepoint != other.timepoint) {
        return timepoint < other.timepoint;
    }

    return fiber < other.fiber;
}

NAMESPACE_CTZ_END