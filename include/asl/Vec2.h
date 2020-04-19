// Copyright(c) 1999-2019 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_VECTOR2_H
#define ASL_VECTOR2_H

#include <asl/defs.h>
#include <math.h>

namespace asl {

/**
A Vec2 represents a vector in 2D space.

This class allows operating with vectors as with primitive types via operators.

~~~
Vec2 a (10, 10);
Vec2 b (-5, 1.5);
Vec2 c = -a.rotate(PI/4) + 2.0 * (b - a).normalized();
~~~

The angle between two vectors can be computed with:
~~~
T angle = acos(a/!a * b/!b);
~~~
\ingroup Math3D
*/
template<class T>
class Vec2_
{
 public:
	Vec2_() {}
	/** Constructs a vector with the given (x, y) coordinates */
	Vec2_(T x, T y): x(x), y(y) {}
	Vec2_(const Vec2_& v): x(v.x), y(v.y) {}
	operator const T* () const { return (T*)this; }
	/** Returns a vector constructed from polar coordinates */
	static Vec2_ polar(T r, T a) {return Vec2_(r*cos(a), r*sin(a));}
	/** Returns this vector rotated 90 degrees counterclockwise. */
	Vec2_ perpend() const {return Vec2_(-y, x);}
	/** Returns this vector rotated an angle `a`. */
	Vec2_ rotate(T a) const {T s=sin(a), c=cos(a); return Vec2_(c*x-s*y, s*x+c*y);}
	/** Returns this vector normalized. */
	Vec2_ normalized() const {return (*this)/length();}
	/** Returns the angle between this vector and the +X axis in the [-PI, PI] range */
	T angle()     const {return (T) atan2(y, x);}
	/** Returns the length of this vector */
	T operator!() const {return sqrt(x*x+y*y);}
	/** Returns the length of this vector */
	T length()    const {return sqrt(x*x+y*y);}
	/** Returns the length of this vector squared */
	T length2()   const {return x*x+y*y;}
	T norm1()     const {return (T)(fabs(x)+fabs(y));}
	T normInf()   const {return (T)max(fabs((T)x), fabs((T)y));}

	/** Assings `b` to this vector */
	void operator=(const Vec2_& b) {x=b.x; y=b.y;}
	/** Returns this vector plus `b` */
	Vec2_ operator+(const Vec2_& b) const {return Vec2_(x+b.x, y+b.y);}
	/** Returns this vector minus `b` */
	Vec2_ operator-(const Vec2_& b) const {return Vec2_(x-b.x, y-b.y);}
	/** Returns the dot product of this vector and `b` */
	T operator*(const Vec2_& b) const {return x*b.x+y*b.y;}
	/** Returns this vector multiplied by scalar `r` */
	Vec2_ operator*(T r) const {return Vec2_(x*r, y*r);}
	/** Returns vector `b` multiplied by scalar `r` */
	friend Vec2_ operator*(T r, const Vec2_& b) {return Vec2_(b.x*r, b.y*r);}
	/** Returns the z-component of the cross product of this vector and `b` */
	T operator^(const Vec2_& b) const {return x*b.y-y*b.x;}
	/** Returns this vector divided by scalar `r` */
	Vec2_ operator/(T r) const {T q=1/r; return Vec2_(x*q, y*q);}
	/** Compares this vector and `b` (warning: test equality of floats with care) */
	bool operator==(const Vec2_& b) const {return x==b.x && y==b.y;}
	/** Compares this vector and `b` (warning: test equality of floats with care) */
	bool operator!=(const Vec2_& b) const {return x!=b.x || y!=b.y;}
	/** Adds `b` to this vector */
	void operator+=(const Vec2_& b) {x += b.x; y += b.y;}
	/** Subtracts `b` from this vector */
	void operator-=(const Vec2_& b) {x -= b.x; y -= b.y;}
	/** Multiplies this vector by scalar `r` */
	void operator*=(T r) {x *= r; y *= r;}
	/** Divides this vector by scalar `r` */
	void operator/=(T r) {x /= r; y /= r;}
	/** Returns this vector negated */
	Vec2_ operator-() const {return Vec2_(-x, -y);}
 public:
	T x, y;
};

template<class T>
inline int compare(const Vec2_<T>& a, const Vec2_<T>& b)
{
	if(a.x < b.x) return -1;
	else if(a.x == b.x && a.y < b.y) return -1;
	else if(a.x == b.x && a.y == b.y) return 0;
	else return 1;
}

template<class T>
inline Vec2_<T> min(const Vec2_<T>& a, const Vec2_<T>& b)
{
	return Vec2_<T>(min(a.x, b.x), min(a.y, b.y));
}

template<class T>
inline Vec2_<T> max(const Vec2_<T>& a, const Vec2_<T>& b)
{
	return Vec2_<T>(max(a.x, b.x), max(a.y, b.y));
}

template<class T>
inline const Vec2_<T> min(const Vec2_<T>& a, const Vec2_<T>& b, const Vec2_<T>& c)
{
	return min(min(a, b), c);
}

template<class T>
inline Vec2_<T> max(const Vec2_<T>& a, const Vec2_<T>& b, const Vec2_<T>& c)
{
	return max(max(a, b), c);
}

typedef Vec2_<float> Vec2;
typedef Vec2_<double> Vec2d;

}
#endif
