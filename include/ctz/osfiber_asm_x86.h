#ifndef CTZ_OSFIBER_ASM_X86_H_
#define CTZ_OSFIBER_ASM_X86_H_

#define CTZ_REG_EBX 0x00
#define CTZ_REG_EBP 0x04
#define CTZ_REG_ESI 0x08
#define CTZ_REG_EDI 0x0c
#define CTZ_REG_ESP 0x10
#define CTZ_REG_EIP 0x14

#ifndef CTZ_BUILD_ASM
#include <stdint.h>

// Assumes cdecl calling convention.
// Registers EAX, ECX, and EDX are caller-saved, and the rest are callee-saved.
struct ctz_fiber_context {
    // callee-saved registers
    uintptr_t EBX;
    uintptr_t EBP;
    uintptr_t ESI;
    uintptr_t EDI;

    // stack and instruction registers
    uintptr_t ESP;
    uintptr_t EIP;

};  // struct ctz_fiber_context

#ifdef __cplusplus
#include <cstddef>

static_assert(offsetof(ctz_fiber_context, EBX) == CTZ_REG_EBX, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, EBP) == CTZ_REG_EBP, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, ESI) == CTZ_REG_ESI, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, EDI) == CTZ_REG_EDI, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, ESP) == CTZ_REG_ESP, "Bad register offset");
static_assert(offsetof(ctz_fiber_context, EIP) == CTZ_REG_EIP, "Bad register offset");

#endif // __cplusplus

#endif // CTZ_BUILD_ASM

#endif // CTZ_OSFIBER_ASM_X86_H_