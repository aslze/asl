// Copyright(c) 1999-2018 ASL author
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_POSE_H
#define ASL_POSE_H
#include <asl/Matrix4.h>
#include <asl/Quaternion.h>

namespace asl {

/**
A Pose is a combination of a position and an orientation in 3D space.
*/

class Pose
{
	Vec3 p;
	Quaternion q;
public:
	Pose() {}
	/** Constructs a pose given an affine transformation matrix (it should be orthonormal) */
	Pose(const Matrix4& m): p(m.translation()), q(m.rotation()) {}
	/** Constructs a pose given a rotation and a position */
	Pose(const Vec3& pos, const Quaternion& rot): p(pos), q(rot) {}
	/** Constructs a pose given two pan/tilt angles and a position*/
	Pose(float pan, float tilt, const Vec3& pos, const Vec3& up = Vec3(0,1,0), const Vec3& right = Vec3(1,0,0)): p(pos)
	{
		q = Quaternion(up, pan) ^ Quaternion(right, tilt);
	}
	/** Returns the affine transformation matrix equivalent of this pose */
	Matrix4 matrix() const {return q.matrix().setTranslation(p);}
	/** Returns the translation part of this pose */
	const Vec3& position() const {return p;}
	/** Returns the orientation part of this pose */
	const Quaternion& orientation() const {return q;}
	/** Returns the interpolated pose between this and 'pose' with t as interpolation factor [0,1] */
	Pose interpolate(const Pose& pose, float t)
	{
		return Pose((1 - t)*p + t*pose.p, q.slerp(pose.q, t));
	}
};

}
#endif
