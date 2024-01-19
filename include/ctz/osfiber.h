#ifndef CTZ_OSFIBER_H_
#define CTZ_OSFIBER_H_

#if defined(__x86_64__)
#  include <ctz/osfiber_x64.h>
#elif defined(__aarch64__)
#  include <ctz/osfiber_aarch64.h>
#elif defined(__arm__)
#  include <ctz/osfiber_arm.h>
#else
#  error "Unsupported target"
#endif

#include <cstdint>
#include <ctz/config.h>
#include <functional>
#include <memory>

extern "C" {

extern void ctz_fiber_set_target(ctz_fiber_context*, void*, uint32_t, void (*)(void*), void*);

extern void ctz_fiber_swap(ctz_fiber_context*, const ctz_fiber_context*);

} // extern "C"

NAMESPACE_CTZ_BEGIN

class OSFiber {
public:
    CIEL_NODISCARD static std::unique_ptr<OSFiber> createFiberFromCurrentThread();

    CIEL_NODISCARD static std::unique_ptr<OSFiber> createFiber(size_t, std::function<void()>&&);

    void switchTo(OSFiber*) noexcept;

    OSFiber(const OSFiber&)            = delete;
    OSFiber& operator=(const OSFiber&) = delete;

    ~OSFiber();

private:
    friend class Fiber;

    OSFiber() noexcept = default;

    static void run(OSFiber*);

    ctz_fiber_context context{};
    std::function<void()> target;
    void* stack{nullptr};

}; // class OSFiber

NAMESPACE_CTZ_END

#endif // CTZ_OSFIBER_H_
