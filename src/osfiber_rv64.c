#if defined(__riscv) && __riscv_xlen == 64

#include <ctz/osfiber_asm_rv64.h>

void ctz_fiber_trampoline(void (*target)(void*), void* arg) {
    target(arg);
}

void ctz_fiber_set_target(struct ctz_fiber_context* ctx,
                         void* stack,
                         uint32_t stack_size,
                         void (*target)(void*),
                         void* arg) {

    uintptr_t* stack_top = (uintptr_t*)((uint8_t*)(stack) + stack_size);
    ctx->ra = (uintptr_t)&ctz_fiber_trampoline;
    ctx->a0 = (uintptr_t)target;
    ctx->a1 = (uintptr_t)arg;
    ctx->sp = ((uintptr_t)stack_top) & ~(uintptr_t)15;
}

#endif  // defined(__riscv) && __riscv_xlen == 64