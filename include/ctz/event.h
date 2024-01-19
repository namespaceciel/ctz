#ifndef CTZ_EVENT_H_
#define CTZ_EVENT_H_

#include <ciel/shared_ptr.hpp>
#include <ciel/vector.hpp>
#include <ctz/conditionvariable.h>
#include <ctz/config.h>
#include <memory>
#include <mutex>

NAMESPACE_CTZ_BEGIN

class Event {
public:
    enum class Mode : uint8_t {
        Auto,
        Manual

    }; // enum class Mode

    Event(Mode = Mode::Auto, bool = false);

    void signal() const;

    void clear() const;

    void wait() const;

    // Return true if the event is signaled.
    // In auto mode, test() will clear the state, while isSignalled won't.
    CIEL_NODISCARD bool test() const noexcept;
    CIEL_NODISCARD bool isSignalled() const noexcept;

    // Construct a new Event, insert it into each one of the ranges' deps.
    // When one of them is signaled, this new Event will be signaled too.
    template<class Iter>
    CIEL_NODISCARD static Event any(Mode mode, Iter begin, Iter end) {
        Event any(mode, false);

        for (auto it = begin; it != end; ++it) {
            auto s = it->shared;

            std::lock_guard<std::mutex> lg(s->mutex);

            if (s->signalled) {
                any.signal();
            }

            s->deps.push_back(any.shared);
        }

        return any;
    }

    template<class Iter>
    CIEL_NODISCARD static Event any(Iter begin, Iter end) {
        return any(Mode::Auto, begin, end);
    }

private:
    struct Shared {
        Shared(Mode, bool) noexcept;

        void signal();

        void wait();

        ciel::vector<ciel::shared_ptr<Shared>> deps;
        ConditionVariable cv;
        std::mutex mutex;
        const Mode mode;
        bool signalled;

    }; // struct Shared

    const ciel::shared_ptr<Shared> shared;

}; // class Event

NAMESPACE_CTZ_END

#endif // CTZ_EVENT_H_
