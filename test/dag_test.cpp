#include <gtest/gtest.h>

#include <algorithm>
#include <ciel/core/finally.hpp>
#include <ciel/vector.hpp>
#include <ctz/dag.h>
#include <ctz/scheduler.h>
#include <functional>
#include <mutex>
#include <string>
#include <utility>

namespace {

struct Data {
    std::mutex mutex;
    ciel::vector<std::string> order;

    void push(std::string&& s) {
        const std::lock_guard<std::mutex> lg(mutex);
        order.emplace_back(std::move(s));
    }

}; // struct Data

} // namespace

// [A] --> [B] --> [C]
TEST(DAGTest, DAGChainNoArg) {
    ctz::Scheduler::start(ctz::SchedulerConfig::allCores());
    CIEL_DEFER({ ctz::Scheduler::stop(); });

    ctz::DAG<>::Builder builder;

    Data data;
    builder.root()
        .then([&] {
            data.push("A");
        })
        .then([&] {
            data.push("B");
        })
        .then([&] {
            data.push("C");
        });

    auto dag = builder.build();
    dag->run();

    ASSERT_EQ(data.order, ciel::vector<std::string>({"A", "B", "C"}));
}

// [A] --> [B] --> [C]
TEST(DAGTest, DAGChain) {
    ctz::Scheduler::start(ctz::SchedulerConfig::allCores());
    CIEL_DEFER({ ctz::Scheduler::stop(); });

    ctz::DAG<Data&>::Builder builder;

    builder.root()
        .then([](Data& data) {
            data.push("A");
        })
        .then([](Data& data) {
            data.push("B");
        })
        .then([](Data& data) {
            data.push("C");
        });

    auto dag = builder.build();

    Data data;
    dag->run(data);

    ASSERT_EQ(data.order, ciel::vector<std::string>({"A", "B", "C"}));
}

// [A] --> [B] --> [C]
TEST(DAGTest, DAGRunRepeat) {
    ctz::Scheduler::start(ctz::SchedulerConfig::allCores());
    CIEL_DEFER({ ctz::Scheduler::stop(); });

    ctz::DAG<Data&>::Builder builder;

    builder.root()
        .then([](Data& data) {
            data.push("A");
        })
        .then([](Data& data) {
            data.push("B");
        })
        .then([](Data& data) {
            data.push("C");
        });

    auto dag = builder.build();

    Data dataA, dataB;
    dag->run(dataA);
    dag->run(dataB);
    dag->run(dataA);

    ASSERT_EQ(dataA.order, ciel::vector<std::string>({"A", "B", "C", "A", "B", "C"}));
    ASSERT_EQ(dataB.order, ciel::vector<std::string>({"A", "B", "C"}));
}

/*
           /--> [A]
  [root] --|--> [B]
           \--> [C]
*/
TEST(DAGTest, DAGFanOutFromRoot) {
    ctz::Scheduler::start(ctz::SchedulerConfig::allCores());
    CIEL_DEFER({ ctz::Scheduler::stop(); });

    ctz::DAG<Data&>::Builder builder;

    auto root = builder.root();
    root.then([](Data& data) {
        data.push("A");
    });
    root.then([](Data& data) {
        data.push("B");
    });
    root.then([](Data& data) {
        data.push("C");
    });

    auto dag = builder.build();

    Data data;
    dag->run(data);

    ciel::vector<std::string> tmp({"A", "B", "C"});
    ASSERT_EQ(data.order.size(), tmp.size());
    ASSERT_TRUE(std::is_permutation(data.order.begin(), data.order.end(), tmp.begin()));
}

/*
                /--> [A]
 [root] -->[N]--|--> [B]
                \--> [C]
*/
TEST(DAGTest, DAGFanOutFromNonRoot) {
    ctz::Scheduler::start(ctz::SchedulerConfig::allCores());
    CIEL_DEFER({ ctz::Scheduler::stop(); });

    ctz::DAG<Data&>::Builder builder;

    auto root = builder.root();
    auto node = root.then([](Data& data) {
        data.push("N");
    });
    node.then([](Data& data) {
        data.push("A");
    });
    node.then([](Data& data) {
        data.push("B");
    });
    node.then([](Data& data) {
        data.push("C");
    });

    auto dag = builder.build();

    Data data;
    dag->run(data);

    ciel::vector<std::string> tmp({"N", "A", "B", "C"});
    ASSERT_EQ(data.order.size(), tmp.size());
    ASSERT_TRUE(std::is_permutation(data.order.begin(), data.order.end(), tmp.begin()));
    ASSERT_EQ(data.order[0], "N");
}

/*
          /--> [A0] --\        /--> [C0] --\        /--> [E0] --\
 [root] --|--> [A1] --|-->[B]--|--> [C1] --|-->[D]--|--> [E1] --|-->[F]
                               \--> [C2] --/        |--> [E2] --|
                                                    \--> [E3] --/
*/
TEST(DAGTest, DAGFanOutFanIn) {
    ctz::Scheduler::start(ctz::SchedulerConfig::allCores());
    CIEL_DEFER({ ctz::Scheduler::stop(); });

    ctz::DAG<Data&>::Builder builder;

    auto root = builder.root();
    auto a0   = root.then([](Data& data) {
        data.push("A0");
    });
    auto a1   = root.then([](Data& data) {
        data.push("A1");
    });

    auto b = builder.node(
        [](Data& data) {
            data.push("B");
        },
        {a0, a1});

    auto c0 = b.then([](Data& data) {
        data.push("C0");
    });
    auto c1 = b.then([](Data& data) {
        data.push("C1");
    });
    auto c2 = b.then([](Data& data) {
        data.push("C2");
    });

    auto d = builder.node(
        [](Data& data) {
            data.push("D");
        },
        {c0, c1, c2});

    auto e0 = d.then([](Data& data) {
        data.push("E0");
    });
    auto e1 = d.then([](Data& data) {
        data.push("E1");
    });
    auto e2 = d.then([](Data& data) {
        data.push("E2");
    });
    auto e3 = d.then([](Data& data) {
        data.push("E3");
    });

    builder.node(
        [](Data& data) {
            data.push("F");
        },
        {e0, e1, e2, e3});

    auto dag = builder.build();

    Data data;
    dag->run(data);

    ciel::vector<std::string> tmp({"A0", "A1", "B", "C0", "C1", "C2", "D", "E0", "E1", "E2", "E3", "F"});

    ASSERT_EQ(data.order.size(), tmp.size());
    ASSERT_TRUE(std::is_permutation(data.order.begin(), data.order.end(), tmp.begin()));

    ASSERT_TRUE(std::is_permutation(data.order.begin() + 0, data.order.begin() + 2, tmp.begin() + 0));

    ASSERT_EQ(data.order[2], "B");

    ASSERT_TRUE(std::is_permutation(data.order.begin() + 3, data.order.begin() + 6, tmp.begin() + 3));

    ASSERT_EQ(data.order[6], "D");

    ASSERT_TRUE(std::is_permutation(data.order.begin() + 7, data.order.begin() + 11, tmp.begin() + 7));

    ASSERT_EQ(data.order[11], "F");
}

TEST(DAGTest, DAGForwardFunc) {
    ctz::DAG<void>::Builder builder;

    const std::function<void()> func([] {});
    ASSERT_TRUE(func);

    auto a = builder.root().then(func).then(func);

    builder.node(func, {a});

    ASSERT_TRUE(func);
}
