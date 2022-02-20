// ASL - All-purpose Simple Library
// Copyright(c) 1999-2022 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_DEFS_H
#define ASL_DEFS_H

/*! \file

Main definitions.
*/

#define ASL_VERSION 11003

#ifdef _WIN32
#ifndef _CRT_SECURE_NO_DEPRECATE
 #define _CRT_SECURE_NO_DEPRECATE
#endif
#ifndef _CRT_SECURE_NO_WARNINGS
 #define _CRT_SECURE_NO_WARNINGS
#endif
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#undef BIGENDIAN
#undef LITTLEENDIAN
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 1900)
#include <cmath>
namespace asl {
	using std::cos; using std::sin; using std::tan; using std::floor; using std::sqrt; using std::pow; using std::acos;
	using std::fmod; using std::exp;
}
#else
#include <math.h>
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef ASL_NOEXCEPT
#include <new>
#define ASL_BAD_ALLOC() throw std::bad_alloc()
#else
#define ASL_BAD_ALLOC() asl::asl_die("Out of memory in " __FILE__, __LINE__)
#endif

//#define ASL_HAVE_MOVE

#ifdef _MSC_VER
#define ASL_C_VER _MSC_VER
#elif defined __clang__
#define ASL_C_VER (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__)
#elif defined __GNUC__
#define ASL_C_VER (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#elif defined __INTEL_COMPILER
#define ASL_C_VER __INTEL_COMPILER
#else
#define ASL_C_VER 0
#endif

#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
#define ASL_GCC11
#endif

#ifndef __has_feature
 #define __has_feature(x) 0
#endif

#if __has_feature(cxx_auto_type) || (defined( _MSC_VER ) && _MSC_VER >= 1600) || (defined(__GNUC__) && ASL_C_VER >= 40400)
#define ASL_HAVE_AUTO
#endif

#if __has_feature(cxx_lambdas) || (defined( _MSC_VER ) && _MSC_VER >= 1600) || (defined(__GNUC__) && defined(ASL_GCC11) && ASL_C_VER >= 40500)
#define ASL_HAVE_LAMBDA
#endif

#if __has_feature(cxx_generalized_initializers) || (defined( _MSC_VER ) && _MSC_VER >= 1800) || (defined(__GNUC__) && defined(ASL_GCC11)  && ASL_C_VER >= 40503)
#define ASL_HAVE_INITLIST
#endif

#if __has_feature(cxx_range_for) || (defined( _MSC_VER ) && _MSC_VER >= 1700) || (defined(__GNUC__) && defined(ASL_GCC11)  && ASL_C_VER >= 40600)
#define ASL_HAVE_RANGEFOR
#endif

#if !defined(_WIN32) || defined(ASL_STATIC)
 #define ASL_API
#elif defined(asl_EXPORTS)
  #define ASL_API __declspec(dllexport)
#else
  #define ASL_API __declspec(dllimport)
#endif

#ifdef _WIN32
#define ASL_EXPORT __declspec(dllexport)
#define ASL_PATH_SEP '\\'
#else
#define ASL_EXPORT
#define ASL_PATH_SEP '/'
#endif

/**
Check that the argument is true.
@hideinitializer
*/
#define ASL_ASSERT(x) if(!(x)) { printf("\n%s: %i\n\n* Failed: '%s'\n\n", __FILE__, __LINE__, #x); exit(1); }

#ifdef _MSC_VER
#define ASL_DEPRECATED(x, m) __declspec(deprecated("Deprecated. " ## m)) x
#elif defined(__GNUC__) && ASL_C_VER < 40503
#define ASL_DEPRECATED(x, m) x __attribute__((deprecated))
#else
#define ASL_DEPRECATED(x, m) x __attribute__((deprecated(m)))
#endif


