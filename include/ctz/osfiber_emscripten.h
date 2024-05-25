#ifndef CTZ_OSFIBER_ASM_EMSCRIPTEN_H_
#define CTZ_OSFIBER_ASM_EMSCRIPTEN_H_

#ifndef CTZ_BUILD_WASM

#include <array>
#include <cstddef>
#include <cstdint>
#include <emscripten.h>
#include <emscripten/fiber.h>

struct ctz_fiber_context {
    // callee-saved data
    static constexpr size_t asyncify_stack_size = 1024 * 1024;
    emscripten_fiber_t context;
    std::array</*std::byte*/ char, asyncify_stack_size> asyncify_stack;

}; // struct ctz_fiber_context

#endif // CTZ_BUILD_ASM

#endif // CTZ_OSFIBER_ASM_EMSCRIPTEN_H_
