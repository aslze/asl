// Copyright(c) 1999-2022 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_MATRIX4_H
#define ASL_MATRIX4_H

#include <asl/Vec3.h>
#include <asl/Vec4.h>

namespace asl {

template<class T> class Matrix4_;
template<class T> class Quaternion_;
class String;

/**
A Matrix4 is a 4x4 matrix useful for representing affine transformations in 3D space.

~~~
Matrix4 a = Matrix4::translate(10, 4, 0) * Matrix4::rotateX(PI/2);
Vec3 v = a.inverse().t() * Vec3(1, 0, 0);

Matrix4 r = Matrix4::fromEuler(Vec3(alpha, beta, gamma), "XYZ*");
Vec3 angles = r.eulerAngles("ZYX");
~~~
\ingroup Math3D
*/
template<class T>
class Matrix4_
{
	T a[4][4]; // row-major
 
public:
	bool isRowMajor() const { return true; }
	/**
	Returns the nubmer of rows of this matrix (4)
	*/
	int rows() const { return 4; }
	/**
	Returns the nubmer of columns of this matrix (4)
	*/
	int cols() const { return 4; }

	/**
	Returns the trace of this matrix
	*/
	T trace() const { return a[0][0] + a[1][1] + a[2][2] + a[3][3]; }

	Matrix4_() {}
	/**
	Returns the element at row `i`, column `j`.
	*/
	T& operator()(int i,int j) {return a[i][j];}
	T operator()(int i,int j) const {return a[i][j];}
	T& at(int i,int j) {return a[i][j];}
	T at(int i,int j) const {return a[i][j];}
	template<class T2>
	Matrix4_(const Matrix4_<T2>& m)
	{
		a[0][0] = (T)m(0, 0); a[0][1] = (T)m(0, 1); a[0][2] = (T)m(0, 2); a[0][3] = (T)m(0, 3);
		a[1][0] = (T)m(1, 0); a[1][1] = (T)m(1, 1); a[1][2] = (T)m(1, 2); a[1][3] = (T)m(1, 3);
		a[2][0] = (T)m(2, 0); a[2][1] = (T)m(2, 1); a[2][2] = (T)m(2, 2); a[2][3] = (T)m(2, 3);
		a[3][0] = (T)m(3, 0); a[3][1] = (T)m(3, 1); a[3][2] = (T)m(3, 2); a[3][3] = (T)m(3, 3);
	}
	Matrix4_(const Matrix4_& m)
	{
		memcpy(this, &m, sizeof(m));
	}
	/**
	Constructs a matrix with the given elements (by rows) with last row [0 0 0 1] by default.
	*/
	Matrix4_(T a11, T a12, T a13, T a14,
		T a21, T a22, T a23, T a24,
		T a31, T a32, T a33, T a34,
		T a41=0, T a42=0, T a43=0, T a44=1)
	{
		a[0][0] = a11; a[0][1] = a12; a[0][2] = a13; a[0][3] = a14;
		a[1][0] = a21; a[1][1] = a22; a[1][2] = a23; a[1][3] = a24;
		a[2][0] = a31; a[2][1] = a32; a[2][2] = a33; a[2][3] = a34;
		a[3][0] = a41; a[3][1] = a42; a[3][2] = a43; a[3][3] = a44;
	}

	/**
	Constructs a matrix from 4 3D vectors used as columns, setting the last row as [0 0 0 1]
	*/
	Matrix4_(const Vec3& v1, const Vec3& v2, const Vec3& v3, const Vec3& v4=Vec3(0,0,0))
	{
		a[0][0] = v1.x; a[0][1] = v2.x; a[0][2] = v3.x; a[0][3] = v4.x;
		a[1][0] = v1.y; a[1][1] = v2.y; a[1][2] = v3.y; a[1][3] = v4.y;
		a[2][0] = v1.z; a[2][1] = v2.z; a[2][2] = v3.z; a[2][3] = v4.z;
		a[3][0] = 0;    a[3][1] = 0;    a[3][2] = 0;    a[3][3] = 1;
	}
	
