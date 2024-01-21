#include <gtest/gtest.h>

#include <ctz/thread.h>

namespace {

ctz::Core core(int idx) {
    ctz::Core c;
    c.pthread.index = static_cast<uint16_t>(idx);
    return c;
}

} // namespace

TEST(thread_test, ThreadAffinityCount) {
    auto affinity = ctz::Affinity({core(10), core(20), core(30), core(40)});
    
    ASSERT_EQ(affinity.count(), 4);
}

TEST(thread_test, ThreadAdd) {
    auto affinity = ctz::Affinity({core(10), core(20), core(30), core(40)});
    
    affinity.add(ctz::Affinity({core(25), core(15)}))
           .add(ctz::Affinity({core(35)}));

    ASSERT_EQ(affinity.count(), 7U);
    ASSERT_EQ(affinity[0], core(10));
    ASSERT_EQ(affinity[1], core(15));
    ASSERT_EQ(affinity[2], core(20));
    ASSERT_EQ(affinity[3], core(25));
    ASSERT_EQ(affinity[4], core(30));
    ASSERT_EQ(affinity[5], core(35));
    ASSERT_EQ(affinity[6], core(40));
}

TEST(thread_test, ThreadRemove) {
    auto affinity = ctz::Affinity({core(10), core(20), core(30), core(40)});

    affinity.remove(ctz::Affinity({core(25), core(20)}))
           .remove(ctz::Affinity({core(40)}));

    ASSERT_EQ(affinity.count(), 2U);
    ASSERT_EQ(affinity[0], core(10));
    ASSERT_EQ(affinity[1], core(30));
}

TEST(thread_test, ThreadAffinityAllCountNonzero) {
    auto affinity = ctz::Affinity::all();
    
    if (ctz::Affinity::supported) {
        ASSERT_NE(affinity.count(), 0U);
        
    } else {
        ASSERT_EQ(affinity.count(), 0U);
    }
}

TEST(thread_test, ThreadAffinityFromVector) {
    ciel::small_vector<ctz::Core, 32> cores;
    cores.push_back(core(10));
    cores.push_back(core(20));
    cores.push_back(core(30));
    cores.push_back(core(40));
    
    auto affinity = ctz::Affinity(cores);
    ASSERT_EQ(affinity.count(), cores.size());
    ASSERT_EQ(affinity[0], core(10));
    ASSERT_EQ(affinity[1], core(20));
    ASSERT_EQ(affinity[2], core(30));
    ASSERT_EQ(affinity[3], core(40));
}

TEST(thread_test, ThreadAffinityPolicyOneOf) {
    auto all = ctz::Affinity({core(10), core(20), core(30), core(40)});

    auto policy = ctz::Policy::oneOf(std::move(all));
    ASSERT_EQ(policy->get(0).count(), 1U);
    ASSERT_EQ(policy->get(0)[0].pthread.index, 10);
    ASSERT_EQ(policy->get(1).count(), 1U);
    ASSERT_EQ(policy->get(1)[0].pthread.index, 20);
    ASSERT_EQ(policy->get(2).count(), 1U);
    ASSERT_EQ(policy->get(2)[0].pthread.index, 30);
    ASSERT_EQ(policy->get(3).count(), 1U);
    ASSERT_EQ(policy->get(3)[0].pthread.index, 40);
}