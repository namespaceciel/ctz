#include <ctz/fiber.h>
#include <ctz/worker.h>

NAMESPACE_CTZ_BEGIN

// Fiber
Fiber* Fiber::current() noexcept {
    auto worker = Worker::getCurrent();
    return worker != nullptr ? worker->getCurrentFiber() : nullptr;
}

void Fiber::wait(std::mutex& mtx, const Predicate& pred) {
    CTZ_ASSERT(worker == Worker::getCurrent(), "Fiber::wait() must only be called on the currently executing fiber");
    worker->wait(mtx, nullptr, pred);
}

void Fiber::wait() {
    worker->wait(nullptr);
}

void Fiber::notify() {
    worker->enqueue(this);
}

Fiber::Fiber(std::unique_ptr<OSFiber>&& impl, uint32_t id) : id(id), impl(std::move(impl)), worker(Worker::getCurrent()) {
    CTZ_ASSERT(worker != nullptr, "No Worker bound");
}

void Fiber::switchTo(Fiber* to) {
    CTZ_ASSERT(worker == Worker::getCurrent(), "Fiber::switchTo() must only be called on the currently executing fiber");

    if (to != this) {
        impl->switchTo(to->impl.get());
    }
}

std::unique_ptr<Fiber> Fiber::create(uint32_t id, size_t stackSize, const std::function<void()>& func) {
    return std::unique_ptr<Fiber>(new Fiber(OSFiber::createFiber(stackSize, func), id));
}

std::unique_ptr<Fiber> Fiber::createFromCurrentThread(uint32_t id) {
    return std::unique_ptr<Fiber>(new Fiber(OSFiber::createFiberFromCurrentThread(), id));
}

// WaitingFibers
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