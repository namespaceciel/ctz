#ifndef CTZ_CONFIG_H
#define CTZ_CONFIG_H

#include <ciel/core/config.hpp>

// disable exceptions

#ifdef CIEL_HAS_EXCEPTIONS
#  error "Exceptions are disabled since we use Fiber as the implementation of stackful symmetric coroutines."
#endif

// namespace ctz

#define NAMESPACE_CTZ_BEGIN namespace ctz {
#define NAMESPACE_CTZ_END   }

#endif // CTZ_CONFIG_H