	/**
	Constructs a matrix from 4 4D vectors used as columns
	*/
	Matrix4_(const Vec4& v1, const Vec4& v2, const Vec4& v3, const Vec4& v4)
	{
		a[0][0] = v1.x; a[0][1] = v2.x; a[0][2] = v3.x; a[0][3] = v4.x;
		a[1][0] = v1.y; a[1][1] = v2.y; a[1][2] = v3.y; a[1][3] = v4.y;
		a[2][0] = v1.z; a[2][1] = v2.z; a[2][2] = v3.z; a[2][3] = v4.z;
		a[3][0] = v1.w; a[3][1] = v2.w; a[3][2] = v3.w; a[3][3] = v4.w;
	}
	
	/**
	Constructs a matrix from elements pointed by m, row-major (default) or column-major
	*/
	Matrix4_(const T* m, bool colmajor = false)
	{
		if (colmajor)
		{
			a[0][0] = m[0]; a[0][1] = m[4]; a[0][2] = m[8]; a[0][3] = m[12];
			a[1][0] = m[1]; a[1][1] = m[5]; a[1][2] = m[9]; a[1][3] = m[13];
			a[2][0] = m[2]; a[2][1] = m[6]; a[2][2] = m[10]; a[2][3] = m[14];
			a[3][0] = m[3]; a[3][1] = m[7]; a[3][2] = m[11]; a[3][3] = m[15];
		}
		else
		{
			a[0][0] = m[0]; a[0][1] = m[1]; a[0][2] = m[2]; a[0][3] = m[3];
			a[1][0] = m[4]; a[1][1] = m[5]; a[1][2] = m[6]; a[1][3] = m[7];
			a[2][0] = m[8]; a[2][1] = m[9]; a[2][2] = m[10]; a[2][3] = m[11];
			a[3][0] = m[12]; a[3][1] = m[13]; a[3][2] = m[14]; a[3][3] = m[15];
		}
	}

	template<class T2>
	Matrix4_<T2> with() const
	{
		return Matrix4_<T2>(*this);
	}

	/**
	Returns this matrix transposed
	*/
	Matrix4_ t() const {
		return Matrix4_(
		a[0][0], a[1][0], a[2][0], a[3][0],
		a[0][1], a[1][1], a[2][1], a[3][1],
		a[0][2], a[1][2], a[2][2], a[3][2],
		a[0][3], a[1][3], a[2][3], a[3][3]);
	}

	bool isColmajor() const { return false; }
	/**
	Returns this matrix plus `B`
	*/
	Matrix4_ operator+(const Matrix4_& B) const;
	/**
	Returns this matrix minus `B`
	*/
	Matrix4_ operator-(const Matrix4_& B) const;
	/**
	Returns this matrix multiplied by `B`
	*/
	Matrix4_ operator*(const Matrix4_& B) const;
	/**
	Returns this matrix multipled by scalar `t`
	*/
	Matrix4_ operator*(T t) const;
	friend Matrix4_ operator*(T t, const Matrix4_& m) {return m * t;}
	/**
	Multiplies this matrix by scalar `t`
	*/
	Matrix4_& operator*=(T t);
	/**
	Returns vector `p` left-multiplied by this matrix.
	*/
	Vec4_<T> operator*(const Vec4_<T>& p) const
	{
		return Vec4_<T>(
			a[0][0] * p.x + a[0][1] * p.y + a[0][2] * p.z + a[0][3] * p.w,
			a[1][0] * p.x + a[1][1] * p.y + a[1][2] * p.z + a[1][3] * p.w,
			a[2][0] * p.x + a[2][1] * p.y + a[2][2] * p.z + a[2][3] * p.w,
			a[3][0] * p.x + a[3][1] * p.y + a[3][2] * p.z + a[3][3] * p.w);
	}
	/**
	Returns vector `p` left-multiplied by this matrix.
	*/
	Vec3_<T> operator*(const Vec3_<T>& p) const
	{
		return Vec3_<T>(
			a[0][0] * p.x + a[0][1] * p.y + a[0][2] * p.z + a[0][3],
			a[1][0] * p.x + a[1][1] * p.y + a[1][2] * p.z + a[1][3],
			a[2][0] * p.x + a[2][1] * p.y + a[2][2] * p.z + a[2][3]);
	}
	/**
	Returns vector `p` left-multiplied by this matrix without applying the translation part.
	*/
	Vec3_<T> operator%(const Vec3_<T>& p) const;
	/**
	Returns a translation matrix for the given vector.
	*/
	static Matrix4_ translate(const Vec3_<T>&);
	/**
	Returns a translation matrix for the given coordinates.
	*/
	static Matrix4_ translate(T x, T y, T z) {return translate(Vec3_<T>(x,y,z));}
	/**
	Returns a scale matrix.
	*/
	static Matrix4_ scale(const Vec3_<T>& s);
	/**
	Returns a uniform scale matrix.
	*/
	static Matrix4_ scale(T s) {return scale(Vec3_<T>(s, s, s));}
	/**
	Returns a rotation matrix of the given angle in radians around the *x* axis.
	*/
	static Matrix4_ rotateX(T angle);
	/**
	Returns a rotation matrix of the given angle in radians around the *y* axis.
	*/
	static Matrix4_ rotateY(T angle);
	/**
	Returns a rotation matrix of the given angle in radians around the *z* axis.
	*/
	static Matrix4_ rotateZ(T angle);
	/**
	Returns a rotation matrix of an angle in radians around a given axis.
	*/
	static Matrix4_ rotate(const Vec3_<T>& axis, T angle);

