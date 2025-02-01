#ifndef CTZ_FIBER_H_
#define CTZ_FIBER_H_

#include <ctz/config.hpp>
#include <ctz/osfiber.hpp>
#include <functional>
#include <memory>

NAMESPACE_CTZ_BEGIN

class Worker;

class Fiber {
public:
    // Must only be called on the currently executing fiber.
    void switchTo(Fiber*);

    Fiber(const Fiber&)            = delete;
    Fiber& operator=(const Fiber&) = delete;

    Worker* const worker;

private:
    friend class Worker;
    friend class ConditionVariable;

    Fiber(Worker*, std::unique_ptr<OSFiber>&&);

    CIEL_NODISCARD static std::unique_ptr<Fiber> create(Worker*, size_t, std::function<void()>&&);

    CIEL_NODISCARD static std::unique_ptr<Fiber> createFromCurrentThread(Worker*);

    static thread_local Fiber* current;

    const std::unique_ptr<OSFiber> impl;

}; // class Fiber

NAMESPACE_CTZ_END

#endif // CTZ_FIBER_H_
