#include <gtest/gtest.h>

#include <ctz/event.h>
#include <ctz/scheduler.h>
#include <ctz/waitgroup.h>

#include <array>

TEST(EventTest, EventIsSignalled) {
    for (auto mode : {ctz::Event::Mode::Manual, ctz::Event::Mode::Auto}) {
        auto event = ctz::Event(mode);

        ASSERT_EQ(event.isSignalled(), false);

        event.signal();
        ASSERT_EQ(event.isSignalled(), true);

        ASSERT_EQ(event.isSignalled(), true);

        event.clear();
        ASSERT_EQ(event.isSignalled(), false);
    }
}

TEST(EventTest, EventAutoTest) {
    auto event = ctz::Event(ctz::Event::Mode::Auto);

    ASSERT_EQ(event.test(), false);

    event.signal();
    ASSERT_EQ(event.test(), true);

    ASSERT_EQ(event.test(), false);
}

TEST(EventTest, EventManualTest) {
    auto event = ctz::Event(ctz::Event::Mode::Manual);

    ASSERT_EQ(event.test(), false);

    event.signal();
    ASSERT_EQ(event.test(), true);

    ASSERT_EQ(event.test(), true);
}

TEST(EventTest, EventAutoWait) {
    ctz::Scheduler scheduler(ctz::SchedulerConfig::allCores());
    scheduler.bind();
    CIEL_DEFER({ scheduler.unbind(); });

    std::atomic<int> counter = {0};
    auto event               = ctz::Event(ctz::Event::Mode::Auto);
    auto done                = ctz::Event(ctz::Event::Mode::Auto);

    for (int i = 0; i < 3; ++i) {
        ctz::schedule([=, &counter] {
            event.wait();
            ++counter;
            done.signal();
        });
    }

    ASSERT_EQ(counter.load(), 0);

    event.signal();
    done.wait();
    ASSERT_EQ(counter.load(), 1);

    event.signal();
    done.wait();
    ASSERT_EQ(counter.load(), 2);

    event.signal();
    done.wait();
    ASSERT_EQ(counter.load(), 3);
}

TEST(EventTest, EventManualWait) {
    ctz::Scheduler scheduler(ctz::SchedulerConfig::allCores());
    scheduler.bind();
    CIEL_DEFER({ scheduler.unbind(); });

    std::atomic<int> counter = {0};
    auto event               = ctz::Event(ctz::Event::Mode::Manual);
    auto wg                  = ctz::WaitGroup(3);

    for (int i = 0; i < 3; ++i) {
        ctz::schedule([=, &counter] {
            event.wait();
            ++counter;
            wg.done();
        });
    }

    event.signal();
    wg.wait();
    ASSERT_EQ(counter.load(), 3);
}

TEST(EventTest, EventSequence) {
    ctz::Scheduler scheduler(ctz::SchedulerConfig::allCores());
    scheduler.bind();
    CIEL_DEFER({ scheduler.unbind(); });

    for (auto mode : {ctz::Event::Mode::Manual, ctz::Event::Mode::Auto}) {
        std::string sequence;
        auto eventA = ctz::Event(mode);
        auto eventB = ctz::Event(mode);
        auto eventC = ctz::Event(mode);
        auto done   = ctz::Event(mode);

        ctz::schedule([=, &sequence] {
            eventA.wait();
            sequence += "A";
            eventB.signal();
        });

        ctz::schedule([=, &sequence] {
            eventB.wait();
            sequence += "B";
            eventC.signal();
        });

        ctz::schedule([=, &sequence] {
            eventC.wait();
            sequence += "C";
            done.signal();
        });

        ASSERT_EQ(sequence, "");

        eventA.signal();
        done.wait();
        ASSERT_EQ(sequence, "ABC");
    }
}

TEST(EventTest, EventAny) {
    for (int i = 0; i < 3; ++i) {
        std::array<ctz::Event, 3> events = {
            ctz::Event(ctz::Event::Mode::Auto),
            ctz::Event(ctz::Event::Mode::Auto),
            ctz::Event(ctz::Event::Mode::Auto),
        };

        auto any = ctz::Event::any(events.begin(), events.end());

        events[i].signal();
        ASSERT_TRUE(any.isSignalled());
    }
}
