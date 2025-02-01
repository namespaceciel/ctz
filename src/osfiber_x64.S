#if defined(__x86_64__)

#define CTZ_BUILD_ASM 1
#include <ctz/osfiber_x64.hpp>

// void ctz_fiber_swap(ctz_fiber_context* from, const ctz_fiber_context* to)
// rdi: from
// rsi: to
.text
.global CTZ_ASM_SYMBOL(ctz_fiber_swap)
.align 4
CTZ_ASM_SYMBOL(ctz_fiber_swap):

    // Save context 'from'

    // Store callee-preserved registers
    movq        %rbx, CTZ_REG_RBX(%rdi)
    movq        %rbp, CTZ_REG_RBP(%rdi)
    movq        %r12, CTZ_REG_R12(%rdi)
    movq        %r13, CTZ_REG_R13(%rdi)
    movq        %r14, CTZ_REG_R14(%rdi)
    movq        %r15, CTZ_REG_R15(%rdi)

    movq        (%rsp), %rcx             /* call stores the return address on the stack before jumping */
    movq        %rcx, CTZ_REG_RIP(%rdi)
    leaq        8(%rsp), %rcx            /* skip the pushed return address */
    movq        %rcx, CTZ_REG_RSP(%rdi)

    // Load context 'to'
    movq        %rsi, %r8

    // Load callee-preserved registers
    movq        CTZ_REG_RBX(%r8), %rbx
    movq        CTZ_REG_RBP(%r8), %rbp
    movq        CTZ_REG_R12(%r8), %r12
    movq        CTZ_REG_R13(%r8), %r13
    movq        CTZ_REG_R14(%r8), %r14
    movq        CTZ_REG_R15(%r8), %r15

    // Load first two call parameters
    movq        CTZ_REG_RDI(%r8), %rdi
    movq        CTZ_REG_RSI(%r8), %rsi

    // Load stack pointer
    movq        CTZ_REG_RSP(%r8), %rsp

    // Load instruction pointer, and jump
    movq        CTZ_REG_RIP(%r8), %rcx
    jmp         *%rcx

#endif // defined(__x86_64__)
