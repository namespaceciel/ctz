#include <ctz/event.h>

NAMESPACE_CTZ_BEGIN

// Event::Shared
Event::Shared::Shared(const Mode m, const bool initialState) noexcept
    : mode(m), signalled(initialState) {}

void
Event::Shared::signal() {
    std::lock_guard<std::mutex> lg(mutex);

    if (signalled) {
        return;
    }
    signalled = true;

    if (mode == Mode::Auto) {
        cv.notify_one();

    } else {
        cv.notify_all();
    }

    for (auto& dep : deps) {
        dep->signal();
    }
}

void
Event::Shared::wait() {
    std::unique_lock<std::mutex> ul(mutex);

    cv.wait(ul, [&] {
        return signalled;
    });

    if (mode == Mode::Auto) {
        signalled = false;
    }
}

// Event
Event::Event(const Mode mode, const bool initialState)
    : shared(std::make_shared<Shared>(mode, initialState)) {}

void
Event::signal() const {
    shared->signal();
}

void
Event::clear() const {
    std::lock_guard<std::mutex> lg(shared->mutex);
    shared->signalled = false;
}

void
Event::wait() const {
    shared->wait();
}

CIEL_NODISCARD bool
Event::test() const noexcept {
    std::lock_guard<std::mutex> lg(shared->mutex);

    if (!shared->signalled) {
        return false;
    }

    if (shared->mode == Mode::Auto) {
        shared->signalled = false;
    }
    return true;
}

CIEL_NODISCARD bool
Event::isSignalled() const noexcept {
    std::lock_guard<std::mutex> lg(shared->mutex);
    return shared->signalled;
}

NAMESPACE_CTZ_END