	// New, unstable API
	static Matrix4_ rotate(int axis, T angle);

	/**
	Returns a rotation matrix from a rotation vector (aligned with the axis and with the rotation angle as magnitude)
	*/
	static Matrix4_ rotate(const Vec3_<T>& axisAngle) { return rotate(axisAngle, axisAngle.length()); }

	/**
	Returns a rotation matrix created from Euler angles rotating the components of r in axes a0, a1, a2 (each one of 0, 1 or 2), the result
	is R[a0](r.x) * R[a1](r.y) * R[a2](r.z)
	*/
	static Matrix4_ fromEuler(const Vec3_<T>& r, int a0, int a1, int a2);

	/**
	Returns a rotation matrix created from Euler angles rotating the components of r in axes given as a string, such as "XYZ",
	if an '*' is appended then the result is equivalent to rotations on fixed axes, while by default it is equivalent to rotations
	on moving axes.
	*/
	static Matrix4_ fromEuler(const Vec3_<T>& r, const char* a);

	/**
	Computes the Euler angles corresponding to this rotation matrix (the top-left 3x3 submatrix) for the given axes rotation order
	given as [0, 1, 2] indices, equivalent to the fromEuler function
	*/
	Vec3_<T> eulerAngles(int a0, int a1, int a2) const;

	/**
	Computes the Euler angles corresponding to this rotation matrix (the top-left 3x3 submatrix) for the given axes rotation order
	given as a string, such as "XYZ" or "XYZ*", equivalent to the fromEuler function
	*/
	Vec3_<T> eulerAngles(const char* a) const;

	/**
	Multipies this matrix by `B`
	*/
	Matrix4_& operator*=(const Matrix4_& B);
	/**
	Returns the inverse of this matrix
	*/
	Matrix4_ inverse() const;
	/**
	Returns an identity matrix
	*/
	static Matrix4_ identity();
	/**
	Returns the *rotation* part of this matrix as a Quaternion
	*/
	Quaternion_<T> rotation() const;

	/**
	Returns the i-th column's top 3 elements as a Vec3
	*/
	Vec3_<T> column3(int i) const { return Vec3_<T>(a[0][i], a[1][i], a[2][i]); }

	/**
	Returns the i-th column as a Vec4
	*/
	Vec4_<T> column(int i) const { return Vec4_<T>(a[0][i], a[1][i], a[2][i], a[3][i]); }
	/**
	Returns the *translation* part of this matrix (the last column) as a Vec3
	*/
	Vec3_<T> translation() const {return Vec3_<T>(a[0][3], a[1][3], a[2][3]);}
	/**
	Sets the *translation* part of this matrix (the last column)
	*/
	Matrix4_& setTranslation(const Vec3_<T>& t) { a[0][3] = t.x; a[1][3] = t.y; a[2][3] = t.z; return *this; }
	/**
	Returns the *determinant* of this matrix.
	*/
	T det() const;

