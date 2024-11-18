#include <ciel/core/config.hpp>
#include <ciel/core/message.hpp>
#include <cstddef>
#include <ctz/config.h>
#include <ctz/fiber.h>
#include <ctz/osfiber.h>
#include <functional>
#include <memory>
#include <utility>

NAMESPACE_CTZ_BEGIN

thread_local Fiber* Fiber::current = nullptr;

Fiber::Fiber(Worker* w, std::unique_ptr<OSFiber>&& i)
    : worker(w), impl(std::move(i)) {}

void Fiber::switchTo(Fiber* to) {
    CIEL_ASSERT(this == Fiber::current, "The Fiber calling Fiber::switchTo() is not running");
    CIEL_ASSERT(this != to, "Fiber::switchTo() the same Fiber");

    Fiber::current = to;
    impl->switchTo(to->impl.get());
}

CIEL_NODISCARD std::unique_ptr<Fiber> Fiber::create(Worker* w, size_t stackSize, std::function<void()>&& func) {
    return std::unique_ptr<Fiber>(new Fiber(w, OSFiber::createFiber(stackSize, std::move(func))));
}

CIEL_NODISCARD std::unique_ptr<Fiber> Fiber::createFromCurrentThread(Worker* w) {
    return std::unique_ptr<Fiber>(new Fiber(w, OSFiber::createFiberFromCurrentThread()));
}

NAMESPACE_CTZ_END
