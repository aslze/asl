// Copyright(c) 1999-2018 ASL author
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_QUATERNION_H
#define ASL_QUATERNION_H

#include <asl/Vec3.h>
#include <asl/Matrix4.h>

namespace asl {

/**
A Quaternion representing an orientation or rotation in 3D space
\ingroup Math3D
*/
template<class T>
class ASL_API Quaternion_
{
public:
	T w, x, y, z;
public:
	Quaternion_() {x = y = z = 0.0f; w = 1.0f;};
	/** Creates a quaternion representing a rotation of angle `t` around axis `v` */
	Quaternion_(const Vec3_<T>& v, T t) { T k = sin((T)0.5 * t)/v.length(); w = cos((T)0.5 * t); x = k * v.x; y = k * v.y; z = k * v.z; };
	/** Creates a quaternion by its 4 elements */
	Quaternion_(T _r, T _x, T _y, T _z) { w=_r; x=_x; y=_y; z=_z; }
	/** Returns the angle rotated by this quaternion */
	T angle() const { return 2 * acos(w); }
	/** Returns the normalized axis of rotation of this quaternion */
	Vec3_<T> axis() const { return Vec3_<T>(x, y, z).normalized(); }
	/**
	Returns the axis-angle representation of the rotation equivalent to this quaternion (i.e. A vector aligned with the
	rotation axis and with length equal to the rotation angle) */
	Vec3_<T> axisAngle() const { Vec3_<T> v(x, y, z); T k = v.length(); return v * (angle() / k); }
	/** Returns a transform matrix equivalent to the rotation of this quaternion */
	Matrix4_<T> matrix() const {return Matrix4_<T>(
		1 - 2 * (y*y + z*z), 2 * (x*y - w*z), 2 * (x*z + w*y), 0,
		2 * (x*y + w*z), 1 - 2 * (x*x + z*z), 2 * (y*z - w*x), 0,
		2 * (x*z - w*y), 2 * (y*z + w*x), 1 - 2 * (x*x + y*y), 0
	);}
	Quaternion_ operator-() const {return Quaternion_(-w, -x, -y, -z);}
	Quaternion_ conj() const { return Quaternion_(w, -x, -y, -z); }
	Quaternion_ operator*(T t) const { return Quaternion_(w*t, x*t, y*t, z*t); }
	Quaternion_ operator/(T t) const { return *this * (1 / t); }
	T length() const { return sqrt(x*x + y*y + z*z + w*w); }
	T length2() const { return x*x + y*y + z*z + w*w; }

	Quaternion_ inverse() const { return conj() / length2(); }

	Vec3_<T> operator*(const Vec3_<T>& v) const
	{
		return matrix() * v;
	}
	Quaternion_ operator+(const Quaternion_& b) const
	{
		return Quaternion_(w+b.w, x+b.x, y+b.y, z+b.z);
	}
	T operator*(const Quaternion_& b) const
	{
		return  w*b.w + x*b.x + y*b.y + z*b.z;
	}
	Quaternion_ operator^(const Quaternion_& b) const
	{
		Quaternion_ c;
		c.w = w*b.w - x*b.x - y*b.y - z*b.z;
		c.x = w*b.x + x*b.w + y*b.z - z*b.y;
		c.y = w*b.y - x*b.z + y*b.w + z*b.x;
		c.z = w*b.z + x*b.y - y*b.x + z*b.w;
		return c;
	}
	/** Returns an interpolated quaternion between this and `b` at the given interpolation factor `t` in [0..1] */
	Quaternion_ slerp(const Quaternion_& b, T t) const
	{
		Quaternion_ a = *this;
		if (a*b < 0.0)
			a = -a;
		T theta = (T)acos(a*b);
		if (theta == 0)
			return a;
		return a*(sin(theta - t * theta) / sin(theta)) + b*(sin(t * theta) / sin(theta));
	}
};

typedef Quaternion_<float> Quaternion;
typedef Quaternion_<double> Quaterniond;

}
#endif