	T normSq() const;

	T norm() const { return sqrt(normSq()); }
};

typedef Matrix4_<float> Matrix4;
typedef Matrix4_<double> Matrix4d;

/**
Returns an orthonormal approximation of this transform matrix
\ingroup Math3D
*/
template <class T>
asl::Matrix4_<T> orthonormalize(const asl::Matrix4_<T>& m)
{
	Vec3_<T> v1 = m.column3(0).normalized();
	Vec3_<T> v2 = m.column3(1).normalized();
	Vec3_<T> v3 = m.column3(2).normalized();
	Vec3_<T> x = v2 ^ v3;
	Vec3_<T> y = v3 ^ v1;
	Vec3_<T> z = v1 ^ v2;
	v1 += x;
	v2 += y;
	v3 += z;
	x = (v2 ^ v3).normalized();
	y = (v3 ^ x).normalized();
	z = (x ^ y).normalized();
	return asl::Matrix4_<T>(x, y, z, m.column3(3));
}

}

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#include <asl/Quaternion.h>

namespace asl {

template<class T>
Matrix4_<T> Matrix4_<T>::operator+(const Matrix4_<T>& B) const
{
	Matrix4_<T> C;
	for(int i=0; i<4; i++)
		for(int j=0; j<4; j++)
			C(i,j) = a[i][j] + B(i,j);
	return C;
}

template<class T>
Matrix4_<T> Matrix4_<T>::operator-(const Matrix4_<T>& B) const
{
	Matrix4_<T> C;
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			C(i, j) = a[i][j] - B(i, j);
	return C;
}

template<class T>
inline Matrix4_<T> Matrix4_<T>::operator*(const Matrix4_<T>& B) const
{
	Matrix4_<T> C;
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			C(i, j) = a[i][0] * B(0, j) + a[i][1] * B(1, j) + a[i][2] * B(2, j) + a[i][3] * B(3, j);
	return C;
}

template<class T>
Matrix4_<T> Matrix4_<T>::operator*(T t) const
{
	Matrix4_<T> C;
	for(int i=0; i<4; i++)
		for(int j=0; j<4; j++)
			C(i,j) = t * a[i][j];
	return C;
}

template<class T>
Matrix4_<T>& Matrix4_<T>::operator*=(T t)
{
	for (int i = 0; i<4; i++)
		for (int j = 0; j<4; j++)
			a[i][j] *= t;
	return *this;
}

template<class T>
inline Matrix4_<T>& Matrix4_<T>::operator*=(const Matrix4_<T>& B)
{
	Matrix4_<T> C;
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			C(i, j) = a[i][0] * B(0, j) + a[i][1] * B(1, j) + a[i][2] * B(2, j) + a[i][3] * B(3, j);
	*this = C;
	return *this;
}

template<class T>
Matrix4_<T> Matrix4_<T>::identity()
{
	return Matrix4_<T>(
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1);
}

template<class T>
Matrix4_<T> Matrix4_<T>::translate(const Vec3_<T>& t)
{
	return Matrix4_<T>(
		1, 0, 0, t.x,
		0, 1, 0, t.y,
		0, 0, 1, t.z,
		0, 0, 0, 1);
}

template<class T>
Matrix4_<T> Matrix4_<T>::scale(const Vec3_<T>& s)
{
	return Matrix4_<T>(
		s.x, 0,   0,   0,
		0,   s.y, 0,   0,
		0,   0,   s.z, 0,
		0,   0,   0,   1);
}

template<class T>
Matrix4_<T> Matrix4_<T>::rotateX(T phi)
{
	return Matrix4_<T>(
		1, 0,        0,         0,
		0, cos(phi), -sin(phi), 0,
		0, sin(phi), cos(phi),  0,
		0, 0,        0,         1);
}

template<class T>
Matrix4_<T> Matrix4_<T>::rotateY(T phi)
{
	return Matrix4_<T>(
		cos(phi),  0, sin(phi), 0,
		0,         1, 0,        0,
		-sin(phi), 0, cos(phi), 0,
		0,         0, 0,        1);
}

template<class T>
Matrix4_<T> Matrix4_<T>::rotateZ(T phi)
{
	return Matrix4_<T>(
		cos(phi), -sin(phi), 0, 0,
		sin(phi), cos(phi),  0, 0,
		0,        0,         1, 0,
		0,        0,         0, 1);
}

template<class T>
inline Matrix4_<T> Matrix4_<T>::rotate(const Vec3_<T>& axis, T angle)
{
	return Quaternion_<T>::fromAxisAngle(axis, angle).matrix();
}

template<class T>
Matrix4_<T> Matrix4_<T>::rotate(int axis, T angle)
{
	switch (axis)
	{
	case 0: case 'X': return rotateX(angle);
	case 1: case 'Y': return rotateY(angle);
	case 2: case 'Z': return rotateZ(angle);
	}
	return identity();
}

template<class T>
Matrix4_<T> Matrix4_<T>::fromEuler(const Vec3_<T>& r, int a0, int a1, int a2)
{
	return rotate(a0, r.x) * rotate(a1, r.y) * rotate(a2, r.z);
}

template<class T>
Vec3_<T> Matrix4_<T>::eulerAngles(int a0, int a1, int a2) const
{
	T r0, r1, r2;

	if (a0 != a2)
	{
		T s = (a1 - a0 + 3) % 3 == 1 ? -1.0f : 1.0f;
		if (fabs(at(a0, a2)) < 1)
		{
			r1 = asin(-s * at(a0, a2));
			r2 = atan2(s * at(a1, a2), at(a2, a2));
			r0 = atan2(s * at(a0, a1), at(a0, a0));
		}
		else
		{
			r1 = -at(a0, a2) * s * ((T)PI / 2);
			r2 = at(a0, a2) * atan2(-s * at(a1, a0), at(a1, a1));
			r0 = 0;
		}
		return Vec3_<T>(r2, r1, r0);
	}
	else
	{
		int k = 3 - a0 - a1;
		T s = (a1 - a0 + 3) % 3 == 2 ? -1.0f : 1.0f;
		if (fabs(at(a0, a0)) < 1)
		{
			r1 = acos(at(a0, a0));
			r2 = atan2(at(a1, a0), -s * at(k, a0));
			r0 = atan2(at(a0, a1), s * at(a0, k));
		}
		else
		{
			r1 = at(a0, a0) < 0 ? (T)PI : 0;
			r2 = at(a0, a0) * atan2(-s * at(a1, k), at(a1, a1));
			r0 = 0;
		}
	}
	return Vec3_<T>(r2, r1, r0);
}

template<class T>
inline Matrix4_<T> Matrix4_<T>::fromEuler(const Vec3_<T>& r, const char* a)
{
	if (strlen(a) < 3)
		return Matrix4_<T>::identity();
	return (a[3] == '*') ? fromEuler(r.zyx(), a[2] - 'X', a[1] - 'X', a[0] - 'X') : fromEuler(r, a[0] - 'X', a[1] - 'X', a[2] - 'X');
}

template<class T>
inline Vec3_<T> Matrix4_<T>::eulerAngles(const char* a) const
{
	if (strlen(a) < 3)
		return Vec3_<T>(0, 0, 0);
	return (a[3] == '*') ? eulerAngles(a[2] - 'X', a[1] - 'X', a[0] - 'X').zyx() : eulerAngles(a[0] - 'X', a[1] - 'X', a[2] - 'X');
}


template<class T>
inline Vec3_<T> Matrix4_<T>::operator%(const Vec3_<T>& p) const
{
	return Vec3_<T>(
		a[0][0] * p.x + a[0][1] * p.y + a[0][2] * p.z,
		a[1][0] * p.x + a[1][1] * p.y + a[1][2] * p.z,
		a[2][0] * p.x + a[2][1] * p.y + a[2][2] * p.z);
}

template<class T>
Matrix4_<T> Matrix4_<T>::inverse() const
{
	T d = a[0][0] * (a[1][1] * a[2][2] - a[1][2] * a[2][1]) + a[1][0] * (a[0][2] * a[2][1] - a[0][1] * a[2][2]) + a[2][0] * (a[0][1] * a[1][2] - a[0][2] * a[1][1]);
	T x = a[0][1] * (a[1][3] * a[2][2] - a[1][2] * a[2][3]) + a[1][1] * (a[0][2] * a[2][3] - a[0][3] * a[2][2]) + a[2][1] * (a[0][3] * a[1][2] - a[0][2] * a[1][3]);
	T y = a[0][0] * (a[1][2] * a[2][3] - a[1][3] * a[2][2]) + a[1][0] * (a[0][3] * a[2][2] - a[0][2] * a[2][3]) + a[2][0] * (a[0][2] * a[1][3] - a[0][3] * a[1][2]);
	T z = a[0][0] * (a[1][3] * a[2][1] - a[1][1] * a[2][3]) + a[1][0] * (a[0][1] * a[2][3] - a[0][3] * a[2][1]) + a[2][0] * (a[0][3] * a[1][1] - a[0][1] * a[1][3]);

	Matrix4_<T> m(
		a[1][1] * a[2][2] - a[1][2] * a[2][1], a[0][2] * a[2][1] - a[0][1] * a[2][2], a[0][1] * a[1][2] - a[0][2] * a[1][1], x,
		a[1][2] * a[2][0] - a[1][0] * a[2][2], a[0][0] * a[2][2] - a[0][2] * a[2][0], a[0][2] * a[1][0] - a[0][0] * a[1][2], y,
		a[1][0] * a[2][1] - a[1][1] * a[2][0], a[0][1] * a[2][0] - a[0][0] * a[2][1], a[0][0] * a[1][1] - a[0][1] * a[1][0], z);
	m *= T(1)/d;
	m(3, 3) = 1;
	return m;
}

template<class T>
Quaternion_<T> Matrix4_<T>::rotation() const
{
    T t = at(0,0) + at(1,1) + at(2,2), s, r;
    if(t >= 0)
	{
		r = sqrt(1 + t);
		s = (T)0.5 / r;
		return Quaternion_<T>((T)0.5 * r, (at(2, 1) - at(1, 2)) * s, (at(0, 2) - at(2, 0)) * s, (at(1, 0) - at(0, 1)) * s);
	}
    else if (at(1, 1) > at(0, 0) && at(1, 1) >= at(2, 2))
	{
		r = sqrt(1 + at(1, 1) - at(2, 2) - at(0, 0));
		s = (T)0.5 / r;
		return Quaternion_<T>((at(0, 2) - at(2, 0)) * s, (at(0, 1) + at(1, 0)) * s, (T)0.5 * r, (at(1, 2) + at(2, 1)) * s);
	}
	else if (at(2, 2) > at(0, 0) /*&& at(2, 2) > at(1, 1)*/)
	{
		r = sqrt(1 + at(2, 2) - at(0, 0) - at(1, 1));
		s = (T)0.5 / r;
		return Quaternion_<T>((at(1, 0) - at(0, 1)) * s, (at(2, 0) + at(0, 2)) * s, (at(1, 2) + at(2, 1)) * s, (T)0.5 * r);
	}
	else
	{
		r = sqrt(1 + at(0, 0) - at(1, 1) - at(2, 2));
		s = (T)0.5 / r;
		return Quaternion_<T>((at(2, 1) - at(1, 2)) * s, (T)0.5 * r, (at(0, 1) + at(1, 0)) * s, (at(2, 0) + at(0, 2)) * s);
	}
}

template<class T>
T Matrix4_<T>::det() const 
{
	return a[0][0]*a[1][1]*a[2][2]-a[0][0]*a[2][1]*a[1][2]-a[1][0]*a[0][1]*a[2][2]+
		a[1][0]*a[2][1]*a[0][2]+a[2][0]*a[0][1]*a[1][2]-a[2][0]*a[1][1]*a[0][2];
}

template<class T>
T Matrix4_<T>::normSq() const
{
	T t = 0;
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			t += sqr(a[i][j]);
	return t;
}

template class Matrix4_<float>;
template class Matrix4_<double>;
}

#endif

#endif
