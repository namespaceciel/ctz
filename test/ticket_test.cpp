#include <gtest/gtest.h>

#include <atomic>
#include <ciel/core/finally.hpp>
#include <ctz/scheduler.h>
#include <ctz/ticket.h>

TEST(TicketTest, all) {
    ctz::Scheduler scheduler(ctz::SchedulerConfig::allCores());
    scheduler.bind();
    CIEL_DEFER({ scheduler.unbind(); });

    ctz::TicketQueue queue;

    constexpr int count   = 1000;
    std::atomic<int> next = {0};
    int result[count]     = {};

    for (int i = 0; i < count; ++i) {
        auto ticket = queue.take();

        ctz::schedule([ticket, i, &result, &next] {
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
