#ifndef CTZ_FIBER_H_
#define CTZ_FIBER_H_

#include <ctz/config.h>

#include <functional>
#include <memory>

// OSFiber is an internal implementation detail, and is not exposed in the public API.
#if defined(_WIN32)
#include <ctz/osfiber_windows.h>
#else
#include <ctz/osfiber_asm.h>
#endif

NAMESPACE_CTZ_BEGIN

class Worker;

class Fiber {
public:
    // Must only be called on the currently executing fiber.
    void
    switchTo(Fiber*);

    Fiber(const Fiber&) = delete;
    Fiber(Fiber&&)      = delete;
    // clang-format off
    Fiber& operator=(const Fiber&) = delete;
    Fiber& operator=(Fiber&&)      = delete;
    // clang-format on

    Worker* const worker;

private:
    friend class Worker;
    friend class ConditionVariable;

    CIEL_NODISCARD static std::unique_ptr<Fiber>
    create(Worker*, size_t, std::function<void()>&&);

    CIEL_NODISCARD static std::unique_ptr<Fiber>
    createFromCurrentThread(Worker*);

    Fiber(Worker*, std::unique_ptr<OSFiber>&&);

    static thread_local Fiber* current;

    const std::unique_ptr<OSFiber> impl;

}; // class Fiber

NAMESPACE_CTZ_END

#endif // CTZ_FIBER_H_
