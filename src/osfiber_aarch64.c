#if defined(__aarch64__)

#  include <ctz/osfiber_aarch64.hpp>
#  include <stddef.h>

void ctz_fiber_trampoline(void (*target)(void*), void* arg) {
    target(arg);
}

// __attribute__((weak)) doesn't work on macOS.
#  if defined(linux) || defined(__linux) || defined(__linux__)
// This is needed for HWSAan runtimes that don't have this commit:
// https://reviews.llvm.org/D149228.
__attribute__((weak)) void __hwasan_tag_memory(const volatile void* p, unsigned char tag, size_t size);
__attribute((weak)) void* __hwasan_tag_pointer(const volatile void* p, unsigned char tag);
#  endif

void ctz_fiber_set_target(struct ctz_fiber_context* ctx, void* stack, uint32_t stack_size, void (*target)(void*),
                          void* arg) {
#  if defined(linux) || defined(__linux) || defined(__linux__)
    if (__hwasan_tag_memory && __hwasan_tag_pointer) {
        stack = __hwasan_tag_pointer(stack, 0);
        __hwasan_tag_memory(stack, 0, stack_size);
    }
#  endif
    uintptr_t* stack_top = (uintptr_t*)((uint8_t*)(stack) + stack_size);
    ctx->LR              = (uintptr_t)&ctz_fiber_trampoline;
    ctx->r0              = (uintptr_t)target;
    ctx->r1              = (uintptr_t)arg;
    ctx->SP              = ((uintptr_t)stack_top) & ~(uintptr_t)15;
}

#endif // defined(__aarch64__)
