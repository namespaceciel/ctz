#if defined(__i386__)

#define CTZ_BUILD_ASM 1
#include <ctz/osfiber_asm_x86.h>

// void ctz_fiber_swap(ctz_fiber_context* from, const ctz_fiber_context* to)
// esp+4: from
// esp+8: to
.text
.global ctz_fiber_swap
.align 4
ctz_fiber_swap:
    // Save context 'from'
    movl        4(%esp), %eax

    // Store callee-preserved registers
    movl        %ebx, CTZ_REG_EBX(%eax)
    movl        %ebp, CTZ_REG_EBP(%eax)
    movl        %esi, CTZ_REG_ESI(%eax)
    movl        %edi, CTZ_REG_EDI(%eax)

    movl        (%esp), %ecx             /* call stores the return address on the stack before jumping */
    movl        %ecx, CTZ_REG_EIP(%eax)
    lea         4(%esp), %ecx            /* skip the pushed return address */
    movl        %ecx, CTZ_REG_ESP(%eax)

    // Load context 'to'
    movl        8(%esp), %ecx

    // Load callee-preserved registers
    movl        CTZ_REG_EBX(%ecx), %ebx
    movl        CTZ_REG_EBP(%ecx), %ebp
    movl        CTZ_REG_ESI(%ecx), %esi
    movl        CTZ_REG_EDI(%ecx), %edi

    // Load stack pointer
    movl        CTZ_REG_ESP(%ecx), %esp

    // Load instruction pointer, and jump
    movl        CTZ_REG_EIP(%ecx), %ecx
    jmp         *%ecx

#endif // defined(__i386__)
