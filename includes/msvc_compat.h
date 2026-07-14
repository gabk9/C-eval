#ifndef CEVAL_MSVC_COMPAT_H
#define CEVAL_MSVC_COMPAT_H

/* Neutralize GNU-style attributes that cause syntax errors on MSVC */
#define __attribute__(x)

/* Provide ssize_t for MSVC builds */
#include <stdint.h>
typedef int64_t ssize_t;

#endif /* CEVAL_MSVC_COMPAT_H */
