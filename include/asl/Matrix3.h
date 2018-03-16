// Copyright(c) 1999-2018 ASL author
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_MATRIX3_H
#define ASL_MATRIX3_H

#include <asl/Vec2.h>
#include <asl/Vec3.h>

namespace asl {

/**
A Matrix3 is a 3x3 matrix useful for representing affine transformations in 2D space.

~~~
Matrix3 translation = Matrix3::translate(10, 4);
Matrix3 rotation = Matrix3::rotate(PI/3) * Matrix3::scale(0.5);
Vec2 v = translation * rotation * Vec2(x, y);
~~~
*/

class ASL_API Matrix3
{
public:
	Matrix3() {}
	/**
	Constructs a 3x3 matrix given its elements (by rows).
	*/
	Matrix3(float a00, float a01, float a02,
		float a10, float a11, float a12,
		float a20 = 0, float a21 = 0, float a22 = 1)
	{
		a[0][0]=a00; a[0][1]=a01; a[0][2]=a02;
		a[1][0]=a10; a[1][1]=a11; a[1][2]=a12;
		a[2][0]=a20; a[2][1]=a21; a[2][2]=a22;
	}
	/** Returns the element at row `i`, column `j`. */
	float& operator()(int i, int j) {return a[i][j];}
	float operator()(int i, int j) const {return a[i][j];}

	/**
	Returns vector `v` left-multiplied by this matrix (only two components, to apply an affine transform).
	*/
	Vec2 operator*(const Vec2& v) const
	{
		return Vec2(a[0][0]*v.x + a[0][1]*v.y + a[0][2],
					a[1][0]*v.x + a[1][1]*v.y + a[1][2]);
	}
	/**
	Returns vector `v` left-multiplied by this matrix.
	*/
	Vec3 operator*(const Vec3& v) const
	{
		return Vec3(a[0][0]*v.x + a[0][1]*v.y + a[0][2]*v.z,
					a[1][0]*v.x + a[1][1]*v.y + a[1][2]*v.z,
					a[2][0]*v.x + a[2][1]*v.y + a[2][2]*v.z);
	}
	Vec2 operator%(const Vec2& v) const
	{
		return Vec2(a[0][0]*v.x + a[0][1]*v.y,
					a[1][0]*v.x + a[1][1]*v.y);
	}
	Matrix3& operator*=(float t)
	{
		for (int i = 0; i<3; i++)
			for (int j = 0; j<3; j++)
				a[i][j] *= t;
		return *this;
	}
	/**
	Returns this matrix multiplied by scalar t
	*/
	Matrix3 operator*(float t) const
	{
		Matrix3 C;
		for(int i=0; i<3; i++)
			for(int j=0; j<3; j++)
				C(i,j) = t * a[i][j];
		return C;
	}
	friend Matrix3 operator*(float t, const Matrix3& m)
	{
		return m * t;
	}
	/**
	Returns the sum of this matrix and B
	*/
	Matrix3 operator+(const Matrix3& B) const
	{
		Matrix3 C;
		for(int i=0; i<3; i++)
			for(int j=0; j<3; j++)
				C(i,j) = a[i][j] + B(i,j);
		return C;
	}
	/**
	Returns this matrix multiplied by matrix B
	*/
	Matrix3 operator*(const Matrix3& B) const
	{
		int i,j,k;
		Matrix3 C;
		const Matrix3& A=*this;
		float ab;

		for(i=0; i<3; i++)
		{
			for(j=0; j<3; j++)
			{
				for(ab=0.0f, k=0; k<2; k++)
					ab += A(i,k)*B(k,j);
				C(i,j)=ab;
	  		}
			C(i,2)+=A(i,2);
		}
		return C;
	}
	/**
	Returns the 3x3 identity matrix
	*/
	static Matrix3 identity()
	{
		return Matrix3(1, 0, 0,
		               0, 1, 0,
		               0, 0, 1);
	}
	/**
	Returns a 2D rotation matrix for angle t
	*/
	static Matrix3 rotate(float t)
	{
		float c=cos(t), s=sin(t);
		return Matrix3(c,-s, 0,
		               s, c, 0);
	}
	/**
	Returns a 2D translation matrix for vector t
	*/
	static Matrix3 translate(const Vec2& t)
	{
		return Matrix3(1, 0, t.x,
		               0, 1, t.y);
	}
	static Matrix3 translate(float x, float y)
	{
		return Matrix3(1, 0, x,
		               0, 1, y);
	}
	/**
	Returns a 2D scale matrix for factor t
	*/
	static Matrix3 scale(float t)
	{
		return Matrix3(t, 0, 0,
		               0, t, 0);
	}
	static Matrix3 scale(float x, float y)
	{
		return Matrix3(x, 0, 0,
		               0, y, 0);
	}
	/**
	Returns the inverse of this matrix
	*/
	Matrix3 inverse() const
	{
		float det = a[0][0] * a[1][1] * a[2][2] - a[0][0] * a[2][1] * a[1][2] - a[1][0] * a[0][1] * a[2][2] +
			a[1][0] * a[2][1] * a[0][2] + a[2][0] * a[0][1] * a[1][2] - a[2][0] * a[1][1] * a[0][2];

		Matrix3 m(
			-(-a[1][1] * a[2][2] + a[2][1] * a[1][2]), -a[0][1] * a[2][2] + a[2][1] * a[0][2], a[0][1] * a[1][2] - a[1][1] * a[0][2],
			-a[1][0] * a[2][2] + a[2][0] * a[1][2], -(-a[0][0] * a[2][2] + a[2][0] * a[0][2]), -(a[0][0] * a[1][2] - a[1][0] * a[0][2]),
			-(-a[1][0] * a[2][1] + a[2][0] * a[1][1]), -a[0][0] * a[2][1] + a[2][0] * a[0][1], a[0][0] * a[1][1] - a[1][0] * a[0][1]);
		m *= 1.0f / det;
		return m;
	}
private:
	float a[3][3];
};

typedef Matrix3 Matrix33;

}
#endif
