// Copyright(c) 1999-2018 ASL author
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_MATRIX4_H
#define ASL_MATRIX4_H

#include <asl/Vec3.h>
#include <asl/Vec4.h>
//#include <asl/Matrix3.h>

namespace asl {

template<class T> class ASL_API Matrix4_;
template<class T> class Quaternion_;

/**
A Matrix4 is a 4x4 matrix useful for representing affine transformations in 3D space.

~~~
Matrix4 a = Matrix4::translate(10, 4, 0) * Matrix4::rotateX(PI/2);
Vec3 v = a.inverse().t() * Vec3(1, 0, 0);
~~~
\ingroup Math3D
*/
template<class T>
class ASL_API Matrix4_
{
	T a[4][4]; // row-major
 public:
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
		a[0][0]=a11; a[0][1]=a12; a[0][2]=a13; a[0][3]=a14;
		a[1][0]=a21; a[1][1]=a22; a[1][2]=a23; a[1][3]=a24;
		a[2][0]=a31; a[2][1]=a32; a[2][2]=a33; a[2][3]=a34;
		a[3][0]=a41; a[3][1]=a42; a[3][2]=a43; a[3][3]=a44;
	}
	Matrix4_(const Vec3& v1, const Vec3& v2, const Vec3& v3, const Vec3& v4=Vec3(0,0,0)) // by columns
	{
		a[0][0]=v1.x; a[0][1]=v2.x; a[0][2]=v3.x; a[0][3]=v4.x;
		a[1][0]=v1.y; a[1][1]=v2.y; a[1][2]=v3.y; a[1][3]=v4.y;
		a[2][0]=v1.z; a[2][1]=v2.z; a[2][2]=v3.z; a[2][3]=v4.z;
		a[3][0]=0;    a[3][1]=0;    a[3][2]=0 ;   a[3][3]=1;
	}
	Matrix4_(const T* m, bool gl=true)
	{
		a[0][0]=m[0]; a[0][1]=m[4]; a[0][2]=m[8]; a[0][3]=m[12];
		a[1][0]=m[1]; a[1][1]=m[5]; a[1][2]=m[9]; a[1][3]=m[13];
		a[2][0]=m[2]; a[2][1]=m[6]; a[2][2]=m[10]; a[2][3]=m[14];
		a[3][0]=m[3]; a[3][1]=m[7]; a[3][2]=m[11]; a[3][3]=m[15];
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
	/**
	Returns this matrix plus `B`
	*/
	Matrix4_ operator+(const Matrix4_& B) const;
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
	static Matrix4_ scale(const Vec3_<T>&);
	/**
	Returns a scale matrix.
	*/
	static Matrix4_ scale(T s) {return scale(Vec3_<T>(s, s, s));}
	/**
	Returns a rotation matrix around the *x* axis.
	*/
	static Matrix4_ rotateX(T);
	/**
	Returns a rotation matrix around the *y* axis.
	*/
	static Matrix4_ rotateY(T);
	/**
	Returns a rotation matrix around the *z* axis.
	*/
	static Matrix4_ rotateZ(T);
	/**
	Returns a rotation matrix of an angle around a given axis.
	*/
	static Matrix4_ rotate(const Vec3_<T>& axis, T angle);
	/**
	Returns a rotation matrix from a rotation vector (aligned with the axis and with the rotation angle as magnitude)
	*/
	static Matrix4_ rotate(const Vec3_<T>& axisAngle) { return rotate(axisAngle, axisAngle.length()); }

	static Matrix4_ rotateAxis(const Vec3_<T>& axis, T angle) { return rotate(axis, angle); } // [deprecated]
	
	static Matrix4_ reflection(const Vec3_<T>& p, const Vec3_<T>& n);
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
};

typedef Matrix4_<float> Matrix4;
typedef Matrix4_<double> Matrix4d;

}

#endif
