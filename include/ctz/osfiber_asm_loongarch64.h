#ifndef CTZ_OSFIBER_ASM_LOONGARCH64_H_
#define CTZ_OSFIBER_ASM_LOONGARCH64_H_

#define CTZ_REG_a0 0x00
#define CTZ_REG_a1 0x08
#define CTZ_REG_s0 0x10
#define CTZ_REG_s1 0x18
#define CTZ_REG_s2 0x20
#define CTZ_REG_s3 0x28
#define CTZ_REG_s4 0x30
#define CTZ_REG_s5 0x38
#define CTZ_REG_s6 0x40
#define CTZ_REG_s7 0x48
#define CTZ_REG_s8 0x50
#define CTZ_REG_fs0 0x58
#define CTZ_REG_fs1 0x60
#define CTZ_REG_fs2 0x68
#define CTZ_REG_fs3 0x70
#define CTZ_REG_fs4 0x78
#define CTZ_REG_fs5 0x80
#define CTZ_REG_fs6 0x88
#define CTZ_REG_fs7 0x90
#define CTZ_REG_ra 0x98
#define CTZ_REG_sp 0xa0
#define CTZ_REG_fp 0xa8

#ifndef CTZ_BUILD_ASM

#include <stdint.h>

// Procedure Call Standard for the LoongArch 64-bit Architecture
// https://loongson.github.io/LoongArch-Documentation/LoongArch-ELF-ABI-EN.html
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
    uintptr_t s8;

    uintptr_t fs0;
    uintptr_t fs1;
    uintptr_t fs2;
    uintptr_t fs3;
    uintptr_t fs4;
    uintptr_t fs5;
    uintptr_t fs6;
    uintptr_t fs7;

    uintptr_t ra;
    uintptr_t sp;
    uintptr_t fp;

};  // struct ctz_fiber_context

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
static_assert(offsetof(ctz_fiber_context, s8) == CTZ_REG_s8, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, fs0) == CTZ_REG_fs0, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, fs1) == CTZ_REG_fs1, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, fs2) == CTZ_REG_fs2, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, fs3) == CTZ_REG_fs3, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, fs4) == CTZ_REG_fs4, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, fs5) == CTZ_REG_fs5, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, fs6) == CTZ_REG_fs6, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, fs7) == CTZ_REG_fs7, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, ra) == CTZ_REG_ra, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, sp) == CTZ_REG_sp, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, fp) == CTZ_REG_fp, "Bad register offset");

#endif // __cplusplus

#endif // CTZ_BUILD_ASM

#endif // CTZ_OSFIBER_ASM_LOONGARCH64_H_