#ifndef CTZ_OSFIBER_X64_H_
#define CTZ_OSFIBER_X64_H_

#define CTZ_REG_RBX 0x00
#define CTZ_REG_RBP 0x08
#define CTZ_REG_R12 0x10
#define CTZ_REG_R13 0x18
#define CTZ_REG_R14 0x20
#define CTZ_REG_R15 0x28
#define CTZ_REG_RDI 0x30
#define CTZ_REG_RSI 0x38
#define CTZ_REG_RSP 0x40
#define CTZ_REG_RIP 0x48

#if defined(__APPLE__)
#  define CTZ_ASM_SYMBOL(x) _##x
#else
#  define CTZ_ASM_SYMBOL(x) x
#endif

#ifndef CTZ_BUILD_ASM
#  include <stdint.h>

struct ctz_fiber_context {
    // callee-saved registers
    uintptr_t RBX;
    uintptr_t RBP;
    uintptr_t R12;
    uintptr_t R13;
    uintptr_t R14;
    uintptr_t R15;

    // parameter registers
    uintptr_t RDI;
    uintptr_t RSI;

    // stack and instruction registers
    uintptr_t RSP;
    uintptr_t RIP;
}; // struct ctz_fiber_context

#  ifdef __cplusplus
#    include <cstddef>

static_assert(offsetof(ctz_fiber_context, RBX) == CTZ_REG_RBX, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, RBP) == CTZ_REG_RBP, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, R12) == CTZ_REG_R12, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, R13) == CTZ_REG_R13, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, R14) == CTZ_REG_R14, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, R15) == CTZ_REG_R15, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, RDI) == CTZ_REG_RDI, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, RSI) == CTZ_REG_RSI, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, RSP) == CTZ_REG_RSP, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, RIP) == CTZ_REG_RIP, "Bad register offset");

#  endif

#endif

#endif // CTZ_OSFIBER_X64_H_
