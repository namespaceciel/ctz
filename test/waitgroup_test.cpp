#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <ciel/core/finally.hpp>
#include <cstddef>
#include <ctz/scheduler.h>
#include <ctz/waitgroup.h>
#include <thread>

TEST(WaitGroupTest, WaitGroup_OneTask) {
    ctz::Scheduler::start(ctz::SchedulerConfig::allCores());
    CIEL_DEFER({ ctz::Scheduler::stop(); });

    const ctz::WaitGroup wg(1);
    std::atomic<size_t> counter = {0};

    ctz::Scheduler::schedule([&counter, wg] {
        ++counter;
        wg.done();
    });

    wg.wait();

    ASSERT_EQ(counter.load(), 1);
}

TEST(WaitGroupTest, WaitGroup_10Tasks) {
    ctz::Scheduler::start(ctz::SchedulerConfig::allCores());
    CIEL_DEFER({ ctz::Scheduler::stop(); });

    const ctz::WaitGroup wg(10);
    std::atomic<size_t> counter = {0};

    for (size_t i = 0; i < 10; ++i) {
        ctz::Scheduler::schedule([&counter, wg] {
            ++counter;
            wg.done();
        });
    }

    wg.wait();

    ASSERT_EQ(counter.load(), 10);
}

TEST(WaitGroupTest, NotifyAsTask) {
    std::atomic<size_t> counter = {0};

    {
        ctz::Scheduler::start(ctz::SchedulerConfig::allCores());
        CIEL_DEFER({ ctz::Scheduler::stop(); });

        const ctz::WaitGroup wg(1);

        for (size_t i = 0; i < 1000; ++i) {
            ctz::Scheduler::schedule([&counter, wg] {
                wg.wait();
                ++counter;
            });
        }

        ctz::Scheduler::schedule([=] {
            wg.done();
        });
    }

    ASSERT_EQ(counter.load(), 1000);
}

TEST(WaitGroupTest, NotifyAsTask2) {
    std::atomic<size_t> counter = {0};

    {
        ctz::Scheduler::start(ctz::SchedulerConfig::allCores());
        CIEL_DEFER({ ctz::Scheduler::stop(); });

        const ctz::WaitGroup wg(1);

        for (size_t i = 0; i < 1000; ++i) {
            ctz::Scheduler::schedule([&counter, wg] {
                ++counter;
                wg.wait();
                ++counter;
            });
        }

        ctz::Scheduler::schedule([=] {
            wg.done();
        });
    }

    ASSERT_EQ(counter.load(), 2000);
}

TEST(WaitGroupTest, SameThread) {
    ctz::Scheduler::start(ctz::SchedulerConfig::allCores());
    CIEL_DEFER({ ctz::Scheduler::stop(); });

    const ctz::WaitGroup fence(1);
    const ctz::WaitGroup wg(1000);

    for (int i = 0; i < 1000; ++i) {
        ctz::Scheduler::schedule([=] {
            auto threadID = std::this_thread::get_id();
            fence.wait();
            ASSERT_EQ(threadID, std::this_thread::get_id());
            wg.done();
        });
    }

    // Get some tasks to yield.
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    fence.done();
    wg.wait();
}
