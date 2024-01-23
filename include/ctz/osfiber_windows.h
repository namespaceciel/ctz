#ifndef CTZ_OSFIBER_WINDOWS_H_
#define CTZ_OSFIBER_WINDOWS_H_

#if defined(_WIN32)

#include <ctz/config.h>

#include <functional>
#include <memory>

#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

NAMESPACE_CTZ_BEGIN

class OSFiber {
public:
    ~OSFiber();

    // createFiberFromCurrentThread() returns a fiber created from the current thread.
    static std::unique_ptr<OSFiber> createFiberFromCurrentThread();

    // createFiber() returns a new fiber with the given stack size that will
    // call func when switched to. func() must end by switching back to another
    // fiber, and must not return.
    static std::unique_ptr<OSFiber> createFiber(size_t stackSize, const std::function<void()>& func);

    // switchTo() immediately switches execution to the given fiber.
    // switchTo() must be called on the currently executing fiber.
    void switchTo(OSFiber*);

private:
    static inline void WINAPI run(void* self);
    LPVOID fiber = nullptr;
    bool isFiberFromThread = false;
    std::function<void()> target;

};  // class OSFiber

NAMESPACE_CTZ_END

#endif

#endif // CTZ_OSFIBER_WINDOWS_H_