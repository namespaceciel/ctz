#if defined(__powerpc64__)

#include <ctz/osfiber_asm_ppc64.h>

void ctz_fiber_trampoline(void (*target)(void*), void* arg) {
    target(arg);
}

void ctz_fiber_set_target(struct ctz_fiber_context* ctx,
                         void* stack,
                         uint32_t stack_size,
                         void (*target)(void*),
                         void* arg) {

    uintptr_t stack_top = (uintptr_t)((uint8_t*)(stack) + stack_size - sizeof(uintptr_t));
    if ((stack_top % 16) != 0) {
        stack_top -= (stack_top % 16);
    }

    // Write a backchain and subtract a minimum stack frame size (32/48)
    *(uintptr_t*)stack_top = 0;
#if !defined(_CALL_ELF) || (_CALL_ELF != 2)
    stack_top -= 48;
    *(uintptr_t*)stack_top = stack_top + 48;
#else
    stack_top -= 32;
    *(uintptr_t*)stack_top = stack_top + 32;
#endif

    // Load registers
    ctx->r1 = stack_top;
#if !defined(_CALL_ELF) || (_CALL_ELF != 2)
    ctx->lr = ((const uintptr_t *)ctz_fiber_trampoline)[0];
    ctx->r2 = ((const uintptr_t *)ctz_fiber_trampoline)[1];
#else
    ctx->lr = (uintptr_t)ctz_fiber_trampoline;
#endif
    ctx->r3 = (uintptr_t)target;
    ctx->r4 = (uintptr_t)arg;

    // Thread pointer must be saved in r13
    __asm__ volatile("mr %0, 13\n" : "=r"(ctx->r13));
}

#endif  // __powerpc64__