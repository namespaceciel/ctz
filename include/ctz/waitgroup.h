#ifndef CTZ_WAITGROUP_H_
#define CTZ_WAITGROUP_H_

#include <ctz/conditionvariable.h>
#include <ctz/config.h>

#include <atomic>
#include <cstddef>
#include <memory>

NAMESPACE_CTZ_BEGIN

struct Data {
    Data(size_t = 0) noexcept;

    ConditionVariable cv;
    std::atomic<size_t> count;
    std::mutex mutex;

}; // struct Data

class WaitGroup {
public:
    WaitGroup(size_t = 0);

    void
    add(size_t = 1) const noexcept;

    bool
    done() const;

    void
    wait() const;

private:
    const std::shared_ptr<Data> data;

}; // class WaitGroup

NAMESPACE_CTZ_END

#endif // CTZ_WAITGROUP_H_
