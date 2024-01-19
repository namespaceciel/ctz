#include <cmath>
#include <cstdio>
#include <ctz/ticket.h>

bool isPrime(int i) {
    auto c = static_cast<int>(std::sqrt(i));

    for (int j = 2; j <= c; j++) {
        if (i % j == 0) {
            return false;
        }
    }

    return true;
}

int main() {
    // Choose how many cores needed, allCores() will pick all of them.
    ctz::Scheduler scheduler(ctz::SchedulerConfig::allCores());
    scheduler.bind(); // Bound to global, check out waitgroup_example for reasons

    // CIEL_DEFER wrap the expression into a lambda, storing it to ciel::finally,
    // when it reaches out of scope, the destructor of ciel::finally will call that expression earlier.
    CIEL_DEFER({ scheduler.unbind(); });

    ctz::TicketQueue queue;

    for (int searchBase = 1; searchBase <= 100000; searchBase += 1000) {
        // Take a Ticket being in queue.
        auto ticket = queue.take();

        ctz::schedule([=] { // lambda capture by value, all tools are implemented as shared_ptr, guaranteeing the
                            // lifetime correctness.
            // Multiple tasks calculate primes at the same time, storing them into vector
            ciel::vector<int> primes;
            for (int i = searchBase; i < searchBase + 1000; ++i) {
                if (isPrime(i)) {
                    primes.push_back(i);
                }
            }

            // Blocked here, waiting for last task's done,
            // this thread will process other tasks then.
            ticket->wait();

            for (auto prime : primes) {
                printf("%d is prime\n", prime);
            }

            // Tell next task to go.
            ticket->done();
        });
    }

    // Blocked here, waiting for all of their done.
    queue.take()->wait();

    return 0;
}
