#include <ctz/config.h>

#include <cstdarg>
#include <cstdio>
#include <cstdlib>

NAMESPACE_CTZ_BEGIN

void
fatal(const char* msg, ...) {
    va_list vararg;
    va_start(vararg, msg);
    vfprintf(stderr, msg, vararg);
    va_end(vararg);
    abort();
}

void
warn(const char* msg, ...) {
    va_list vararg;
    va_start(vararg, msg);
    vfprintf(stdout, msg, vararg);
    va_end(vararg);
}

void
nop() noexcept {
    __asm__ __volatile__("nop");
}

NAMESPACE_CTZ_END
