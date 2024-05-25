#if defined(__i386__)

#include <ctz/osfiber_asm_x86.h>

void
ctz_fiber_trampoline(void (*target)(void*), void* arg) {
    target(arg);
}

void
ctz_fiber_set_target(struct ctz_fiber_context* ctx, void* stack, uint32_t stack_size, void (*target)(void*),
                     void* arg) {
    // The stack pointer needs to be 16-byte aligned when making a 'call'.
    // The 'call' instruction automatically pushes the return instruction to the
    // stack (4-bytes), before making the jump.
    // The ctz_fiber_swap() assembly function does not use 'call', instead it
    // uses 'jmp', so we need to offset the ESP pointer by 4 bytes so that the
    // stack is still 16-byte aligned when the return target is stack-popped by
    // the callee.
    uintptr_t* stack_top = (uintptr_t*)((uint8_t*)(stack) + stack_size);
    ctx->EIP             = (uintptr_t)&ctz_fiber_trampoline;
    ctx->ESP             = (uintptr_t)&stack_top[-5];
    stack_top[-3]        = (uintptr_t)arg;
    stack_top[-4]        = (uintptr_t)target;
    stack_top[-5]        = 0; // No return target.
}

#endif // defined(__i386__)
