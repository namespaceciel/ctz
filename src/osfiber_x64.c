#if defined(__x86_64__)

#  include <ctz/osfiber_x64.hpp>

// You can find an explanation of this code here:
// https://github.com/google/marl/issues/199

void ctz_fiber_trampoline(void (*target)(void*), void* arg) {
    target(arg);
}

void ctz_fiber_set_target(struct ctz_fiber_context* ctx, void* stack, uint32_t stack_size, void (*target)(void*),
                          void* arg) {
    uintptr_t* stack_top = (uintptr_t*)((uint8_t*)(stack) + stack_size);
    ctx->RIP             = (uintptr_t)&ctz_fiber_trampoline;
    ctx->RDI             = (uintptr_t)target;
    ctx->RSI             = (uintptr_t)arg;
    ctx->RSP             = (uintptr_t)&stack_top[-3];
    stack_top[-2]        = 0; // No return target.
}

#endif // defined(__x86_64__)
