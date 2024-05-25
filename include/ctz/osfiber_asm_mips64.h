#ifndef CTZ_OSFIBER_ASM_MIPS64_H_
#define CTZ_OSFIBER_ASM_MIPS64_H_

#define CTZ_REG_a0  0x00
#define CTZ_REG_a1  0x08
#define CTZ_REG_s0  0x10
#define CTZ_REG_s1  0x18
#define CTZ_REG_s2  0x20
#define CTZ_REG_s3  0x28
#define CTZ_REG_s4  0x30
#define CTZ_REG_s5  0x38
#define CTZ_REG_s6  0x40
#define CTZ_REG_s7  0x48
#define CTZ_REG_f24 0x50
#define CTZ_REG_f25 0x58
#define CTZ_REG_f26 0x60
#define CTZ_REG_f27 0x68
#define CTZ_REG_f28 0x70
#define CTZ_REG_f29 0x78
#define CTZ_REG_f30 0x80
#define CTZ_REG_f31 0x88
#define CTZ_REG_gp  0x90
#define CTZ_REG_sp  0x98
#define CTZ_REG_fp  0xa0
#define CTZ_REG_ra  0xa8

#if defined(__APPLE__)
#define CTZ_ASM_SYMBOL(x) _##x
#else
#define CTZ_ASM_SYMBOL(x) x
#endif

#ifndef CTZ_BUILD_ASM

#include <stdint.h>

struct ctz_fiber_context {
    // parameter registers (First two)
    uintptr_t a0;
    uintptr_t a1;

    // callee-saved registers
    uintptr_t s0;
    uintptr_t s1;
    uintptr_t s2;
    uintptr_t s3;
    uintptr_t s4;
    uintptr_t s5;
    uintptr_t s6;
    uintptr_t s7;

    uintptr_t f24;
    uintptr_t f25;
    uintptr_t f26;
    uintptr_t f27;
    uintptr_t f28;
    uintptr_t f29;
    uintptr_t f30;
    uintptr_t f31;

    uintptr_t gp;
    uintptr_t sp;
    uintptr_t fp;
    uintptr_t ra;

}; // struct ctz_fiber_context

#ifdef __cplusplus
#include <cstddef>

static_assert(offsetof(ctz_fiber_context, a0) == CTZ_REG_a0, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, a1) == CTZ_REG_a1, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, s0) == CTZ_REG_s0, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, s1) == CTZ_REG_s1, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, s2) == CTZ_REG_s2, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, s3) == CTZ_REG_s3, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, s4) == CTZ_REG_s4, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, s5) == CTZ_REG_s5, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, s6) == CTZ_REG_s6, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, s7) == CTZ_REG_s7, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, f24) == CTZ_REG_f24, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, f25) == CTZ_REG_f25, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, f26) == CTZ_REG_f26, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, f27) == CTZ_REG_f27, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, f28) == CTZ_REG_f28, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, f29) == CTZ_REG_f29, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, f30) == CTZ_REG_f30, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, f31) == CTZ_REG_f31, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, gp) == CTZ_REG_gp, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, sp) == CTZ_REG_sp, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, fp) == CTZ_REG_fp, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, ra) == CTZ_REG_ra, "Bad register offset");

#endif // __cplusplus

#endif // CTZ_BUILD_ASM

#endif // CTZ_OSFIBER_ASM_MIPS64_H_
