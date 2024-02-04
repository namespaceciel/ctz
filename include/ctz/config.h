#ifndef CTZ_CONFIG_H
#define CTZ_CONFIG_H

// debug level
#if !defined(NDEBUG) || defined(DCHECK_ALWAYS_ON)
#define CTZ_DEBUG_ENABLED 1
#else
#define CTZ_DEBUG_ENABLED 0
#endif

// namespace ctz
#define NAMESPACE_CTZ_BEGIN namespace ctz {
#define NAMESPACE_CTZ_END }

NAMESPACE_CTZ_BEGIN

void fatal(const char*, ...);
void warn(const char*, ...);

void nop() noexcept;

#if CTZ_DEBUG_ENABLED

#define CTZ_FATAL(msg, ...) ctz::fatal(msg "\n", ##__VA_ARGS__);

#define CTZ_ASSERT(cond, msg, ...)                   \
do {                                                \
    if (!(cond)) {                                  \
        CTZ_FATAL("ASSERT: " msg, ##__VA_ARGS__);   \
    }                                               \
} while (false)

#define CTZ_WARN(msg, ...) ctz::warn("WARNING: " msg "\n", ##__VA_ARGS__);

#else

#define CTZ_FATAL(msg, ...)
#define CTZ_ASSERT(cond, msg, ...)
#define CTZ_WARN(msg, ...)

#endif

NAMESPACE_CTZ_END

#endif // CTZ_CONFIG_H