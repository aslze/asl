// Copyright(c) 1999-2025 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_VECTOR4_H
#define ASL_VECTOR4_H

#include <asl/defs.h>
#include <asl/Vec3.h>
#include <math.h>

namespace asl {

/**
A Vec4 is a 4-dimensional vector usually representing homogenous coordinates.

It can be used together with class Matrix4 to transform vectors in projective space.

~~~
Vec4 a (10, 10, 0.1, 1.0);
Vec3 v = a.h2c();
~~~
\ingroup Math3D
*/
template<class T>
class Vec4_
{
 public:
	Vec4_() {}
	Vec4_(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}
	Vec4_(const Vec3_<T>& v, T w) : x(v.x), y(v.y), z(v.z), w(w) {}
	Vec4_(const T* v) : x(v[0]), y(v[1]), z(v[2]), w(v[3]) {}
	static Vec4_ zeros() { return Vec4_(0, 0, 0, 0); }
	template<class T2>
	Vec4_<T2> with() const
	{
		return Vec4_<T2>(T2(x), T2(y), T2(z), T2(w));
	}
	operator const T*() const {return &x;}
	/** Returns the *x*, *y*, *z* components as a Vec3 */
	Vec3_<T> xyz() const {return Vec3_<T>(x, y, z);}
	/** Returns the cartesian coordinates vector corresponding to this homogenous coordinates */
	Vec3_<T> h2c() const {T iw=1/w; return Vec3_<T>(iw*x, iw*y, iw*z);}

	/** Returns a normalized version of this vector */
	Vec4_ normalized() const {
		return *this/length();
	}
	/** Returns the length of the vector */
	T length() const {return sqrt(x*x+y*y+z*z+w*w);}
	/** Returns the length of the vector squared */
	T length2() const {return x*x+y*y+z*z+w*w;}
	/** Returns the length of the vector \deprecated */
	T operator!() const {return length();}
	/** Returns this vector with absolute coordinates */
	Vec4_ abs() const {return Vec4_(fabs(x), fabs(y), fabs(z), fabs(w));}

	/** Returns this plus `b` */
	Vec4_ operator+(const Vec4_& b) const {return Vec4_(x+b.x, y+b.y, z+b.z, w+b.w);}
	/** Returns this minus `b` */
	Vec4_ operator-(const Vec4_& b) const {return Vec4_(x-b.x, y-b.y, z-b.z, w-b.w);}
	/** Returns the *dot product* of this vector and `b` */
	T operator*(const Vec4_& b) const {return x*b.x+y*b.y+z*b.z+w*b.w;}
	/** Returns this vector multiplied by scalar `r` */
	Vec4_ operator*(T r) const {return Vec4_(x*r, y*r, z*r, w*r);}
	/** Returns this vector multiplied by scalar `r` */
	friend Vec4_ operator*(T r, const Vec4_& b) {return b*r;}
	/** Returns this vector divided by scalar `r` */
	Vec4_ operator/(T r) const {T t=1/r; return Vec4_(t*x, t*y, t*z, t*w);}
	/** Returns a vector that is a component-wise product of this vector and `b` */
	Vec4_ operator%(const Vec4_& b) const {return Vec4_(x*b.x, y*b.y, z*b.z, w*b.w);}
	/** Checks if this vector is equal to `b` */
	bool operator==(const Vec4_& b) const {return x==b.x && y==b.y && z==b.z && w==b.w;}
	/** Checks if this vector is not equal to `b` */
	bool operator!=(const Vec4_& b) const {return x!=b.x || y!=b.y || z!=b.z || w!=b.w;}
	/** Adds vector `b` to this vector */
	void operator+=(const Vec4_& b) {x += b.x; y += b.y; z += b.z; w += b.w;}
	/** Subtracts vector `b` from this vector */
	void operator-=(const Vec4_& b) {x -= b.x; y -= b.y; z -= b.z; w -= b.w;}
	/** Multiplies this vector by scalar `r` */
	void operator*=(T r) {x *= r; y *= r; z *= r; w *= r;}
	/** Divides this vector by scalar `r` */
	void operator/=(T r) {T t=1/r; x *= t; y *= t; z *= t; w *= t;}
	/** Multiplies this vector by another, component-wise */
	void operator%=(const Vec4_& b) { x *= b.x; y *= b.y; z *= b.z; w *= b.w; }
	/** Returns this vector negated */
	Vec4_ operator-() const {return Vec4_(-x,-y,-z, -w);}

	bool operator<(const Vec4_& b) const
	{
		return (x < b.x) ? true : ((x == b.x && y < b.y) ? true : (x == b.x && y == b.y && z < b.z) ? true
			: (x == b.x && y == b.y && z == b.z && w < b.w) ? true : false);
	}

public:
	/** The x, y, z, w components */
	T x, y, z, w;
};

template<class T>
inline int compare(const Vec4_<T>& a, const Vec4_<T>& b)
{
	if(a.x < b.x) return -1;
	else if(a.x == b.x && a.y < b.y) return -1;
	else if(a.x == b.x && a.y == b.y && a.z < b.z) return -1;
	else if(a.x == b.x && a.y == b.y && a.z == b.z) return 0;
	else if(a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w) return 0;
	else return 1;
}

typedef Vec4_<float> Vec4;
typedef Vec4_<double> Vec4d;

}

#endif
