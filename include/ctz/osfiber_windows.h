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

    CIEL_NODISCARD static std::unique_ptr<OSFiber>
    createFiberFromCurrentThread();

    CIEL_NODISCARD static std::unique_ptr<OSFiber>
    createFiber(size_t, std::function<void()>&&);

    void
    switchTo(OSFiber*);

    OSFiber(const OSFiber&) = delete;
    OSFiber(OSFiber&&)      = delete;
    // clang-format off
    OSFiber& operator=(const OSFiber&) = delete;
    OSFiber& operator=(OSFiber&&)      = delete;
    // clang-format on

private:
    friend class Fiber;

    OSFiber() noexcept = default;

    inline static void WINAPI
    run(void*);

    LPVOID fiber{nullptr};
    bool isFiberFromThread{false};
    std::function<void()> target;

}; // class OSFiber

NAMESPACE_CTZ_END

#endif

#endif // CTZ_OSFIBER_WINDOWS_H_
