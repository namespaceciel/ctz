#include <ciel/shared_ptr.hpp>
#include <cstddef>
#include <ctz/config.hpp>
#include <ctz/waitgroup.hpp>
#include <mutex>

NAMESPACE_CTZ_BEGIN

// Data
Data::Data(size_t initialCount) noexcept
    : count(initialCount) {}

// WaitGroup
WaitGroup::WaitGroup(size_t initialCount)
    : data(ciel::make_shared<Data>(initialCount)) {}

void WaitGroup::add(size_t num) const noexcept {
    data->count.fetch_add(num, std::memory_order_relaxed);
}

bool WaitGroup::done() const {
    if (data->count.fetch_sub(1, std::memory_order_relaxed) == 1) {
        const std::lock_guard<std::mutex> ul(data->mutex);

        data->cv.notify_all();
        return true;
    }

    return false;
}

void WaitGroup::wait() const {
    std::unique_lock<std::mutex> ul(data->mutex);

    data->cv.wait(ul, [this] {
        return data->count.load(std::memory_order_relaxed) == 0;
    });
}

NAMESPACE_CTZ_END
