#include <ciel/core/config.hpp>
#include <ciel/shared_ptr.hpp>
#include <ctz/config.hpp>
#include <ctz/ticket.hpp>
#include <mutex>

NAMESPACE_CTZ_BEGIN

// Ticket
Ticket::Ticket(TicketQueue* q) noexcept
    : queue(q) {}

void Ticket::wait() {
    std::unique_lock<std::mutex> ul(mutex);

    cv.wait(ul, [this] {
        return isReady;
    });
}

void Ticket::done() noexcept {
    const std::lock_guard<std::mutex> lg(queue->mutex);

    auto next = iter.next();
    if (next != queue->queue.end()) {
        (*next)->isReady = true;
        (*next)->cv.notify_one();
    }

    // Since each Ticket is shared_ptr, we can guarantee this object is valid before return.
    queue->queue.erase(iter);
}

// TicketQueue
CIEL_NODISCARD ciel::shared_ptr<Ticket> TicketQueue::take() {
    const std::lock_guard<std::mutex> lg(mutex);

    auto it = queue.insert(queue.end(), ciel::make_shared<Ticket>(this));

    if (it == queue.begin()) {
        (*it)->isReady = true;
    }

    (*it)->iter = it;
    return *it;
}

NAMESPACE_CTZ_END
