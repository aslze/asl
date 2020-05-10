// Copyright(c) 1999-2020 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_EIGENCAST_H
#define ASL_EIGENCAST_H

#include <asl/Vec3.h>
#include <asl/Matrix4.h>

namespace asl {

template<>
struct Castable<Vec3, Eigen::Vector3f> { typedef int is; };

template<>
struct Castable<Vec3d, Eigen::Vector3d> { typedef int is; };

template<>
struct Castable<Eigen::Vector3f, Vec3> { typedef int is; };

template<>
struct Castable<Eigen::Vector3d, Vec3d> { typedef int is; };

template<>
struct Castable<Eigen::Matrix4f, asl::Matrix4> { typedef int is; };

template<>
struct Castable<Eigen::Matrix4d, asl::Matrix4d> { typedef int is; };

template<>
struct Castable<asl::Matrix4, Eigen::Matrix4f> { typedef int is; };

template<>
struct Castable<asl::Matrix4d, Eigen::Matrix4d> { typedef int is; };

template<>
inline Vec3 to(const Eigen::Vector3f& v) { return Vec3(v[0], v[1], v[2]); }

template<>
inline Vec3d to(const Eigen::Vector3d& v) { return Vec3d(v[0], v[1], v[2]); }

template<>
inline Eigen::Vector3f to(const asl::Vec3& v) { return Eigen::Vector3f(v.x, v.y, v.z); }

template<>
inline Eigen::Vector3d to(const asl::Vec3d& v) { return Eigen::Vector3d(v.x, v.y, v.z); }

template<>
inline Matrix4 to(const Eigen::Matrix4f& m)
{
	return Matrix4(
		m(0, 0), m(0, 1), m(0, 2), m(0, 3),
		m(1, 0), m(1, 1), m(1, 2), m(1, 3),
		m(2, 0), m(2, 1), m(2, 2), m(2, 3),
		m(3, 0), m(3, 1), m(3, 2), m(3, 3)
	);
}

template<>
inline Matrix4d to(const Eigen::Matrix4d& m)
{
	return Matrix4d(
		m(0, 0), m(0, 1), m(0, 2), m(0, 3),
		m(1, 0), m(1, 1), m(1, 2), m(1, 3),
		m(2, 0), m(2, 1), m(2, 2), m(2, 3),
		m(3, 0), m(3, 1), m(3, 2), m(3, 3)
	);
}

template<>
inline Eigen::Matrix4f to(const Matrix4& m)
{
	Eigen::Matrix4f e;
	e << m(0, 0), m(0, 1), m(0, 2), m(0, 3),
	     m(1, 0), m(1, 1), m(1, 2), m(1, 3),
	     m(2, 0), m(2, 1), m(2, 2), m(2, 3),
	     m(3, 0), m(3, 1), m(3, 2), m(3, 3);
	return e;
}

template<>
inline Eigen::Matrix4d to(const Matrix4d& m)
{
	Eigen::Matrix4d e;
	e << m(0, 0), m(0, 1), m(0, 2), m(0, 3),
	     m(1, 0), m(1, 1), m(1, 2), m(1, 3),
	     m(2, 0), m(2, 1), m(2, 2), m(2, 3),
	     m(3, 0), m(3, 1), m(3, 2), m(3, 3);
	return e;
}

}

#endif
