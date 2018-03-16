// Copyright(c) 1999-2018 ASL author
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_VECTOR3_H
#define ASL_VECTOR3_H

#include <asl/defs.h>
#include <asl/Vec2.h>
#include <math.h>

namespace asl {

/**
Vec3 represents a vector in 3D space.

This class allows operating with vectors as with primitive types via operators. It can be used
together with class Matrix4 to transform vectors in space.

~~~
Vec3 a (10, 10, 0.1);
Vec3 b (-5, 1.5, 50);
Vec3 c = -a + 2.0 * (b - a).normalized();
~~~

The angle between two vectors can be computed with:
~~~
float angle = acos(a/!a * b/!b);
~~~
*/

class Vec3
{
 public:
	Vec3(float X, float Y, float Z): x(X), y(Y), z(Z) {}
	Vec3(const Vec3& v): x(v.x), y(v.y), z(v.z) {}
	Vec3(const float* v): x(v[0]), y(v[1]), z(v[2]) {}
	Vec3() {}
	operator const float*() const {return (float*)this;}
	/** Returns the *x* and *y* components as a Vec2 */
	Vec2 xy() const {return Vec2(x, y);}
	/** Returns the cartesian coordinates vector corresponding to this homogenous coordinates */
	Vec2 h2c() const {float iz=1/z; return Vec2(iz*x, iz*y);}
	static Vec3 fromCylindrical( float r, float a, float z) {return Vec3(r*cos(a), r*sin(a), z);}
	static Vec3 fromSpherical( float r, float a, float b)
	{float R=r*cos(b); return Vec3(R*cos(a), R*sin(b), r*sin(b));}

	void set( float X,float Y,float Z) {x=X; y=Y; z=Z;}
	void get( float& X,float& Y,float& Z) const {X=x; Y=y; Z=z;}

	/** Returns a normalized version of this vector */
	Vec3 normalized() const {
		return *this/length();
	}
	/** Returns the length of the vector */
	float length() const {return sqrt(x*x+y*y+z*z);}
	/** Returns the length of the vector squared */
	float length2() const {return x*x+y*y+z*z;}
	/** Returns the length of the vector */
	float operator!() const {return length();}
	Vec3 abs() const {return Vec3(fabs(x), fabs(y), fabs(z));}
	/** Returns the angle between this vector and `b` */
	float angle(const Vec3& b) const {return acosf(clamp((*this) * b / (!(*this)*!b), -1.0f, 1.0f) );}

	void operator=(const Vec3& b) {x=b.x; y=b.y; z=b.z;}
	/** Returns this plus `b` */
	Vec3 operator+(const Vec3& b) const {return Vec3(x+b.x, y+b.y, z+b.z);}
	/** Returns this minus `b` */
	Vec3 operator-(const Vec3& b) const {return Vec3(x-b.x, y-b.y, z-b.z);}
	/** Returns the *cross product* of this vector and `b` */
	Vec3 operator^(const Vec3& b) const {return Vec3(y*b.z-z*b.y,z*b.x-x*b.z,x*b.y-y*b.x);}
	/** Returns the *dot product* of this vector and `b` */
	float operator*(const Vec3& b) const {return x*b.x+y*b.y+z*b.z;}
	/** Returns this vector multiplied by scalar `r` */
	Vec3 operator*(float r) const {return Vec3(x*r, y*r, z*r);}
	/** Returns this vector multiplied by scalar `r` */
	friend Vec3 operator*(float r, const Vec3& b) {return b*r;}
	Vec3 operator/(float r) const {float t=1.0f/r; return Vec3(t*x, t*y, t*z);}
	/** Returns a vector that is a component-wise product of this vector and `b` */
	Vec3 operator%(const Vec3& b) const {return Vec3(x*b.x, y*b.y, z*b.z);}
	/** Checks if this vector is equal to `b` */
	bool operator==(const Vec3& b) const {return x==b.x && y==b.y && z==b.z;}
	/** Checks if this vector is not equal to `b` */
	bool operator!=(const Vec3& b) const {return x!=b.x || y!=b.y || z!=b.z;}
	/** Adds vector `b` to this vector */
	void operator+=(const Vec3& b) {x += b.x; y += b.y; z += b.z;}
	/** Subtracts vector `b` from this vector */
	void operator-=(const Vec3& b) {x -= b.x; y -= b.y; z -= b.z;}
	/** Multiplies this vector by scalar `r` */
	void operator*=(float r) {x *= r; y *= r; z *= r;}
	/** Divides this vector by scalar `r` */
	void operator/=(float r) {float t=1.0f/r; x *= t; y *= t; z *= t;}
	/** Returns this vector negated */
	Vec3 operator-() const {return Vec3(-x,-y,-z);}

	/** Returns true if this vector's length is less than a given threshold (almost zero) */
	bool isNull( float tol=0.000001f) const {return length2() < tol;}

	/** Returns true if this vector is nearly parallel to vector `v2` with a given tolerance */
	bool isParallelToVector(const Vec3& v2, float tol=0.000001f)
	{
		return fabsf( ((*this) * v2)/(length() * v2.length()) ) > (1.0f-tol);
	}

public:
	/** The x, y, z components */
	float x, y, z;
};

inline int compare(const Vec3& a, const Vec3& b)
{
	if(a.x < b.x) return -1;
	else if(a.x == b.x && a.y < b.y) return -1;
	else if(a.x == b.x && a.y == b.y && a.z < b.z) return -1;
	else if(a.x == b.x && a.y == b.y && a.z == b.z) return 0;
	else return 1;
}

inline Vec3 min(const Vec3& a, const Vec3& b)
{
	return Vec3(min(a.x, b.x), min(a.y, b.y), min(a.z, b.z) );
}

inline Vec3 max(const Vec3& a, const Vec3& b)
{
	return Vec3(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z) );
}

inline const Vec3 min(const Vec3& a, const Vec3& b, const Vec3& c)
{
	return min(min(a,b), c);
}

inline Vec3 max(const Vec3& a, const Vec3& b, const Vec3& c)
{
	return max(max(a,b), c);
}

}

#endif
