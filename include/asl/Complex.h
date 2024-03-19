#ifndef ASL_COMPLEX_H
#define ASL_COMPLEX_H

#include <asl/defs.h>
#include <math.h>

namespace asl {

/**
Complex numbers.
\deprecated This class name might change to match Vec2 and others (`Complex_<T>`, `Complex`, `Complexd`)
*/
template<class T = double>
class Complex
{
public:
	Complex() {}
	/** Creates a complex number from a real number (imaginary = 0) */
	Complex(T x) : r(x), i(0) {}
	/** Creates a complex number given real and imaginary parts */
	Complex(T r_, T i_) : r(r_), i(i_) {}
	Complex(const Complex& z) : r(z.r), i(z.i) {}
	/** Creates a complex number in polar form given a modulus and argument (angle) */
	static Complex polar(T m, T t) {return Complex(m*cos(t), m*sin(t));}
	/** Returns exp(i * x) where x is a real number*/
	friend Complex exp_i(T t) {return Complex(cos(t), sin(t));}
	/** Returns *this * exp(i * x) where x is a real number */
	Complex exp_i(T t) const { T c = cos(t), s = sin(t); return Complex(r*c - i*s, i*c + r*s); }
	/** Returns exp(z) where z is a complex number */
	friend Complex exp(const Complex& z) {return ::exp(z.r)*Complex(cos(z.i), sin(z.i));}
	/** Returns the argument (angle) of this number if polar form */
	T angle() const {return atan2(i, r);}
	/** Returns the magnitude this number */
	T operator!() const { return sqrt(r*r + i*i); }
	/** Returns the magnitude this number */
	T magnitude() const {return sqrt(r*r+i*i);}
	/** Returns the magnitude this number squared */
	T magnitude2() const {return r*r+i*i;}
	/** Returns the complex conjugate of this number */
	Complex conj() const {return Complex(r,-i);}
	Complex operator~() const {return conj();}

	void operator=(const Complex& z) {r=z.r; i=z.i;}
	Complex operator+(const Complex& z) const {return Complex(r+z.r, i+z.i);}
	Complex operator-(const Complex& z) const {return Complex(r-z.r, i-z.i);}
	Complex operator*(const Complex& z) const {return Complex(r*z.r-i*z.i, r*z.i+i*z.r);}
	Complex operator*(T x) const {return Complex(r*x, i*x);}
	friend Complex operator*(T x, const Complex& z) {return Complex(z.r*x, z.i*x);}
	Complex operator/(T x) const {T q=1./x; return Complex(r*q, i*q);}
	Complex operator/(const Complex& z) const { Complex q = ~(z / z.magnitude2()); return (*this)*q; }
	bool operator==(const Complex& z) const {return r==z.r && i==z.i;}
	bool operator!=(const Complex& z) const {return r!=z.r || i!=z.i;}
	void operator+=(const Complex& z) {r += z.r; i += z.i;}
	void operator-=(const Complex& z) {r -= z.r; i -= z.i;}
	void operator*=(T x) {r *= x; i *= x;}
	void operator/=(T x) {r /= x; i /= x;}
	Complex operator-() const {return Complex(-r, -i);}
public:
	T r, i;
};

typedef Complex<float> Complexf;
typedef Complex<double> Complexd;

}
#endif
