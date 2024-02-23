#ifndef CTZ_TICKET_H_
#define CTZ_TICKET_H_

#include <ctz/conditionvariable.h>
#include <ctz/config.h>

#include <ciel/list.hpp>

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
    ciel::list<std::shared_ptr<Ticket>>::iterator iter;
    TicketQueue* const queue;
    bool isReady{false};

};  // class Ticket

class TicketQueue {
public:
    CIEL_NODISCARD std::shared_ptr<Ticket> take();

private:
    friend class Ticket;

    ciel::list<std::shared_ptr<Ticket>> queue;
    std::mutex mutex;

};  // class TicketQueue

NAMESPACE_CTZ_END

#endif // CTZ_TICKET_H_