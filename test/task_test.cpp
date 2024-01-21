#include <gtest/gtest.h>

#include <ctz/task.h>

TEST(task_test, all) {
    ctz::Task t1{};

    ctz::Task t2{[]() {}};
    t2();

    size_t num = 0;
    ctz::Task t3{[&]() {
        for (size_t i = 0; i < 100; ++i) {
            num += i;
        }
    }};
    t3();

    ASSERT_EQ(num, 4950);
}