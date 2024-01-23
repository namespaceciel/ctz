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
void ctz_main_fiber_init(ctz_fiber_context* ctx);
#else
inline void ctz_main_fiber_init(ctz_fiber_context*) {}
#endif
extern void ctz_fiber_set_target(ctz_fiber_context*,
                                void* stack,
                                uint32_t stack_size,
                                void (*target)(void*),
                                void* arg);

extern void ctz_fiber_swap(ctz_fiber_context* from, const ctz_fiber_context* to);

}  // extern "C"

NAMESPACE_CTZ_BEGIN

class OSFiber {
public:
    ~OSFiber();

    static std::unique_ptr<OSFiber> createFiberFromCurrentThread();

    // 返回一个带任务的纤程，switchTo 到这个纤程后会自动启动任务。（但是 switchTo 是用汇编写的所以并没有看懂为什么会自动启动）
    // 任务最后必须 switchTo 到另一个纤程，而且不要写 return。
    static std::unique_ptr<OSFiber> createFiber(size_t stackSize, const std::function<void()>& func);

    void switchTo(OSFiber*);

private:
    static void run(OSFiber* self);

    ctz_fiber_context context;
    std::function<void()> target;
    void* stack;

};  // class OSFiber

NAMESPACE_CTZ_END

#endif

#endif // CTZ_OSFIBER_ASM_H_