namespace asl {

typedef unsigned char byte;

struct Exception {};

/**
\defgroup Global Global functions
@{
*/

#if defined _WIN32 && !defined GCC
	typedef __int64 Long;
	typedef unsigned __int64 ULong;
#else
	typedef long long Long;
	typedef unsigned long long ULong;
#endif

extern double ASL_API bigval;

/**
Returns +infinity
*/
inline float infinity()
{
	return float(bigval);
}

/**
Returns a NaN value
*/
inline float nan()
{
	static const float n = infinity() / infinity();
	return n;
}

/** Returns `x` squared */
template <class T>
inline T sqr(T x) {return x*x;}

/** Returns the fractional part of `x` */
template <class T>
inline T fract(T x) {return x - floor(x);}

/** Clamps the value of x to make it lie inside the [a,b] interval */
template <class T, class C>
inline T clamp(T x, C a, C b) { const T t = x < a ? a : x; return t > b ? b : t; }

/** Returns `x` degrees converted to radians */
template <class T>
inline T deg2rad(T x) {return (T)(x*0.017453292519943295);}

/** Returns `x` radians converted to degrees */
template <class T>
inline T rad2deg(T x) {return (T)(x*57.29577951308232);}

/**@}*/

static const double PI = 3.14159265358979323;

/**
Endianness types for binary parsing/writing
*/
enum Endian { ENDIAN_BIG, ENDIAN_LITTLE, ENDIAN_NATIVE };

#ifndef ASL_BIGENDIAN
#define ASL_OTHER_ENDIAN ENDIAN_BIG
#else
#define ASL_OTHER_ENDIAN ENDIAN_LITTLE
#endif

/**
A random number generator.

Generates uniformly distributed pseudo-random numbers, except in the `normal()` functions.

```
Random random;
int n = random(256);                // get an integer between 0 and 255
double x = random(-1.5, 1.5);       // get a number between -1.5 and +1.5
double y = random.normal(10, 0.75); // get a number from a normal distribution
```

The generator initially is seeded pseudorandomly. If you need a constant sequence you can create with
a false argument, to prevent this or call seed().

For compatibility with older code, there is a global `asl::random` object already random initialized
ready for use. But it is recommended to use new Random objects when separate sequences or multithreading
are needed.
*/
class ASL_API Random
{
	ULong _state[4];
	static const int _size;
public:
	Random(bool autoseed = true, bool fast = true);
	/** Returns an integer pseudo-random number in the [0, 2^32-1] interval */
	unsigned get();

	ULong getLong();

	/** Returns a floating point random number in the [0, m] interval */
	double operator()(double m) { return m * 1.1102230246251565e-16 * (getLong() >> 11); } // 0x1.0p-53

	/** Returns a floating point random number in the [m, M] interval */
	double operator()(double m, double M) { return m + (*this)(M - m); }
	
	/** Returns a floating point random number in the [0, M] interval */
	float operator()(float m) { return (float)(*this)((double)m); }

	/** Returns a floating point random number in the [m, M] interval */
	float operator()(float m, float M) { return (float)(*this)((double)m, (double)M); }

	/** Returns an integer random number in the [0, M] interval */
	template<class T>
	T operator()(T m) { return (T)(*this)((double)m + 1); }

	/** Returns an integer random number in the [m, M] interval */
	template<class T>
	T operator()(T m, T M) { return (T)(*this)((double)m, (double)M + 1); }

	/** Returns a floating point random number with standard normal distribution */
	double normal() { double u = (*this)(1e-30, 1.0), v = (*this)(1e-30, 1.0); return sqrt(-2 * log(u))*cos(2 * PI * v); }

	/** Returns a floating point random number with normal distribution with given mean and variance */
	double normal(double m, double s2) { return m + s2 * normal(); }

	/** Returns true or false given a probability (by default it is 0.5, like flipping a coin) */
	bool coin(double p = 0.5) { return (*this)(1.0) < p; }

	/** Initializes the seed for the random functions */
	void seed(ULong s);

	/** Initializes the seed for the random functions randomly (set fast=false for a high quality random seed) */
	void init(bool fast = true);

