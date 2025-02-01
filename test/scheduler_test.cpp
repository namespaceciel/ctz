#include <gtest/gtest.h>

#include <atomic>
#include <ciel/core/finally.hpp>
#include <cstddef>
#include <ctz/scheduler.h>
#include <memory>

TEST(SchedulerTest, SingleThreadCounter) {
    std::atomic<size_t> counter = {0};

    {
        ctz::Scheduler::start(ctz::SchedulerConfig{});
        CIEL_DEFER({ ctz::Scheduler::stop(); });

        for (int i = 0; i < 1000; ++i) {
            ctz::Scheduler::schedule([&] {
                ++counter;
            });
        }
    }

    ASSERT_EQ(counter.load(), 1000);
}

TEST(SchedulerTest, MultipleThreadCounter) {
    std::atomic<size_t> counter = {0};

    {
        ctz::Scheduler::start(ctz::SchedulerConfig::allCores());
        CIEL_DEFER({ ctz::Scheduler::stop(); });

        for (int i = 0; i < 1000; ++i) {
            ctz::Scheduler::schedule([&] {
                ++counter;
            });
        }
    }

    ASSERT_EQ(counter.load(), 1000);
}

TEST(SchedulerTest, TasksInTasks) {
    std::atomic<size_t> counter = {0};

    {
        ctz::Scheduler::start(ctz::SchedulerConfig::allCores());
        CIEL_DEFER({ ctz::Scheduler::stop(); });

        for (int i = 0; i < 500; ++i) {
            ctz::Scheduler::schedule([&] {
                ++counter;

                ctz::Scheduler::schedule([&] {
                    ++counter;
                });
            });
        }
    }

    ASSERT_EQ(counter.load(), 1000);
}
