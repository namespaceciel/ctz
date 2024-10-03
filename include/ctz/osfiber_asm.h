#ifndef CTZ_OSFIBER_ASM_H_
#define CTZ_OSFIBER_ASM_H_

#if !defined(_WIN32)

#if defined(__x86_64__)
#include <ctz/osfiber_asm_x64.h>
#elif defined(__i386__)
#include <ctz/osfiber_asm_x86.h>
#elif defined(__aarch64__)
#include <ctz/osfiber_asm_aarch64.h>
#elif defined(__arm__)
#include <ctz/osfiber_asm_arm.h>
#elif defined(__powerpc64__)
#include <ctz/osfiber_asm_ppc64.h>
#elif defined(__mips__) && _MIPS_SIM == _ABI64
#include <ctz/osfiber_asm_mips64.h>
#elif defined(__riscv) && __riscv_xlen == 64
#include <ctz/osfiber_asm_rv64.h>
#elif defined(__loongarch__) && _LOONGARCH_SIM == _ABILP64
#include <ctz/osfiber_asm_loongarch64.h>
#elif defined(__EMSCRIPTEN__)
#include <ctz/osfiber_emscripten.h>
#else
#error "Unsupported target"
#endif

#include <ctz/config.h>

#include <functional>
#include <memory>

extern "C" {

#if defined(__EMSCRIPTEN__)
void
ctz_main_fiber_init(ctz_fiber_context* ctx);
#else
inline void
ctz_main_fiber_init(ctz_fiber_context*) {}
#endif
extern void
ctz_fiber_set_target(ctz_fiber_context*, void* stack, uint32_t stack_size, void (*target)(void*), void* arg);

extern void
ctz_fiber_swap(ctz_fiber_context* from, const ctz_fiber_context* to);

} // extern "C"

NAMESPACE_CTZ_BEGIN

class OSFiber {
public:
    ~OSFiber();

    CIEL_NODISCARD static std::unique_ptr<OSFiber>
    createFiberFromCurrentThread();

    CIEL_NODISCARD static std::unique_ptr<OSFiber>
    createFiber(size_t, std::function<void()>&&);

    void
    switchTo(OSFiber*) noexcept;

    OSFiber(const OSFiber&) = delete;
    OSFiber(OSFiber&&)      = delete;
    // clang-format off
    OSFiber& operator=(const OSFiber&) = delete;
    OSFiber& operator=(OSFiber&&)      = delete;
    // clang-format on

private:
    friend class Fiber;

    OSFiber() noexcept = default;

    static void
    run(OSFiber*);

    ctz_fiber_context context{};
    std::function<void()> target;
    void* stack{nullptr};

}; // class OSFiber

NAMESPACE_CTZ_END

#endif

#endif // CTZ_OSFIBER_ASM_H_
