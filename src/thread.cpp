#include <ctz/thread.h>

#include <algorithm>
#include <thread>
#include <unistd.h>
#include <unordered_set>
#include <vector>

namespace std {

template<>
struct hash<ctz::Core> {
    size_t operator()(ctz::Core core) const noexcept {
        return core.pthread.index;
    }
};

}   // namespace std

NAMESPACE_CTZ_BEGIN

// Core
bool Core::operator==(Core other) const noexcept {
    return pthread.index == other.pthread.index;
}

bool Core::operator<(Core other) const noexcept {
    return pthread.index < other.pthread.index;
}

// Affinity
Affinity::Affinity() noexcept = default;

Affinity::Affinity(const Affinity&) = default;

Affinity::Affinity(Affinity&&) noexcept = default;

Affinity& Affinity::operator=(Affinity&&) noexcept = default;

Affinity::Affinity(std::initializer_list<Core> il) : cores(il) {}

Affinity::Affinity(const ciel::small_vector<Core, 32>& cs) : cores(cs) {}

Affinity Affinity::all() {
    Affinity res;

#if defined(_WIN32)
    const auto& groups = getProcessorGroups();
    for (size_t groupIdx = 0; groupIdx < groups.count; groupIdx++) {
        const auto& group = groups.groups[groupIdx];
        Core core;
        core.windows.group = static_cast<decltype(Core::windows.group)>(groupIdx);
        for (unsigned int coreIdx = 0; coreIdx < group.count; coreIdx++) {
            if ((group.affinity >> coreIdx) & 1) {
                core.windows.index = static_cast<decltype(core.windows.index)>(coreIdx);
                affinity.cores.emplace_back(std::move(core));
            }
        }
    }
#elif defined(__linux__) && !defined(__ANDROID__) && !defined(__BIONIC__)
    auto thread = pthread_self();
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    if (pthread_getaffinity_np(thread, sizeof(cpu_set_t), &cpuset) == 0) {
        int count = CPU_COUNT(&cpuset);
        for (int i = 0; i < count; i++) {
            Core core;
            core.pthread.index = static_cast<uint16_t>(i);
            affinity.cores.emplace_back(std::move(core));
        }
    }
#elif defined(__FreeBSD__)
    auto thread = pthread_self();
    cpuset_t cpuset;
    CPU_ZERO(&cpuset);
    if (pthread_getaffinity_np(thread, sizeof(cpuset_t), &cpuset) == 0) {
        int count = CPU_COUNT(&cpuset);
        for (int i = 0; i < count; i++) {
            Core core;
            core.pthread.index = static_cast<uint16_t>(i);
            affinity.cores.emplace_back(std::move(core));
        }
    }
#else
    static_assert(!supported, "Affinity::supported is true, but Affinity::all() is not implemented for this platform");
#endif

    return res;
}

size_t Affinity::count() const noexcept {
    return cores.size();
}

Core Affinity::operator[](size_t index) const noexcept {
    return cores[index];
}

Affinity& Affinity::add(const Affinity& other) {
    std::unordered_set<Core> set;
    for (auto core : cores) {
        set.emplace(core);
    }

    for (auto core : other.cores) {
        if (set.find(core) == set.end()) {
            cores.push_back(core);
        }
    }

    std::sort(cores.begin(), cores.end());

    return *this;
}

Affinity& Affinity::remove(const Affinity& other) {
    std::unordered_set<Core> set;
    for (auto core : other.cores) {
        set.emplace(core);
    }

    for (size_t i = 0; i < cores.size(); ++i) {
        if (set.find(cores[i]) != set.end()) {
            cores[i] = cores.back();
            cores.pop_back();
        }
    }

    std::sort(cores.begin(), cores.end());

    return *this;
}

std::shared_ptr<Policy> Policy::anyOf(Affinity&& affinity) {

    struct NewPolicy : public Policy {

        Affinity affinity;
        NewPolicy(Affinity&& affinity) : affinity(std::move(affinity)) {}

        Affinity get(uint32_t ThreadID) const override {
#if defined(_WIN32)
            const auto count = affinity.count();
            if (count == 0) {
                return Affinity(affinity);
            }

            auto group = affinity[ThreadID % count].windows.group;
            Affinity out;
            out.cores.reserve(count);

            for (auto core : affinity.cores) {
                if (core.windows.group == group) {
                    out.cores.push_back(core);
                }
            }

            return out;
#else
            return Affinity(affinity);
#endif
        }

    };  // struct Policy

    return std::make_shared<NewPolicy>(std::move(affinity));
}

std::shared_ptr<Policy> Policy::oneOf(Affinity&& affinity) {

    struct NewPolicy : public Policy {

        Affinity affinity;
        NewPolicy(Affinity&& affi) : affinity(std::move(affi)) {}

        Affinity get(uint32_t ThreadID) const override {
            const auto count = affinity.count();
            if (count == 0) {
                return Affinity(affinity);
            }

            return Affinity({affinity[ThreadID % count]});
        }

    };  // struct Policy

    return std::make_shared<NewPolicy>(std::move(affinity));
}

