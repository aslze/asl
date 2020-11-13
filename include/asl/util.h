// Copyright(c) 1999-2020 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_UTIL_H
#define ASL_UTIL_H

#include <asl/String.h>
#include <asl/Array.h>

namespace asl {

template<class T, int N>
class Array_;

/**
\defgroup Global Global functions
@{
*/

ASL_API Array<byte> decodeBase64(const char* src, int n = -1);

/**
Decodes a base64 encoded string into a byte array; the string can contain whitespace.
*/
inline Array<byte> decodeBase64(const String& s)
{
	return decodeBase64((const char*)s, s.length());
}

ASL_API String encodeBase64(const byte* data, int n);

/**
Encodes a byte array as a string using base64 encoding.
*/
inline String encodeBase64(const Array<byte>& s)
{
	return encodeBase64(s.ptr(), s.length());
}

/**
Encodes a string as a string using base64 encoding.
*/
inline String encodeBase64(const String& s)
{
	return encodeBase64((const byte*)&s[0], s.length());
}


template<int N>
String encodeBase64(const Array_<byte,N>& src) { return encodeBase64((const byte*)src, N); }

ASL_API String encodeHex(const byte* data, int n);

/**
Encodes a byte array as a string using hexadecimal
*/
inline String encodeHex(const Array<byte>& src) { return encodeHex(src.ptr(), src.length()); }

template<int N>
String encodeHex(const Array_<byte, N>& src) { return encodeHex((const byte*)src, N); }

/**
Decodes a hexadecimal encoded string into a byte array
*/
ASL_API Array<byte> decodeHex(const String& src);

/**@}*/

struct NoType {};

template<class R, class T1, class T2 = NoType>
struct FuncB
{
	virtual ~FuncB() {}
	virtual R operator()() { return R(); }
	virtual R operator()(T1 x) { return R(); }
	virtual R operator()(T1 x, T2 y) { return R(); }
};

template<class F, class R, class T1, class T2>
struct Func : FuncB<R, T1, T2>
{
	Func(const F& f) : f(f) {}
	R operator()(T1 x, T2 y) { return f(x, y); }
	F f;
};

template<class F, class R, class T1>
struct Func<F, R, T1, NoType> : FuncB<R, T1, NoType>
{
	Func(const F& f) : f(f) {}
	R operator()(T1 x) { return f(x); }
	F f;
};

template<class F, class R>
struct Func<F, R, NoType, NoType> : FuncB<R, NoType, NoType>
{
	Func(const F& f) : f(f) {}
	R operator()() { return f(); }
	F f;
};

#define ASL_FUNC(T1, T2) \
	Function() : f(NULL) {} \
	Function(const Function& f) : f(f.f) { ((Function&)f).f = NULL; } \
	void operator=(const Function& ff) { f = ff.f; ((Function&)ff).f = NULL; } \
	operator bool() const { return f != 0; } \
	template<class F> \
	Function(const F& f) : f(new Func<F, R, T1, T2>(f)) {} \
	~Function() { delete f; } \
	FuncB<R, T1, T2>* f;

/**
A simple function object that can wrap a function pointer, a functor or a lambda. The first
parameter is the return type, which can be `void`, and the rest, if given, are argument types.
Currently supports up to 2 arguments.

In C++11 you can do this (in older C++ you would assign a function pointer
or a class object implementing `operator()`):

~~~
Function<bool, int> isEven = [=](int x) { return x % 2 == 0; };

bool sixeven = isEven(6);
~~~

A default constructed Function object is null and cannot be invoked. You can check if the function
has been initialized with the `bool` conversion:

~~~
if (function)
	function(a, b);
~~~

Be aware that currently Function objects copied invalidate the source object. This might change in the future.
*/
template<class R, class T1 = NoType, class T2 = NoType>
struct Function {
	ASL_FUNC(T1, T2)
	R operator()(T1 x, T2 y) { return (*f)(x, y); }
};

template<class R, class T1>
struct Function<R, T1, NoType> {
	ASL_FUNC(T1, NoType)
	R operator()(T1 x) { return (*f)(x); }
};

template<class R>
struct Function<R, NoType, NoType> {
	ASL_FUNC(NoType, NoType)
	R operator()() { return (*f)(); }
};

}

#endif
