#include <cstdio>
#include <ctz/waitgroup.h>

int main() {
    ctz::Scheduler::start(ctz::SchedulerConfig::allCores());
    CIEL_DEFER({ ctz::Scheduler::stop(); });

    // __________________________________________________________
    // |                                                          |
    // |               ---> [task B] ----                         |
    // |             /                    \                       |
    // |  [task A] -----> [task A: wait] -----> [task A: resume]  |
    // |             \                    /                       |
    // |               ---> [task C] ----                         |
    // |__________________________________________________________|

    ctz::WaitGroup a_wg(1);

    // task A
    ctz::Scheduler::schedule([=] {
        CIEL_DEFER({ a_wg.done(); }); // Decrement a_wg when task A is done

        puts("1");

        // Create a WaitGroup for waiting on task B and C to finish.
        // This has an initial count of 2 (B + C)
        ctz::WaitGroup bc_wg(2);

        // task B
        ctz::Scheduler::schedule([=] { // schedule inside Scheduler, the reason to use Scheduler::bind()
            CIEL_DEFER({ bc_wg.done(); });
            puts("2");
        });

        // task C
        ctz::Scheduler::schedule([=] {
            CIEL_DEFER({ bc_wg.done(); });
            puts("3");
        });

        // Wait for tasks B and C to finish.
        bc_wg.wait();

        puts("4");
    });

    // Wait for task A (and so B and C) to finish.
    a_wg.wait();
}
