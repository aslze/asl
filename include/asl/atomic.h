// Copyright(c) 1999-2025 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_ATOMIC_H
#define ASL_ATOMIC_H
#include "defs.h"

#ifndef __has_builtin
 #define __has_builtin(X) 0
#endif

#if defined ASL_THREAD_UNSAFE

inline int atomicInc(volatile int* x) { return ++*x; }
inline int atomicDec(volatile int* x) { return --*x; }

#elif defined _WIN32

#include <windows.h>

inline int atomicInc(volatile int* x) { return InterlockedIncrement((long*)(x)); }
inline int atomicDec(volatile int* x) { return InterlockedDecrement((long*)(x)); }

#elif __has_builtin(__sync_add_and_fetch) || (defined(__GNUC__) && ASL_C_VER >= 40102)

inline int atomicInc(int volatile* x) { return __sync_add_and_fetch(x, 1); }
inline int atomicDec(int volatile* x) { return __sync_sub_and_fetch(x, 1); }

// gcc >= 4.7 ?
//inline int atomicInc(int volatile* x) { return __atomic_add_fetch(x, 1, __ATOMIC_RELAXED); }
//inline int atomicDec(int volatile* x) { return __atomic_sub_fetch(x, 1, __ATOMIC_RELAXED); }

#else
#define ASL_NO_ATOMIC_OPS
#include "Mutex.h"
#endif

namespace asl {

class AtomicCount
{
#ifdef ASL_NO_ATOMIC_OPS
	Mutex mutex;
#endif
	volatile int n;
public:
	AtomicCount() : n(0) {}
	AtomicCount(int m) : n(m) {}
#ifdef ASL_NO_ATOMIC_OPS
	AtomicCount(const AtomicCount& a) : n(a.n) {}
	void operator=(const AtomicCount& a) { n = a.n; }
	void operator=(int m) { n = m; }
	int operator++() { Lock _(mutex); return ++n; }
	int operator--() { Lock _(mutex); return --n; }
#else
	int operator++() { return atomicInc(&n); }
	int operator--() { return atomicDec(&n); }
#endif
	operator int() const { return n; }
	bool operator==(int m) const { return n == m; }
	bool operator<(int m) const { return n < m; }
	bool operator>(int m) const { return n > m; }
	bool operator<=(int m) const { return n <= m; }
};

}

#endif
