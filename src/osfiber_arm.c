#if defined(__arm__)

#  include <ctz/osfiber_arm.h>

void ctz_fiber_trampoline(void (*target)(void*), void* arg) {
    target(arg);
}

void ctz_fiber_set_target(struct ctz_fiber_context* ctx, void* stack, uint32_t stack_size, void (*target)(void*),
                          void* arg) {
    uintptr_t* stack_top = (uintptr_t*)((uint8_t*)(stack) + stack_size);
    ctx->LR              = (uintptr_t)&ctz_fiber_trampoline;
    ctx->r0              = (uintptr_t)target;
    ctx->r1              = (uintptr_t)arg;
    ctx->SP              = ((uintptr_t)stack_top) & ~(uintptr_t)15;
}

#endif // defined(__arm__)
