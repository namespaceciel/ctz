#ifndef CTZ_CONFIG_H
#define CTZ_CONFIG_H

// 在 debug 模式下可以用 CTZ_ASSERT(condition, "message"); 进行运行时检查

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

void fatal(const char* msg, ...);
void warn(const char* msg, ...);

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