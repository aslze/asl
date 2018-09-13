// ASL - All-purpose Simple Library
// Copyright(c) 1999-2018 ASL author
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_DEFS_H
#define ASL_DEFS_H

/*! \file

Main definitions.
*/

#ifdef _WIN32
#ifndef _CRT_SECURE_NO_DEPRECATE
 #define _CRT_SECURE_NO_DEPRECATE
#endif
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
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
#include "time.h"

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

#define ASL_ASSERT(x) if(!(x)) { printf("\n%s: %i\n\n* Failed: '%s'\n\n", __FILE__, __LINE__, #x); exit(EXIT_FAILURE); }

typedef unsigned char byte;

#include "atomic.h"

namespace asl {

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

/**
Returns a NaN value
*/
inline float nan()
{
	/*double x;
	Long* bits = (Long*)&x;
	*bits = 0x7ff0000080000001LL;
	return x;*/
	return float((1e100*1e100))*0.0f;
}

/**
Returns +infinity
*/
ASL_API float infinity();

/** Returns `x` squared */
template <class T>
inline T sqr(T x) {return x*x;}

/** Returns the fractional part of `x` */
template <class T>
inline T fract(T x) {return x - floor(x);}

/** Clamps the value of x to make it lie inside the [a,b] interval */
template <class T, class C>
inline T clamp(T x, C a, C b) {if (x<a) return a; else if (x>b) return b; return x;}

/** Returns `x` degrees converted to radians */
template <class T>
inline T deg2rad(T x) {return (T)(x*0.017453292519943295);}

/** Returns `x` radians converted to degrees */
template <class T>
inline T rad2deg(T x) {return (T)(x*57.29577951308232);}

/** Returns an integer pseudo-random number in the [0, 2^31) interval */
ASL_API int random();

/** Returns a floating point random number in the [0, m] interval */
inline double random(double m)
{
	return random()*m*(1.0/((1U<<31)+1));
}

/** Returns a floating point random number in the [m, M] interval */
inline double random(double m, double M)
{
	return m + random()*(M-m)*(1.0/((1U<<31)+1));
}

inline float random(float m) {return (float)random((double)m);}

inline float random(float m, float M) {return (float)random((double)m, (double)M);}

/** Returns an integer random number in the [0, M-1] interval */
inline int random(int m) { return (int)random((double)m); }

/** Returns an integer random number in the [m, M-1] interval */
inline int random(int m, int M) { return (int)random((double)m, (double)M); }


/** Initializes the seed for the random functions */
ASL_API void random_init(Long n = -1);

/**@}*/

static const double PI = 3.14159265358979323;


ASL_API int myatoi(const char* s);

ASL_API Long myatol(const char* s);

ASL_API int myltoa(Long x, char* s);

ASL_API double myatof(const char* s);

ASL_API int myitoa(int x, char* s);

inline bool myisspace(char c)
{
	return c <= ' ' && (c == ' ' || c == '\n' || c == '\r' || c == '\t');
}

// Fatal errors (will become exceptions some day)

void asl_die(const char* msg, int line = 0);

void asl_error(const char* msg);

void os_error(const char* msg);

template <class T>
T bytesSwapped(const T& x)
{
	T y;
	byte* px = (byte*)&x;
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
//inline void* operator new (unsigned, void* p) /*throw()*/ {return p;}
//inline void operator delete(void* p) /*throw()*/ {::operator delete(p);}
inline void* operator new (size_t, int* p) /*throw()*/ {return p;}
//inline void* operator new (unsigned n) /*throw()*/ {return ::operator new(n);}
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

template <typename T>
inline void asl_fix_moved_object(T* x) {}

template <typename T>
inline void asl_fix_moved_objects(T* x, int n) {}


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

}
#endif
