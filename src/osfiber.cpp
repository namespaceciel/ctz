#include <ciel/core/config.hpp>
#include <cstdint>
#include <cstdlib>
#include <ctz/config.h>
#include <ctz/osfiber.h>
#include <functional>
#include <memory>
#include <new>
#include <utility>

NAMESPACE_CTZ_BEGIN

OSFiber::~OSFiber() {
    ::operator delete(stack);
}

CIEL_NODISCARD std::unique_ptr<OSFiber> OSFiber::createFiberFromCurrentThread() {
    auto out = std::unique_ptr<OSFiber>(new OSFiber);
    return out;
}

CIEL_NODISCARD std::unique_ptr<OSFiber> OSFiber::createFiber(const size_t stackSize, std::function<void()>&& func) {
    auto out    = std::unique_ptr<OSFiber>(new OSFiber);
    out->target = std::move(func);
    out->stack  = ::operator new(stackSize);
    ctz_fiber_set_target(&out->context, out->stack, static_cast<uint32_t>(stackSize),
                         reinterpret_cast<void (*)(void*)>(&OSFiber::run), out.get());
    return out;
}

void OSFiber::switchTo(OSFiber* fiber) noexcept {
    ctz_fiber_swap(&context, &fiber->context);
}

void OSFiber::run(OSFiber* self) {
    self->target();
}

NAMESPACE_CTZ_END
