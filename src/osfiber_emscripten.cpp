#if defined(__EMSCRIPTEN__)

#include <ctz/osfiber_emscripten.h>

extern "C" {

void
ctz_fiber_trampoline(void (*target)(void*), void* arg) {
    target(arg);
}

void
ctz_main_fiber_init(ctz_fiber_context* ctx) {
    emscripten_fiber_init_from_current_context(&ctx->context, ctx->asyncify_stack.data(), ctx->asyncify_stack.size());
}

void
ctz_fiber_set_target(ctz_fiber_context* ctx, void* stack, uint32_t stack_size, void (*target)(void*), void* arg) {
    emscripten_fiber_init(&ctx->context, target, arg, stack, stack_size, ctx->asyncify_stack.data(),
                          ctx->asyncify_stack.size());
}

extern void
ctz_fiber_swap(ctz_fiber_context* from, const ctz_fiber_context* to) {
    emscripten_fiber_swap(&from->context, const_cast<emscripten_fiber_t*>(&to->context));
}
}

#endif // defined(__EMSCRIPTEN__)
