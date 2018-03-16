// Copyright(c) 1999-2018 ASL author
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_QUATERNION_H
#define ASL_QUATERNION_H

#include <asl/Vec3.h>
#include <asl/Matrix4.h>

namespace asl {

/**
A Quaternion representing an orientation or rotation in 3D space
*/

class ASL_API Quaternion
{
public:
	float w, x, y, z;
public:
	Quaternion() {x = y = z = 0.0f; w = 1.0f;};
	/** Creates a quaternion representing a rotation of angle `t` around axis `v` */
	Quaternion(const Vec3& v, float t) { float k = sin(0.5f * t)/v.length(); w = cos(0.5f * t); x = k * v.x; y = k * v.y; z = k * v.z; };
	/** Creates a quaternion by its 4 elements */
	Quaternion(float _r, float _x, float _y, float _z) { w=_r; x=_x; y=_y; z=_z; }
	/** Returns the angle rotated by this quaternion */
	float angle() const { return 2 * acos(w); }
	/** Returns the normalized axis of rotation of this quaternion */
	Vec3 axis() const { return Vec3(x, y, z).normalized(); }
	/**
	Returns the axis-angle representation of the rotation equivalent to this quaternion (i.e. A vector aligned with the
	rotation axis and with length equal to the rotation angle) */
	Vec3 axisAngle() const { Vec3 v(x, y, z); float k = v.length(); return v * (angle()/k); }
	/** Returns a transform matrix equivalent to the rotation of this quaternion */
	Matrix4 matrix() const {return Matrix4(
		1 - 2 * (y*y + z*z), 2 * (x*y - w*z), 2 * (x*z + w*y), 0,
		2 * (x*y + w*z), 1 - 2 * (x*x + z*z), 2 * (y*z - w*x), 0,
		2 * (x*z - w*y), 2 * (y*z + w*x), 1 - 2 * (x*x + y*y), 0
	);}
	Quaternion operator-() const {return Quaternion(-w, -x, -y, -z);}
	Quaternion conj() const { return Quaternion(w, -x, -y, -z); }
	Quaternion operator*(float t) const { return Quaternion(w*t, x*t, y*t, z*t); }
	Quaternion operator/(float t) const { return *this * (1 / t); }
	float length() const { return sqrt(x*x + y*y + z*z + w*w); }
	float length2() const { return x*x + y*y + z*z + w*w; }

	Quaternion inverse() const { return conj() / length2(); }

	Vec3 operator*(const Vec3& v) const
	{
		return matrix() * v;
	}
	Quaternion operator+(const Quaternion& b) const
	{
		return Quaternion(w+b.w, x+b.x, y+b.y, z+b.z);
	}
	float operator*(const Quaternion& b) const
	{
		return  w*b.w + x*b.x + y*b.y + z*b.z;
	}
	Quaternion operator^(const Quaternion& b) const
	{
		Quaternion c;
		c.w = w*b.w - x*b.x - y*b.y - z*b.z;
		c.x = w*b.x + x*b.w + y*b.z - z*b.y;
		c.y = w*b.y - x*b.z + y*b.w + z*b.x;
		c.z = w*b.z + x*b.y - y*b.x + z*b.w;
		return c;
	}
	/** Returns an interpolated quaternion between this and `b` at the given interpolation factor `t` in [0..1] */
	Quaternion slerp(const Quaternion& b, float t) const
	{
		Quaternion a = *this;
		if (a*b < 0.0)
			a = -a;
		float theta = (float)acos(a*b);
		if (theta == 0.0f)
			return a;
		return a*(float)(sin(theta - t * theta) / sin(theta)) + b*(float)(sin(t * theta) / sin(theta));
	}
};

}
#endif
