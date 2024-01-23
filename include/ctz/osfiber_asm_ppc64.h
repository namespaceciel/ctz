#ifndef CTZ_OSFIBER_ASM_PPC64_H_
#define CTZ_OSFIBER_ASM_PPC64_H_

#define CTZ_REG_R1 0x00
#define CTZ_REG_R2 0x08
#define CTZ_REG_R13 0x10
#define CTZ_REG_R14 0x18
#define CTZ_REG_R15 0x20
#define CTZ_REG_R16 0x28
#define CTZ_REG_R17 0x30
#define CTZ_REG_R18 0x38
#define CTZ_REG_R19 0x40
#define CTZ_REG_R20 0x48
#define CTZ_REG_R21 0x50
#define CTZ_REG_R22 0x58
#define CTZ_REG_R23 0x60
#define CTZ_REG_R24 0x68
#define CTZ_REG_R25 0x70
#define CTZ_REG_R26 0x78
#define CTZ_REG_R27 0x80
#define CTZ_REG_R28 0x88
#define CTZ_REG_R29 0x90
#define CTZ_REG_R30 0x98
#define CTZ_REG_R31 0xa0

#define CTZ_REG_R3 0xa8
#define CTZ_REG_R4 0xb0

#define CTZ_REG_LR 0xb8
#define CTZ_REG_CCR 0xc0

#define CTZ_REG_FPR14 0xc8
#define CTZ_REG_FPR15 0xd0
#define CTZ_REG_FPR16 0xd8
#define CTZ_REG_FPR17 0xe0
#define CTZ_REG_FPR18 0xe8
#define CTZ_REG_FPR19 0xf0
#define CTZ_REG_FPR20 0xf8
#define CTZ_REG_FPR21 0x100
#define CTZ_REG_FPR22 0x108
#define CTZ_REG_FPR23 0x110
#define CTZ_REG_FPR24 0x118
#define CTZ_REG_FPR25 0x120
#define CTZ_REG_FPR26 0x128
#define CTZ_REG_FPR27 0x130
#define CTZ_REG_FPR28 0x138
#define CTZ_REG_FPR29 0x140
#define CTZ_REG_FPR30 0x148
#define CTZ_REG_FPR31 0x150

#define CTZ_REG_VRSAVE 0x158
#define CTZ_REG_VMX 0x160

#ifndef CTZ_BUILD_ASM

#include <stdint.h>

struct ctz_fiber_context {
    // non-volatile registers
    uintptr_t r1;
    uintptr_t r2;
    uintptr_t r13;
    uintptr_t r14;
    uintptr_t r15;
    uintptr_t r16;
    uintptr_t r17;
    uintptr_t r18;
    uintptr_t r19;
    uintptr_t r20;
    uintptr_t r21;
    uintptr_t r22;
    uintptr_t r23;
    uintptr_t r24;
    uintptr_t r25;
    uintptr_t r26;
    uintptr_t r27;
    uintptr_t r28;
    uintptr_t r29;
    uintptr_t r30;
    uintptr_t r31;

    // first two parameter registers (r3, r4)
    uintptr_t r3;
    uintptr_t r4;

    // special registers
    uintptr_t lr;
    uintptr_t ccr;

    // non-volatile floating-point registers (f14-f31)
    uintptr_t fpr14;
    uintptr_t fpr15;
    uintptr_t fpr16;
    uintptr_t fpr17;
    uintptr_t fpr18;
    uintptr_t fpr19;
    uintptr_t fpr20;
    uintptr_t fpr21;
    uintptr_t fpr22;
    uintptr_t fpr23;
    uintptr_t fpr24;
    uintptr_t fpr25;
    uintptr_t fpr26;
    uintptr_t fpr27;
    uintptr_t fpr28;
    uintptr_t fpr29;
    uintptr_t fpr30;
    uintptr_t fpr31;

    // non-volatile altivec registers
    uint32_t vrsave;
    uintptr_t vmx[12 * 2];

};  // struct ctz_fiber_context

#ifdef __cplusplus
#include <cstddef>

static_assert(offsetof(ctz_fiber_context, r1) == CTZ_REG_R1, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, r2) == CTZ_REG_R2, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, r13) == CTZ_REG_R13, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, r15) == CTZ_REG_R15, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, r16) == CTZ_REG_R16, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, r17) == CTZ_REG_R17, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, r18) == CTZ_REG_R18, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, r19) == CTZ_REG_R19, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, r20) == CTZ_REG_R20, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, r21) == CTZ_REG_R21, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, r22) == CTZ_REG_R22, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, r23) == CTZ_REG_R23, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, r24) == CTZ_REG_R24, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, r25) == CTZ_REG_R25, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, r26) == CTZ_REG_R26, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, r27) == CTZ_REG_R27, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, r28) == CTZ_REG_R28, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, r29) == CTZ_REG_R29, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, r30) == CTZ_REG_R30, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, r31) == CTZ_REG_R31, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, r14) == CTZ_REG_R14, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, lr) == CTZ_REG_LR, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, ccr) == CTZ_REG_CCR, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, fpr14) == CTZ_REG_FPR14, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, fpr15) == CTZ_REG_FPR15, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, fpr16) == CTZ_REG_FPR16, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, fpr17) == CTZ_REG_FPR17, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, fpr18) == CTZ_REG_FPR18, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, fpr19) == CTZ_REG_FPR19, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, fpr20) == CTZ_REG_FPR20, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, fpr21) == CTZ_REG_FPR21, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, fpr22) == CTZ_REG_FPR22, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, fpr23) == CTZ_REG_FPR23, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, fpr24) == CTZ_REG_FPR24, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, fpr25) == CTZ_REG_FPR25, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, fpr26) == CTZ_REG_FPR26, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, fpr27) == CTZ_REG_FPR27, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, fpr28) == CTZ_REG_FPR28, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, fpr29) == CTZ_REG_FPR29, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, fpr30) == CTZ_REG_FPR30, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, fpr31) == CTZ_REG_FPR31, "Bad register offset");
static_assert((offsetof(ctz_fiber_context, vmx) % 16) == 0, "VMX must be quadword aligned");
static_assert(offsetof(ctz_fiber_context, vmx) == CTZ_REG_VMX, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, vrsave) == CTZ_REG_VRSAVE, "Bad register offset");

#endif // __cplusplus

#endif // CTZ_BUILD_ASM

#endif // CTZ_OSFIBER_ASM_PPC64_H_