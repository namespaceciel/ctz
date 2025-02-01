#ifndef CTZ_TICKET_H_
#define CTZ_TICKET_H_

#include <ciel/experimental/list.hpp>
#include <ciel/shared_ptr.hpp>
#include <ctz/conditionvariable.hpp>
#include <ctz/config.hpp>
#include <memory>
#include <mutex>

NAMESPACE_CTZ_BEGIN

class TicketQueue;

class Ticket {
public:
    Ticket(TicketQueue*) noexcept;

    void wait();

    // Make sure isDone = true and next node (if exist) isReady = true won't be disrupted.
    void done() noexcept;

private:
    friend class TicketQueue;

    ConditionVariable cv;
    std::mutex mutex;
    ciel::list<ciel::shared_ptr<Ticket>>::iterator iter;
    TicketQueue* const queue;
    bool isReady{false};

}; // class Ticket

class TicketQueue {
public:
    CIEL_NODISCARD ciel::shared_ptr<Ticket> take();

private:
    friend class Ticket;

    ciel::list<ciel::shared_ptr<Ticket>> queue;
    std::mutex mutex;

}; // class TicketQueue

NAMESPACE_CTZ_END

#endif // CTZ_TICKET_H_
