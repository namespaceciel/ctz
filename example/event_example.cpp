#include <cstdio>
#include <ctz/event.hpp>

int main() {
    ctz::Scheduler::start(ctz::SchedulerConfig::allCores());
    CIEL_DEFER({ ctz::Scheduler::stop(); });

    ctz::Event B;
    ctz::Event C;

    auto il = {B, C};

    ctz::Event A = ctz::Event::any(il.begin(), il.end());

    ctz::Scheduler::schedule([=] {
        A.wait(); // Wait for B or C's done
        puts("A");
    });

    ctz::Scheduler::schedule([=] {
        CIEL_DEFER({ B.signal(); });
        puts("B");
    });

    ctz::Scheduler::schedule([=] {
        CIEL_DEFER({ C.signal(); });
        puts("C");
    });
}