#if defined(_WIN32)

class Thread::Impl {
public:
    Impl(Function&& f, _PROC_THREAD_ATTRIBUTE_LIST* attributes)
        : func(std::move(f)),
          handle(CreateRemoteThreadEx(GetCurrentProcess(),
                                      nullptr,
                                      0,
                                      &Impl::run,
                                      this,
                                      0,
                                      attributes,
                                      nullptr)) {}

    ~Impl() { CloseHandle(handle); }

    Impl(const Impl&) = delete;
    Impl(Impl&&) = delete;
    Impl& operator=(const Impl&) = delete;
    Impl& operator=(Impl&&) = delete;

    void Join() const { WaitForSingleObject(handle, INFINITE); }

    static DWORD WINAPI run(void* self) {
        reinterpret_cast<Impl*>(self)->func();
        return 0;
    }

private:
    const Function func;
    const HANDLE handle;
};

Thread::Thread(Affinity&& affinity, Func&& func) {
    SIZE_T size = 0;
    InitializeProcThreadAttributeList(nullptr, 1, 0, &size);
    CTZ_ASSERT(size > 0, "InitializeProcThreadAttributeList() did not give a size");

    std::vector<uint8_t> buffer(size);
    LPPROC_THREAD_ATTRIBUTE_LIST attributes = reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(buffer.data());
    CHECK_WIN32(InitializeProcThreadAttributeList(attributes, 1, 0, &size));
    CIEL_DEFER(DeleteProcThreadAttributeList(attributes));

    GROUP_AFFINITY groupAffinity = {};

    auto count = affinity.count();
    if (count > 0) {
        groupAffinity.Group = affinity[0].windows.group;

        for (size_t i = 0; i < count; i++) {
            auto core = affinity[i];
            CTZ_ASSERT(groupAffinity.Group == core.windows.group, "Cannot create thread that uses multiple affinity groups");
            groupAffinity.Mask |= (1ULL << core.windows.index);
        }

        CHECK_WIN32(UpdateProcThreadAttribute(attributes, 0, PROC_THREAD_ATTRIBUTE_GROUP_AFFINITY, &groupAffinity,
                                              sizeof(groupAffinity), nullptr, nullptr));
    }

    impl = new Impl(std::move(func), attributes);
}

Thread::~Thread() {
    delete impl;
}

void Thread::join() {
    CTZ_ASSERT(impl != nullptr, "join() called on unjoinable thread");
    impl->Join();

    // TODO: 为什么跟下面的实现不一致
}

unsigned int Thread::numLogicalCPUs() {
    unsigned int count = 0;
    const auto& groups = getProcessorGroups();

    for (size_t groupIdx = 0; groupIdx < groups.count; groupIdx++) {
        const auto& group = groups.groups[groupIdx];
        count += group.count;
    }

    return count;
}

#else

class Thread::Impl {
public:
    Impl(Affinity&& affinity, Function&& f)
        : affinity(std::move(affinity)), func(std::move(f)), thread([this] {
              setAffinity();
              func();
          }) {}
    
    Affinity affinity;
    Function func;
    std::thread thread;

    void setAffinity() {
        const auto count = affinity.count();
        if (count == 0) {
            return;
        }

#if defined(__linux__) && !defined(__ANDROID__) && !defined(__BIONIC__)
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        for (size_t i = 0; i < count; i++) {
            CPU_SET(affinity[i].pthread.index, &cpuset);
        }
        auto thread = pthread_self();
        pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
#elif defined(__FreeBSD__)
        cpuset_t cpuset;
        CPU_ZERO(&cpuset);
        for (size_t i = 0; i < count; i++) {
            CPU_SET(affinity[i].pthread.index, &cpuset);
        }
        auto thread = pthread_self();
        pthread_setaffinity_np(thread, sizeof(cpuset_t), &cpuset);
#else
        CTZ_ASSERT(!Affinity::supported, "Attempting to use thread affinity on a unsupported platform");
#endif
    }

};  // class Thread::Impl

Thread::Thread(Affinity&& affinity, Function&& func)
    : impl(new Thread::Impl(std::move(affinity), std::move(func))) {}

Thread::~Thread() {
    CTZ_ASSERT(impl == nullptr, "Thread::join() was not called before destruction");
}

void Thread::join() {
    CTZ_ASSERT(impl != nullptr, "Thread::join() was called on an empty Thread");

    impl->thread.join();
    delete impl;
    impl = nullptr;
}

unsigned int Thread::numLogicalCPUs() {
    return static_cast<unsigned int>(sysconf(_SC_NPROCESSORS_ONLN));
}

#endif

Thread::Thread() noexcept = default;

Thread::Thread(Thread&& other) noexcept : impl(other.impl) {
    other.impl = nullptr;
}

Thread& Thread::operator=(Thread&& other) noexcept {
    if (impl) {
        delete impl;
    }
    
    impl = other.impl;
    other.impl = nullptr;
    return *this;
}

NAMESPACE_CTZ_END