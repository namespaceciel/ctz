#if defined(_WIN32)

#include <ctz/osfiber_windows.h>

NAMESPACE_CTZ_BEGIN

OSFiber::~OSFiber() {
    if (fiber != nullptr) {
        if (isFiberFromThread) {
            ConvertFiberToThread();
        } else {
            DeleteFiber(fiber);
        }
    }
}

std::unique_ptr<OSFiber> OSFiber::createFiberFromCurrentThread() {
    auto out = std::unique_ptr<OSFiber>(new OSFiber);

    out->fiber = ConvertThreadToFiberEx(nullptr, FIBER_FLAG_FLOAT_SWITCH);
    out->isFiberFromThread = true;
    CTZ_ASSERT(out->fiber != nullptr, "ConvertThreadToFiberEx() failed with error 0x%x", int(GetLastError()));
    return out;
}

std::unique_ptr<OSFiber> OSFiber::createFiber(size_t stackSize, const std::function<void()>& func) {
    auto out = std::unique_ptr<OSFiber>(new OSFiber);

    // stackSize is rounded up to the system's allocation granularity (typically 64 KB).
    out->fiber = CreateFiberEx(stackSize - 1, stackSize, FIBER_FLAG_FLOAT_SWITCH, &OSFiber::run, out.get());
    out->target = func;
    CTZ_ASSERT(out->fiber != nullptr, "CreateFiberEx() failed with error 0x%x", int(GetLastError()));
    return out;
}

void OSFiber::switchTo(OSFiber* to) {
    SwitchToFiber(to->fiber);
}

void WINAPI OSFiber::run(void* self) {
    std::function<void()> func;
    std::swap(func, reinterpret_cast<OSFiber*>(self)->target);
    func();
}

NAMESPACE_CTZ_END

#endif