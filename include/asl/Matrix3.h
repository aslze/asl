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
template<class T>
class ASL_API Matrix3_
{
public:
	Matrix3_() {}
	/**
	Constructs a 3x3 matrix given its elements (by rows).
	*/
	Matrix3_(T a00, T a01, T a02,
		T a10, T a11, T a12,
		T a20 = 0, T a21 = 0, T a22 = 1)
	{
		a[0][0]=a00; a[0][1]=a01; a[0][2]=a02;
		a[1][0]=a10; a[1][1]=a11; a[1][2]=a12;
		a[2][0]=a20; a[2][1]=a21; a[2][2]=a22;
	}
	/** Returns the element at row `i`, column `j`. */
	T& operator()(int i, int j) {return a[i][j];}
	T operator()(int i, int j) const {return a[i][j];}

	/**
	Returns vector `v` left-multiplied by this matrix (only two components, to apply an affine transform).
	*/
	Vec2_<T> operator*(const Vec2_<T>& v) const
	{
		return Vec2_<T>(a[0][0]*v.x + a[0][1]*v.y + a[0][2],
		                a[1][0]*v.x + a[1][1]*v.y + a[1][2]);
	}
	/**
	Returns vector `v` left-multiplied by this matrix.
	*/
	Vec3_<T> operator*(const Vec3_<T>& v) const
	{
		return Vec3_<T>(a[0][0]*v.x + a[0][1]*v.y + a[0][2]*v.z,
		                a[1][0]*v.x + a[1][1]*v.y + a[1][2]*v.z,
		                a[2][0]*v.x + a[2][1]*v.y + a[2][2]*v.z);
	}
	Vec2_<T> operator%(const Vec2_<T>& v) const
	{
		return Vec2_<T>(a[0][0]*v.x + a[0][1]*v.y,
		                a[1][0]*v.x + a[1][1]*v.y);
	}
	Matrix3_& operator*=(T t)
	{
		for (int i = 0; i<3; i++)
			for (int j = 0; j<3; j++)
				a[i][j] *= t;
		return *this;
	}
	/**
	Returns this matrix multiplied by scalar t
	*/
	Matrix3_ operator*(T t) const
	{
		Matrix3_ C;
		for(int i=0; i<3; i++)
			for(int j=0; j<3; j++)
				C(i,j) = t * a[i][j];
		return C;
	}
	friend Matrix3_ operator*(T t, const Matrix3_& m)
	{
		return m * t;
	}
	/**
	Returns the sum of this matrix and B
	*/
	Matrix3_ operator+(const Matrix3_& B) const
	{
		Matrix3_ C;
		for(int i=0; i<3; i++)
			for(int j=0; j<3; j++)
				C(i,j) = a[i][j] + B(i,j);
		return C;
	}
	/**
	Returns this matrix multiplied by matrix B
	*/
	Matrix3_ operator*(const Matrix3_& B) const
	{
		int i, j, k;
		Matrix3_ C;
		const Matrix3_& A = *this;
		T ab;

		for(i=0; i<3; i++)
		{
			for(j=0; j<3; j++)
			{
				for(ab=0, k=0; k<2; k++)
					ab += A(i,k)*B(k,j);
				C(i,j) = ab;
	  		}
			C(i,2) += A(i,2);
		}
		return C;
	}
	/**
	Returns the 3x3 identity matrix
	*/
	static Matrix3_ identity()
	{
		return Matrix3_(1, 0, 0,
		                0, 1, 0,
		                0, 0, 1);
	}
	/**
	Returns a 2D rotation matrix for angle t
	*/
	static Matrix3_ rotate(T t)
	{
		T c=cos(t), s=sin(t);
		return Matrix3_(c,-s, 0,
		                s, c, 0);
	}
	/**
	Returns a 2D translation matrix for vector t
	*/
	static Matrix3_ translate(const Vec2& t)
	{
		return Matrix3_(1, 0, t.x,
		                0, 1, t.y);
	}
	static Matrix3_ translate(T x, T y)
	{
		return Matrix3_(1, 0, x,
		                0, 1, y);
	}
	/**
	Returns a 2D scale matrix for factor t
	*/
	static Matrix3_ scale(T t)
	{
		return Matrix3_(t, 0, 0,
		                0, t, 0);
	}
	static Matrix3_ scale(T x, T y)
	{
		return Matrix3_(x, 0, 0,
		                0, y, 0);
	}
	/**
	Returns the inverse of this matrix
	*/
	Matrix3_ inverse() const
	{
		T det = a[0][0] * a[1][1] * a[2][2] - a[0][0] * a[2][1] * a[1][2] - a[1][0] * a[0][1] * a[2][2] +
			a[1][0] * a[2][1] * a[0][2] + a[2][0] * a[0][1] * a[1][2] - a[2][0] * a[1][1] * a[0][2];

		Matrix3_ m(
			-(-a[1][1] * a[2][2] + a[2][1] * a[1][2]), -a[0][1] * a[2][2] + a[2][1] * a[0][2], a[0][1] * a[1][2] - a[1][1] * a[0][2],
			-a[1][0] * a[2][2] + a[2][0] * a[1][2], -(-a[0][0] * a[2][2] + a[2][0] * a[0][2]), -(a[0][0] * a[1][2] - a[1][0] * a[0][2]),
			-(-a[1][0] * a[2][1] + a[2][0] * a[1][1]), -a[0][0] * a[2][1] + a[2][0] * a[0][1], a[0][0] * a[1][1] - a[1][0] * a[0][1]);
		m *= 1 / det;
		return m;
	}
private:
	T a[3][3];
};

typedef Matrix3_<float> Matrix3;
typedef Matrix3_<double> Matrix3d;

}
#endif
