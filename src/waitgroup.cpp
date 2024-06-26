#include <ctz/waitgroup.h>

NAMESPACE_CTZ_BEGIN

// Data
Data::Data(const size_t initialCount) noexcept
    : count(initialCount) {}

// WaitGroup
WaitGroup::WaitGroup(const size_t initialCount)
    : data(std::make_shared<Data>(initialCount)) {}

void
WaitGroup::add(const size_t num) const noexcept {
    data->count += num;
}

bool
WaitGroup::done() const {
    if (--data->count == 0) {
        std::unique_lock<std::mutex> ul(data->mutex);

        data->cv.notify_all();
        return true;
    }

    return false;
}

void
WaitGroup::wait() const {
    std::unique_lock<std::mutex> ul(data->mutex);

    data->cv.wait(ul, [this] {
        return data->count == 0;
    });
}

NAMESPACE_CTZ_END
