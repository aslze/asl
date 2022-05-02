// Copyright(c) 1999-2022 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#include <asl/Matrix4.h>

#ifndef ASL_QUATERNION_H
#define ASL_QUATERNION_H

#include <asl/Vec3.h>

namespace asl {

/**
A Quaternion representing an orientation or rotation in 3D space
\ingroup Math3D
*/
template<class T>
class Quaternion_
{
public:
	T w, x, y, z;
public:
	Quaternion_() : w(1), x(0), y(0), z(0) {}
	/**
	Creates a quaternion from its 4 elements
	*/
	Quaternion_(T w, T x, T y, T z) : w(w), x(x), y(y), z(z) {}
	/**
	Creates a quaternion from a scalar and a vector
	*/
	Quaternion_(T w, const Vec3_<T>& v) : w(w), x(v.x), y(v.y), z(v.z) {}

	template<class T2>
	Quaternion_(const Quaternion_<T2>& q) : w(q.w), x(q.x), y(q.y), z(q.z) {}
	/**
	Creates a quaternion representing a rotation of angle `angle` around axis `axis`
	*/
	static Quaternion_ fromAxisAngle(const Vec3_<T>& axis, T angle)
	{
		T m = axis.length();
		return Quaternion_((T)cos((T)0.5 * angle), ((m != 0) ? (T)sin((T)0.5 * angle) / m : 0) * axis);
	}
	/**
	Creates a quaternion representing a rotation of angle `angle` around axis `axis` **of length one**
	*/
	static Quaternion_ fromAxisAngleU(const Vec3_<T>& axis, T angle)
	{
		return Quaternion_(cos((T)0.5 * angle), sin((T)0.5 * angle) * axis);
	}
	/**
	Creates a quaternion from a **rotation vector** (aligned with the rotation axis and
	with length equal to the rotation angle)
	*/
	static Quaternion_ fromAxisAngle(const Vec3_<T>& v)
	{
		return fromAxisAngle(v, v.length());
	}
	/**
	Returns the angle rotated by this quaternion
	*/
	T angle() const { return 2 * acos(w); }
	/*
	Returns the angle between this orientation and another
	*/
	T angle(const Quaternion_& q) const { return 2 * acos(*this * q) / (length() * q.length()); }
	/**
	Returns the normalized axis of rotation of this quaternion
	*/
	Vec3_<T> axis() const { return Vec3_<T>(x, y, z).normalized(); }
	/**
	Returns the axis-angle representation of the rotation equivalent to this quaternion (a vector aligned with the
	rotation axis and with length equal to the rotation angle)
	*/
	Vec3_<T> axisAngle() const { Vec3_<T> v(x, y, z); T k = v.length(); return (k == 0) ? Vec3_<T>(0, 0, 0) : v * (angle() / k); }
	/**
	Returns a transform matrix equivalent to the rotation of this quaternion
	*/
	Matrix4_<T> matrix() const {return Matrix4_<T>(
		1 - 2 * (y*y + z*z), 2 * (x*y - w*z), 2 * (x*z + w*y), 0,
		2 * (x*y + w*z), 1 - 2 * (x*x + z*z), 2 * (y*z - w*x), 0,
		2 * (x*z - w*y), 2 * (y*z + w*x), 1 - 2 * (x*x + y*y), 0
	);}
	/**
	Returns this quaternion negated
	*/
	Quaternion_ operator-() const {return Quaternion_(-w, -x, -y, -z);}
	/**
	Returns the conjugate of this quaternion
	*/
	Quaternion_ conj() const { return Quaternion_(w, -x, -y, -z); }
	/**
	Returns this quaternion multiplied by a scalar
	*/
	Quaternion_ operator*(T t) const { return Quaternion_(w*t, x*t, y*t, z*t); }
	/**
	Returns this quaternion divided by a scalar
	*/
	Quaternion_ operator/(T t) const { return *this * (1 / t); }
	/**
	Returns the length (norm) of this quaternion
	*/
	T length() const { return sqrt(x*x + y*y + z*z + w*w); }
	/**
	Returns the length (norm) of this quaternion squared
	*/
	T length2() const { return x*x + y*y + z*z + w*w; }
	/**
	Returns the inverse of this quaternion
	*/
	Quaternion_ inverse() const { return conj() / length2(); }
	/**
	Returns the given vector rotated by this quaterion
	*/
	Vec3_<T> operator*(const Vec3_<T>& v) const
	{
		return matrix() * v;
	}
	Quaternion_ operator+(const Quaternion_& q) const
	{
		return Quaternion_(w + q.w, x + q.x, y + q.y, z + q.z);
	}
	/**
	Returns the dot product of this quaternion and q
	*/
	T operator*(const Quaternion_& q) const
	{
		return  w * q.w + x * q.x + y * q.y + z * q.z;
	}
	/**
	Returns the product of this quaternion and q, representing the rotation q followed by this rotation
	*/
	Quaternion_ operator^(const Quaternion_& q) const
	{
		return Quaternion_(
			w*q.w - x*q.x - y*q.y - z*q.z,
			w*q.x + x*q.w + y*q.z - z*q.y,
			w*q.y - x*q.z + y*q.w + z*q.x,
			w*q.z + x*q.y - y*q.x + z*q.w
		);
	}
	/**
	Returns an interpolated quaternion between this and `b` at the given interpolation factor `t` in [0..1]
	*/
	Quaternion_ slerp(const Quaternion_& q, T t) const
	{
		Quaternion_ a = *this;
		if (a*q < 0.0)
			a = -a;
		T theta = (T)acos(a*q);
		if (theta == 0)
			return a;
		return a*(sin(theta - t * theta) / sin(theta)) + q*(sin(t * theta) / sin(theta));
	}
};

typedef Quaternion_<float> Quaternion;
typedef Quaternion_<double> Quaterniond;

}
#endif
