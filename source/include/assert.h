#ifndef ASSERT_H
#define ASSERT_H

#include "types.h"

#undef assert

#ifdef ASSERT

void debug(const u8 *file, const u16 line);
#define assert(x) ((x) ? (void)0 : debug(__FILE__, __LINE__))

#elif defined(DEBUG)

#define assert(x) { if (x) { /* nothing */ } else { __asm__("brk"); } }

#else

#define assert(x);

#endif

#define assert_index(a, idx) assert((idx) < sizeof((a)) / sizeof(*(a)));
#define fail() assert(0)

#endif // ASSERT_H
