#ifndef CTZ_EVENT_H_
#define CTZ_EVENT_H_

#include <ctz/conditionvariable.h>
#include <ctz/config.h>

#include <ciel/small_vector.hpp>

#include <memory>
#include <mutex>

NAMESPACE_CTZ_BEGIN

class Event {
public:
    enum class Mode : uint8_t {
        Auto, Manual

    };  // enum class Mode

    Event(Mode mode = Mode::Auto, bool initialState = false);

    void signal() const;

    void clear() const;

    void wait() const;

    // return true if the event is signaled.
    // In auto mode, test() will clear the state, while isSignalled won't.
    bool test() const noexcept;
    bool isSignalled() const noexcept;

    // Construct a new Event, insert it into each one of the ranges' deps.
    // When one of them is signaled, this new Event will be signaled too.
    template<class Iter>
    static Event any(Mode mode, Iter begin, Iter end) {
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
    static Event any(Iter begin, Iter end) {
        return any(Mode::Auto, begin, end);
    }

private:
    struct Shared {
        Shared(Mode mode, bool initialState) noexcept;

        void signal();

        void wait();

        ciel::small_vector<std::shared_ptr<Shared>, 1> deps;
        ConditionVariable cv;
        std::mutex mutex;
        const Mode mode;
        bool signalled;

    };  // struct Shared

    const std::shared_ptr<Shared> shared;

};  // class Event

NAMESPACE_CTZ_END

#endif // CTZ_EVENT_H_