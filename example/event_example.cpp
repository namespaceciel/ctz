#include <cstdio>
#include <ctz/event.h>

int main() {
    ctz::Scheduler scheduler(ctz::SchedulerConfig::allCores());
    scheduler.bind();
    CIEL_DEFER({ scheduler.unbind(); });

    ctz::Event B;
    ctz::Event C;

    auto il = {B, C};

    ctz::Event A = ctz::Event::any(il.begin(), il.end());

    ctz::schedule([=] {
        A.wait(); // Wait for B or C's done
        puts("A");
    });

    ctz::schedule([=] {
        CIEL_DEFER({ B.signal(); });
        puts("B");
    });

    ctz::schedule([=] {
        CIEL_DEFER({ C.signal(); });
        puts("C");
    });
}