	/** Fills a buffer with OS-provided random bytes or pseudo-random if that fails */
	static void getBytes(void* buffer, int n);
};

extern ASL_API Random random; //!< A global random number generator

ASL_API int myatoi(const char* s);

ASL_API int myatoiz(const char* s);

ASL_API Long myatol(const char* s);

ASL_API int myltoa(Long x, char* s);

ASL_API double myatof(const char* s);

ASL_API int myitoa(int x, char* s);

inline bool myisspace(char c)
{
	return c <= ' ' && (c == ' ' || c == '\n' || c == '\r' || c == '\t');
}

inline bool myisalnum(char c)
{
	return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9');
}


void asl_die(const char* msg, int line = 0);

void asl_error(const char* msg);

void os_error(const char* msg);

template <class T>
T bytesSwapped(const T& x)
{
	T y;
	const byte* px = (const byte*)&x;
	byte* py = (byte*)&y;
	for (int i = 0; i < sizeof(T); i++)
		py[i] = px[sizeof(T) - i - 1];
	return y;
}

template <class T>
void swapBytes(T& x)
{
	T y = x;
	x = bytesSwapped(y);
}

#undef min
#undef max
template <class T>
inline T max(T a, T b) {if(a>b) return a; else return b;}

template <class T>
inline T min(T a, T b) {if(a<b) return a; else return b;}

template <class T>
inline void vswap(T& a, T& b) {T A=a; a=b; b=A;}

template <class T>
inline void swap(T& a, T& b)
{
	char t[sizeof(T)];
	memcpy(t, &a, sizeof(T));
	memcpy(&a, &b, sizeof(T));
	memcpy(&b, t, sizeof(T));
}

}

inline void* operator new (size_t, int* p) { return p; }
inline void operator delete(void* p, int* r) {}

namespace asl {

// Placement constructors

template <class T>
inline void asl_construct(T* p) {new ((int*)p) T;}

template <class T>
inline void asl_construct_copy(T* p, const T& x) {new ((int*)p) T(x);}

template <class T>
inline void asl_construct(T* p, int n) {T* q=p+n; while(p!=q) {new ((int*)p) T; p++;}}

template <class T>
inline void asl_destroy(T* p) {p->~T();}

template <class T>
inline void asl_destroy(T* p, int n) {T* q=p+n; while(p!=q) {p->~T(); p++;}}

// Placement constructors for pointers
#if !defined _MSC_VER || _MSC_VER > 11600

template <class T>
inline void asl_construct(T** p) {}

template <class T>
inline void asl_construct(T** p, int n) {}

template <class T>
inline void asl_destroy(T** p) {}

template <class T>
inline void asl_destroy(T** p, int n) {}
#endif

#define ASL_POD_CONSTRUCT(T) \
inline void asl_construct(T* p) {}\
inline void asl_construct(T* p, int n) {}\
inline void asl_destroy(T* p) {}\
inline void asl_destroy(T* p, int n) {}\
inline void asl_construct_copy(T* p, const T& x) {*p = x;} \
inline void bswap(T& a, T& b) {swap(a, b);}

ASL_POD_CONSTRUCT(byte)
ASL_POD_CONSTRUCT(char)
ASL_POD_CONSTRUCT(int)
ASL_POD_CONSTRUCT(unsigned int)
ASL_POD_CONSTRUCT(float)
ASL_POD_CONSTRUCT(double)
ASL_POD_CONSTRUCT(short)
ASL_POD_CONSTRUCT(unsigned short)
ASL_POD_CONSTRUCT(Long)
ASL_POD_CONSTRUCT(ULong)

template<class T, class F>
struct IsLess {
	IsLess(const F& f) : f(f) {}
	bool operator()(const T& a, const T& b) const { return f(a) < f(b); }
	F f;
};

template<class T, class F>
struct IsMore {
	IsMore(const F& f) : f(f) {}
	bool operator()(const T& a, const T& b) const { return f(b) < f(a); }
	F f;
};

}

#ifndef ASL_NO_GLOBAL_BYTE
using asl::byte;
#endif

#include "time.h"
#include "atomic.h"

#endif
