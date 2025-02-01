#include <gtest/gtest.h>

#include <atomic>
#include <ciel/core/finally.hpp>
#include <ctz/scheduler.hpp>
#include <ctz/ticket.hpp>

TEST(TicketTest, all) {
    ctz::Scheduler::start(ctz::SchedulerConfig::allCores());
    CIEL_DEFER({ ctz::Scheduler::stop(); });

    ctz::TicketQueue queue;

    constexpr int count   = 1000;
    std::atomic<int> next = {0};
    int result[count]     = {};

    for (int i = 0; i < count; ++i) {
        auto ticket = queue.take();

        ctz::Scheduler::schedule([ticket, i, &result, &next] {
            ticket->wait();
            result[next++] = i;
            ticket->done();
        });
    }

    queue.take()->wait();

    for (int i = 0; i < count; ++i) {
        ASSERT_EQ(result[i], i);
    }
}
