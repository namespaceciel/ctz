#if !defined(_WIN32)

#include <ctz/osfiber_asm.h>

NAMESPACE_CTZ_BEGIN

OSFiber::~OSFiber() {
    if (stack != nullptr) {
        free(stack);
    }
}

std::unique_ptr<OSFiber> OSFiber::createFiberFromCurrentThread() {
    auto out = std::unique_ptr<OSFiber>(new OSFiber);
    out->context = {};
    ctz_main_fiber_init(&out->context);
    return out;
}

std::unique_ptr<OSFiber> OSFiber::createFiber(size_t stackSize, const std::function<void()>& func) {

    auto out = std::unique_ptr<OSFiber>(new OSFiber);
    out->context = {};
    out->target = func;
    out->stack = malloc(stackSize);
    ctz_fiber_set_target(
        &out->context, out->stack, static_cast<uint32_t>(stackSize),
        reinterpret_cast<void (*)(void*)>(&OSFiber::run), out.get());
    return out;
}

void OSFiber::run(OSFiber* self) {
    self->target();
}

void OSFiber::switchTo(OSFiber* fiber) {
    ctz_fiber_swap(&context, &fiber->context);
}

NAMESPACE_CTZ_END

#endif