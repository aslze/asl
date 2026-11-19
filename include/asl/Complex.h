// Copyright(c) 1999-2026 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_COMPLEX_H
#define ASL_COMPLEX_H

#include <asl/defs.h>
#include <math.h>

namespace asl {

/**
Complex numbers.
*/
template<class T>
class Complex_
{
public:
	Complex_() {}
	/** Creates a complex number from a real number (imaginary = 0) */
	Complex_(T x) : r(x), i(0) {}
	/** Creates a complex number given real and imaginary parts */
	Complex_(T r_, T i_) : r(r_), i(i_) {}
	Complex_(const Complex_& z) : r(z.r), i(z.i) {}
	static Complex_ zero() { return Complex_(T(0), T(0)); }
	/** Creates a complex number in polar form given a modulus and argument (angle) */
	static Complex_ polar(T m, T t) {return Complex_(m*cos(t), m*sin(t));}
	/** Returns exp(i * x) where x is a real number*/
	friend Complex_ exp_i(T t) {return Complex_(cos(t), sin(t));}
	/** Returns *this * exp(i * x) where x is a real number */
	Complex_ exp_i(T t) const { T c = cos(t), s = sin(t); return Complex_(r*c - i*s, i*c + r*s); }
	/** Returns exp(z) where z is a complex number */
	friend Complex_ exp(const Complex_& z) {return ::exp(z.r)*Complex_(cos(z.i), sin(z.i));}
	/** Returns the argument (angle) of this number if polar form */
	T angle() const {return atan2(i, r);}
	/** Returns the magnitude this number */
	T operator!() const { return sqrt(r*r + i*i); }
	/** Returns the magnitude this number */
	T magnitude() const {return sqrt(r*r+i*i);}
	/** Returns the magnitude this number squared */
	T magnitude2() const {return r*r+i*i;}
	/** Returns the complex conjugate of this number */
	Complex_ conj() const {return Complex_(r,-i);}
	Complex_ operator~() const {return conj();}

	void operator=(const Complex_& z) {r=z.r; i=z.i;}
	Complex_ operator+(const Complex_& z) const {return Complex_(r+z.r, i+z.i);}
	Complex_ operator-(const Complex_& z) const {return Complex_(r-z.r, i-z.i);}
	Complex_ operator*(const Complex_& z) const {return Complex_(r*z.r-i*z.i, r*z.i+i*z.r);}
	Complex_ operator*(T x) const {return Complex_(r*x, i*x);}
	friend Complex_ operator*(T x, const Complex_& z) {return Complex_(z.r*x, z.i*x);}
	Complex_ operator/(T x) const {T q=1./x; return Complex_(r*q, i*q);}
	Complex_ operator/(const Complex_& z) const { Complex_ q = ~(z / z.magnitude2()); return (*this)*q; }
	bool operator==(const Complex_& z) const {return r==z.r && i==z.i;}
	bool operator!=(const Complex_& z) const {return r!=z.r || i!=z.i;}
	void operator+=(const Complex_& z) {r += z.r; i += z.i;}
	void operator-=(const Complex_& z) {r -= z.r; i -= z.i;}
	void operator*=(T x) {r *= x; i *= x;}
	void operator/=(T x) {r /= x; i /= x;}
	Complex_ operator-() const {return Complex_(-r, -i);}
public:
	T r, i;
};

typedef Complex_<float> Complex;
typedef Complex_<double> Complexd;

}
#endif
