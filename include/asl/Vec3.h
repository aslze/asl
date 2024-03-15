// Copyright(c) 1999-2024 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_VECTOR3_H
#define ASL_VECTOR3_H

#include <asl/defs.h>
#include <asl/Vec2.h>
#include <math.h>

namespace asl {

/**
A Vec3_ represents a vector in 3D space. It is a template `Vec3_<T>` and has two predefined 
specializations `Vec3` (float) and `Vec3d` (double)

This class allows operating with vectors as with primitive types via operators. It can be used
together with class Matrix4 to transform vectors in space.

~~~
Vec3 a (10, 10, 0.1);
Vec3 b (-5, 1.5, 50);
Vec3 c = -a + 2.f * (b - a).normalized();
~~~

There are operators for dot product (`*`), cross product (`^`) and multiplication by a scalar (`*`).
~~~
float angle = acos((a * b) / (a.length() * b.length()));
Vec3 trinormal = ((b - a) ^ (c - a)).normalized();
~~~
\ingroup Math3D
*/
template<class T>
class Vec3_
{
 public:
	Vec3_() {}
	Vec3_(T x, T y, T z) : x(x), y(y), z(z) {}
	Vec3_(const Vec2_<T>& v, T z) : x(v.x), y(v.y), z(z) {}
	template<class T2>
	Vec3_(const Vec3_<T2>& v) : x(v.x), y(v.y), z(v.z) {}
	
	Vec3_(const T* v) : x(v[0]), y(v[1]), z(v[2]) {}
	static Vec3_ zeros() { return Vec3_(0, 0, 0); }
	operator const T*() const {return (T*)this;}
	/** Returns the *x* and *y* components as a Vec2 */
	Vec2_<T> xy() const {return Vec2_<T>(x, y);}
	/**
	Returns this vector with components reversed (z,y,x)
	*/
	Vec3_<T> zyx() const { return Vec3_<T>(z, y, x); }
	/** Returns the cartesian coordinates vector corresponding to this homogenous coordinates */
	Vec2_<T> h2c() const {T iz=1/z; return Vec2_<T>(iz*x, iz*y);}

	template<class T2>
	Vec3_<T2> with() const
	{
		return Vec3_<T2>(T2(x), T2(y), T2(z));
	}

	ASL_DEPRECATED(void set(T X, T Y, T Z), "Set components separately") {x=X; y=Y; z=Z;}

	/** Returns a normalized version of this vector */
	Vec3_ normalized() const {
		return *this/length();
	}
	/** Returns the length of the vector */
	T length() const {return sqrt(x*x+y*y+z*z);}
	/** Returns the length of the vector squared */
	T length2() const {return x*x+y*y+z*z;}
	/** Returns the length of the vector */
	T operator!() const {return length();}
	Vec3_ abs() const {return Vec3_(fabs(x), fabs(y), fabs(z));}
	/** Returns the angle between this vector and `b` */
	T angle(const Vec3_& b) const {return acos(clamp((*this) * b / (!(*this)*!b), T(-1), T(1)) );}

	/** Returns this plus `b` */
	Vec3_ operator+(const Vec3_& b) const {return Vec3_(x+b.x, y+b.y, z+b.z);}
	/** Returns this minus `b` */
	Vec3_ operator-(const Vec3_& b) const {return Vec3_(x-b.x, y-b.y, z-b.z);}
	/** Returns the *cross product* of this vector and `b` */
	Vec3_ operator^(const Vec3_& b) const {return Vec3_(y*b.z-z*b.y,z*b.x-x*b.z,x*b.y-y*b.x);}
	/** Returns the *dot product* of this vector and `b` */
	T operator*(const Vec3_& b) const {return x*b.x+y*b.y+z*b.z;}
	/** Returns this vector multiplied by scalar `r` */
	Vec3_ operator*(T r) const {return Vec3_(x*r, y*r, z*r);}
	/** Returns this vector multiplied by scalar `r` */
	friend Vec3_ operator*(T r, const Vec3_& b) {return b*r;}
	/** Returns this vector divided by scalar `r` */
	Vec3_ operator/(T r) const {T t=(T)1/r; return Vec3_(t*x, t*y, t*z);}
	/** Returns a vector that is a component-wise product of this vector and `b` */
	Vec3_ operator%(const Vec3_& b) const {return Vec3_(x*b.x, y*b.y, z*b.z);}
	/** Checks if this vector is equal to `b` */
	bool operator==(const Vec3_& b) const {return x==b.x && y==b.y && z==b.z;}
	/** Checks if this vector is not equal to `b` */
	bool operator!=(const Vec3_& b) const {return x!=b.x || y!=b.y || z!=b.z;}
	/** Adds vector `b` to this vector */
	void operator+=(const Vec3_& b) {x += b.x; y += b.y; z += b.z;}
	/** Subtracts vector `b` from this vector */
	void operator-=(const Vec3_& b) {x -= b.x; y -= b.y; z -= b.z;}
	/** Multiplies this vector by scalar `r` */
	void operator*=(T r) {x *= r; y *= r; z *= r;}
	/** Divides this vector by scalar `r` */
	void operator/=(T r) {T t=(T)1/r; x *= t; y *= t; z *= t;}
	/** Multiplies this vector by another, component-wise */
	void operator%=(const Vec3_& b) { x *= b.x; y *= b.y; z *= b.z; }
	/** Returns this vector negated */
	Vec3_ operator-() const {return Vec3_(-x,-y,-z);}

	/** Returns true if this vector's length is less than a given threshold (almost zero) */
	bool isNull( T tol=(T)0.000001) const {return length2() < tol;}

	/** Returns true if this vector is nearly parallel to vector `v2` with a given tolerance */
	bool isParallelToVector(const Vec3_& v2, T tol=(T)0.000001)
	{
		return fabs( ((*this) * v2)/(length() * v2.length()) ) > ((T)1-tol);
	}

public:
	/** The x, y, z components */
	T x, y, z;
};

template <class T>
inline int compare(const Vec3_<T>& a, const Vec3_<T>& b)
{
	if(a.x < b.x) return -1;
	else if(a.x == b.x && a.y < b.y) return -1;
	else if(a.x == b.x && a.y == b.y && a.z < b.z) return -1;
	else if(a.x == b.x && a.y == b.y && a.z == b.z) return 0;
	else return 1;
}

template <class T>
inline Vec3_<T> min(const Vec3_<T>& a, const Vec3_<T>& b)
{
	return Vec3_<T>(min(a.x, b.x), min(a.y, b.y), min(a.z, b.z));
}

template <class T>
inline Vec3_<T> max(const Vec3_<T>& a, const Vec3_<T>& b)
{
	return Vec3_<T>(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z));
}

template <class T>
inline const Vec3_<T> min(const Vec3_<T>& a, const Vec3_<T>& b, const Vec3_<T>& c)
{
	return min(min(a,b), c);
}

template <class T>
inline Vec3_<T> max(const Vec3_<T>& a, const Vec3_<T>& b, const Vec3_<T>& c)
{
	return max(max(a,b), c);
}

template <class T>
inline Vec3_<T> deg2rad(const Vec3_<T>& v) { return Vec3_<T>(deg2rad(v.x), deg2rad(v.y), deg2rad(v.z)); }

template <class T>
inline Vec3_<T> rad2deg(const Vec3_<T>& v) { return Vec3_<T>(rad2deg(v.x), rad2deg(v.y), rad2deg(v.z)); }

typedef Vec3_<float> Vec3;
typedef Vec3_<double> Vec3d;

}

#endif
