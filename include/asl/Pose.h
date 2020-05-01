// Copyright(c) 1999-2020 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_POSE_H
#define ASL_POSE_H
#include <asl/Matrix4.h>
#include <asl/Quaternion.h>

namespace asl {

/**
A Pose is a combination of a position and an orientation in 3D space.
*/
template<class T>
class Pose_
{
	Vec3_<T> p;
	Quaternion_<T> q;
public:
	Pose_() {}
	/**
	Constructs a pose given an affine transformation matrix (it should be orthonormal)
	*/
	Pose_(const Matrix4_<T>& m): p(m.translation()), q(m.rotation()) {}
	/**
	Constructs a pose given a rotation and a position
	*/
	Pose_(const Vec3_<T>& pos, const Quaternion_<T>& rot): p(pos), q(rot) {}
	/**
	Constructs a pose given two pan/tilt angles and a position
	*/
	Pose_(T pan, T tilt, const Vec3_<T>& pos, const Vec3_<T>& up = Vec3_<T>(0, 1, 0), const Vec3_<T>& right = Vec3_<T>(1, 0, 0)) : p(pos)
	{
		q = Quaternion_<T>::fromAxisAngleU(up, pan) ^ Quaternion_<T>::fromAxisAngleU(right, tilt);
	}
	/**
	Returns the affine transformation matrix equivalent of this pose
	*/
	Matrix4_<T> matrix() const { return q.matrix().setTranslation(p); }
	/**
	Returns the translation part of this pose
	*/
	const Vec3_<T>& position() const { return p; }
	/**
	Returns the orientation part of this pose
	*/
	const Quaternion_<T>& orientation() const { return q; }
	/**
	Returns the interpolated pose between this and 'pose' with t as interpolation factor [0,1]
	*/
	Pose_ interpolate(const Pose_& pose, T t)
	{
		return Pose((1 - t)*p + t*pose.p, q.slerp(pose.q, t));
	}
};

typedef Pose_<float> Pose;
typedef Pose_<double> Posed;

}
#endif
