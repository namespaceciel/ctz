#include <ciel/shared_ptr.hpp>
#include <cstddef>
#include <ctz/config.h>
#include <ctz/waitgroup.h>
#include <mutex>

NAMESPACE_CTZ_BEGIN

// Data
Data::Data(size_t initialCount) noexcept
    : count(initialCount) {}

// WaitGroup
WaitGroup::WaitGroup(size_t initialCount)
    : data(ciel::make_shared<Data>(initialCount)) {}

void WaitGroup::add(size_t num) const noexcept {
    data->count += num;
}

bool WaitGroup::done() const {
    if (--data->count == 0) {
        const std::lock_guard<std::mutex> ul(data->mutex);

        data->cv.notify_all();
        return true;
    }

    return false;
}

void WaitGroup::wait() const {
    std::unique_lock<std::mutex> ul(data->mutex);

    data->cv.wait(ul, [this] {
        return data->count == 0;
    });
}

NAMESPACE_CTZ_END
