#include <gtest/gtest.h>

#include <atomic>
#include <ciel/core/finally.hpp>
#include <cstddef>
#include <ctz/scheduler.h>
#include <memory>

TEST(SchedulerTest, ConstructAndDestruct) {
    auto scheduler = std::unique_ptr<ctz::Scheduler>(new ctz::Scheduler(ctz::SchedulerConfig()));
}

TEST(SchedulerTest, BindAndUnbind) {
    auto scheduler = std::unique_ptr<ctz::Scheduler>(new ctz::Scheduler(ctz::SchedulerConfig()));
    scheduler->bind();

    auto got = ctz::Scheduler::get();
    ASSERT_EQ(scheduler.get(), got);

    scheduler->unbind();
    got = ctz::Scheduler::get();
    ASSERT_EQ(got, nullptr);
}

TEST(SchedulerTest, CheckConfig) {
    ctz::SchedulerConfig cfg;
    cfg.setWorkerThreadCount(10);

    auto scheduler = std::unique_ptr<ctz::Scheduler>(new ctz::Scheduler(cfg));

    auto gotCfg = scheduler->config;
    ASSERT_EQ(gotCfg.threadCount, 10);
}

TEST(SchedulerTest, SingleThreadCounter) {
    std::atomic<size_t> counter = {0};

    {
        ctz::Scheduler scheduler(ctz::SchedulerConfig().setWorkerThreadCount(1));
        scheduler.bind();
        CIEL_DEFER({ scheduler.unbind(); });

        for (int i = 0; i < 1000; ++i) {
            ctz::schedule([&] {
                ++counter;
            });
        }
    }

    ASSERT_EQ(counter.load(), 1000);
}

TEST(SchedulerTest, MultipleThreadCounter) {
    std::atomic<size_t> counter = {0};

    {
        ctz::Scheduler scheduler(ctz::SchedulerConfig::allCores());
        scheduler.bind();
        CIEL_DEFER({ scheduler.unbind(); });

        for (int i = 0; i < 1000; ++i) {
            ctz::schedule([&] {
                ++counter;
            });
        }
    }

    ASSERT_EQ(counter.load(), 1000);
}

TEST(SchedulerTest, TasksInTasks) {
    std::atomic<size_t> counter = {0};

    {
        ctz::Scheduler scheduler(ctz::SchedulerConfig::allCores());
        scheduler.bind();
        CIEL_DEFER({ scheduler.unbind(); });

        for (int i = 0; i < 500; ++i) {
            ctz::schedule([&] {
                ++counter;

                ctz::schedule([&] {
                    ++counter;
                });
            });
        }
    }

    ASSERT_EQ(counter.load(), 1000);
}
