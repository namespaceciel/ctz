#if !defined(_WIN32)

#include <ctz/osfiber_asm.h>

#include <cstdlib>

NAMESPACE_CTZ_BEGIN

OSFiber::~OSFiber() {
    free(stack);
}

CIEL_NODISCARD std::unique_ptr<OSFiber> OSFiber::createFiberFromCurrentThread() {
    auto out = std::unique_ptr<OSFiber>(new OSFiber);
    ctz_main_fiber_init(&out->context);
    return out;
}

CIEL_NODISCARD std::unique_ptr<OSFiber> OSFiber::createFiber(const size_t stackSize, std::function<void()>&& func) {
    auto out = std::unique_ptr<OSFiber>(new OSFiber);
    out->target = std::move(func);
    out->stack = malloc(stackSize);
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

#endif