#include <ctz/fiber.h>

NAMESPACE_CTZ_BEGIN

Fiber::Fiber(Worker* w, std::unique_ptr<OSFiber>&& i)
    : worker(w), impl(std::move(i)) {}

void Fiber::switchTo(Fiber* to) {
    CTZ_ASSERT(this != to, "Fiber::switchTo() the same Fiber");

    impl->switchTo(to->impl.get());
}

std::unique_ptr<Fiber> Fiber::create(Worker* w, const size_t stackSize, std::function<void()>&& func) {
    return std::unique_ptr<Fiber>(new Fiber(w, OSFiber::createFiber(stackSize, std::move(func))));
}

std::unique_ptr<Fiber> Fiber::createFromCurrentThread(Worker* w) {
    return std::unique_ptr<Fiber>(new Fiber(w, OSFiber::createFiberFromCurrentThread()));
}

NAMESPACE_CTZ_END