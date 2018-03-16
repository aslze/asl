// Copyright(c) 1999-2018 ASL author
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_MATRIX4_H
#define ASL_MATRIX4_H

#include <asl/Vec3.h>
#include <asl/Vec4.h>
#include <asl/Matrix3.h>

namespace asl {

class ASL_API Matrix4;
class ASL_API Quaternion;

/**
A Matrix4 is a 4x4 matrix useful for representing affine transformations in 3D space.

~~~
Matrix4 a = Matrix4::translate(10, 4, 0) * Matrix4::rotateX(PI/2);
Vec3 v = a.inverse().t() * Vec3(1, 0, 0);
~~~

*/
class ASL_API Matrix4
{
	float a[4][4]; // by rows
 public:
	Matrix4() {}
	/** Returns the element at row `i`, column `j`. */
	float& operator()(int i,int j) {return a[i][j];}
	float operator()(int i,int j) const {return a[i][j];}
	float& at(int i,int j) {return a[i][j];}
	float at(int i,int j) const {return a[i][j];}
	/**
	Constructs a matrix with the given elements (by rows) with last row [0 0 0 1] by default.
	*/
	Matrix4(float a11, float a12, float a13, float a14,
		float a21, float a22, float a23, float a24,
		float a31, float a32, float a33, float a34,
		float a41=0, float a42=0, float a43=0, float a44=1)
	{
		a[0][0]=a11; a[0][1]=a12; a[0][2]=a13; a[0][3]=a14;
		a[1][0]=a21; a[1][1]=a22; a[1][2]=a23; a[1][3]=a24;
		a[2][0]=a31; a[2][1]=a32; a[2][2]=a33; a[2][3]=a34;
		a[3][0]=a41; a[3][1]=a42; a[3][2]=a43; a[3][3]=a44;
	}
	Matrix4(const Vec3& v1, const Vec3& v2, const Vec3& v3, const Vec3& v4=Vec3(0,0,0)) // by columns
	{
		a[0][0]=v1.x; a[0][1]=v2.x; a[0][2]=v3.x; a[0][3]=v4.x;
		a[1][0]=v1.y; a[1][1]=v2.y; a[1][2]=v3.y; a[1][3]=v4.y;
		a[2][0]=v1.z; a[2][1]=v2.z; a[2][2]=v3.z; a[2][3]=v4.z;
		a[3][0]=0;    a[3][1]=0;    a[3][2]=0 ;   a[3][3]=1;
	}
	Matrix4(const float* m, bool gl=true)
	{
		a[0][0]=m[0]; a[0][1]=m[4]; a[0][2]=m[8]; a[0][3]=m[12];
		a[1][0]=m[1]; a[1][1]=m[5]; a[1][2]=m[9]; a[1][3]=m[13];
		a[2][0]=m[2]; a[2][1]=m[6]; a[2][2]=m[10]; a[2][3]=m[14];
		a[3][0]=m[3]; a[3][1]=m[7]; a[3][2]=m[11]; a[3][3]=m[15];
	}
	/** Returns this matrix transposed */
	Matrix4 t() const {
		return Matrix4(
		a[0][0], a[1][0], a[2][0], a[3][0],
		a[0][1], a[1][1], a[2][1], a[3][1],
		a[0][2], a[1][2], a[2][2], a[3][2],
		a[0][3], a[1][3], a[2][3], a[3][3]);
	}
	/** Returns this matrix plus `B` */
	Matrix4 operator+(const Matrix4& B) const;
	/** Returns this matrix multiplied by `B` */
	Matrix4 operator*(const Matrix4& B) const;
	/** Returns this matrix multiplied by `B`, considering only the 3x3 part not using the translation part. */
	Matrix4 operator%(const Matrix4& B) const;
	/** Returns this matrix multipled by scalar `t` */
	Matrix4 operator*(float t) const;
	friend Matrix4 operator*(float t, const Matrix4& m) {return m * t;}
	/** Multiplies this matrix by scalar `t` */
	Matrix4& operator*=(float t);
	/** Returns vector `p` left-multiplied by this matrix. */
	Vec4 operator*(const Vec4& p) const;
	/** Returns vector `p` left-multiplied by this matrix. */
	Vec3 operator*(const Vec3& p) const;
	/** Returns vector `p` left-multiplied by this matrix without applying the translation part. */
	Vec3 operator%(const Vec3& p) const;
	/** Returns a translation matrix for the given vector. */
	static Matrix4 translate(const Vec3&);
	/** Returns a translation matrix for the given coordinates. */
	static Matrix4 translate(float x, float y, float z) {return translate(Vec3(x,y,z));}
	/** Returns a scale matrix. */
	static Matrix4 scale(const Vec3&);
	/** Returns a scale matrix. */
	static Matrix4 scale(float s) {return scale(Vec3(s, s, s));}
	/** Returns a rotation matrix around the *x* axis. */
	static Matrix4 rotateX(float);
	/** Returns a rotation matrix around the *y* axis. */
	static Matrix4 rotateY(float);
	/** Returns a rotation matrix around the *z* axis. */
	static Matrix4 rotateZ(float);
	/** Returns a rotation matrix of an angle around a given axis. */
	static Matrix4 rotate(const Vec3& axis, float angle);
	static Matrix4 rotateAxis(const Vec3& axis, float angle) { return rotate(axis, angle); } // [deprecated]
	static Matrix4 reflection(Vec3 p, Vec3 n);
	/** Multipies this matrix by `B` */
	Matrix4& operator*=(const Matrix4& B);
	/** Returns the inverse of this matrix */
	Matrix4 inverse() const;
	/** Returns an identity matrix */
	static Matrix4 identity();
	/** Returns the *rotation* part of this matrix as a Quaternion */
	Quaternion rotation() const;
	/** Returns the *translation* part of this matrix (the last column) as a Vec3 */
	Vec3 translation() const {return Vec3(a[0][3], a[1][3], a[2][3]);}
	//Matrix4(const Matrix4& m) {memcpy(this,&m, sizeof(Matrix4));}
	//void operator=(const Matrix4& m) {memcpy(this,&m, sizeof(Matrix4));}
	/** Sets the *translation* part of this matrix (the last column) */
	Matrix4& setTranslation(const Vec3& t) { a[0][3] = t.x; a[1][3] = t.y; a[2][3] = t.z; return *this; }
	/** Returns the *determinant* part of this matrix. */
	float det();
};

inline Vec3 Matrix4::operator*(const Vec3& p) const
{
	return Vec3(
		a[0][0] * p.x + a[0][1] * p.y + a[0][2] * p.z + a[0][3],
		a[1][0] * p.x + a[1][1] * p.y + a[1][2] * p.z + a[1][3],
		a[2][0] * p.x + a[2][1] * p.y + a[2][2] * p.z + a[2][3]);
}

inline Vec4 Matrix4::operator*(const Vec4& p) const
{
	return Vec4(
		a[0][0] * p.x+a[0][1] * p.y + a[0][2] * p.z+a[0][3] * p.w,
		a[1][0] * p.x+a[1][1] * p.y + a[1][2] * p.z+a[1][3] * p.w,
		a[2][0] * p.x+a[2][1] * p.y + a[2][2] * p.z+a[2][3] * p.w,
		a[3][0] * p.x+a[3][1] * p.y + a[3][2] * p.z+a[3][3] * p.w );
}

typedef Matrix4 Matrix44;

}

#endif
