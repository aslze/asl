// Copyright(c) 1999-2025 aslze
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
\ingroup Math3D
*/
template<class T>
class Matrix3_
{
public:
	bool isRowMajor() const { return true; }
	/**
	Returns the nubmer of rows of this matrix (3)
	*/
	int rows() const { return 3; }
	/**
	Returns the nubmer of columns of this matrix (3)
	*/
	int cols() const { return 3; }
	/**
	Returns the trace of this matrix
	*/
	T trace() const { return a[0][0] + a[1][1] + a[2][2]; }

	T* data() { return &a[0][0]; }
	const T* data() const { return &a[0][0]; }

	Matrix3_()
	{
		a[0][0]=1; a[0][1]=0; a[0][2]=0;
		a[1][0]=0; a[1][1]=1; a[1][2]=0;
		a[2][0]=0; a[2][1]=0; a[2][2]=1;
	}

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
	
	/**
	Constructs a matrix from elements pointed by m, row-major (default) or column-major
	*/
	ASL_EXPLICIT Matrix3_(const T* m, bool colmajor = false)
	{
		if (colmajor)
		{
			a[0][0] = m[0]; a[0][1] = m[3]; a[0][2] = m[6];
			a[1][0] = m[1]; a[1][1] = m[4]; a[1][2] = m[7];
			a[2][0] = m[2]; a[2][1] = m[5]; a[2][2] = m[8];
		}
		else
		{
			a[0][0] = m[0]; a[0][1] = m[1]; a[0][2] = m[2];
			a[1][0] = m[3]; a[1][1] = m[4]; a[1][2] = m[5];
			a[2][0] = m[6]; a[2][1] = m[7]; a[2][2] = m[8];
		}
	}

	template<class T2>
	Matrix3_(const Matrix3_<T2>& m)
	{
		a[0][0] = (T)m(0, 0); a[0][1] = (T)m(0, 1); a[0][2] = (T)m(0, 2);
		a[1][0] = (T)m(1, 0); a[1][1] = (T)m(1, 1); a[1][2] = (T)m(1, 2);
		a[2][0] = (T)m(2, 0); a[2][1] = (T)m(2, 1); a[2][2] = (T)m(2, 2);
	}

	/**
	Returns a copy of this matrix with elements converted to the given type
	*/
	template<class T2>
	Matrix3_<T2> with() const
	{
		return Matrix3_<T2>(*this);
	}

	/**
	Returns this matrix transposed
	*/
	Matrix3_ transposed() const
	{
		return Matrix3_(
			a[0][0], a[1][0], a[2][0],
			a[0][1], a[1][1], a[2][1],
			a[0][2], a[1][2], a[2][2]);
	}

	Matrix3_ t() const { return transposed(); }

	/** Returns the element at row `i`, column `j`. */
	T& operator()(int i, int j) { return a[i][j]; }
	const T& operator()(int i, int j) const { return a[i][j]; }

	/**
	Returns vector `v` left-multiplied by this matrix (only two components, to apply an affine transform).
	*/
	Vec2_<T> operator*(const Vec2_<T>& v) const
	{
		return Vec2_<T>(a[0][0]*v.x + a[0][1]*v.y + a[0][2],
		                a[1][0]*v.x + a[1][1]*v.y + a[1][2]);
	}
	/**
	Returns vector `v` transformed by this affine transform.
	*/
	Vec3_<T> operator*(const Vec3_<T>& v) const
	{
		return Vec3_<T>(a[0][0]*v.x + a[0][1]*v.y + a[0][2]*v.z,
		                a[1][0]*v.x + a[1][1]*v.y + a[1][2]*v.z,
		                a[2][0]*v.x + a[2][1]*v.y + a[2][2]*v.z);
	}

	/**
	Returns vector `v` transformed by this projective transform.
	*/
	Vec2_<T> operator^(const Vec2_<T>& v) const
	{
		T iw = 1 / (a[2][0] * v.x + a[2][1] * v.y + a[2][2]);
		return Vec2_<T>((a[0][0] * v.x + a[0][1] * v.y + a[0][2]) * iw,
		                (a[1][0] * v.x + a[1][1] * v.y + a[1][2]) * iw);
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
		Matrix3_ C;
		const Matrix3_& A = *this;

		for(int i=0; i<3; i++)
		{
			for(int j=0; j<3; j++)
			{
				T ab = 0;
				for(int k=0; k<3; k++)
					ab += A(i,k)*B(k,j);
				C(i,j) = ab;
	  		}
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
	static Matrix3_ translate(const Vec2_<T>& t)
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
	Returns a 2D scale matrix with factor s
	*/
	static Matrix3_ scale(T s)
	{
		return Matrix3_(s, 0, 0,
		                0, s, 0);
	}
	/**
	Returns a 2D scale matrix with factor sx, sy
	*/
	static Matrix3_ scale(T sx, T sy)
	{
		return Matrix3_(sx, 0, 0,
		                0, sy, 0);
	}

	/**
	Returns the i-th column's top 2 elements as a Vec2
	*/
	Vec2_<T> column2(int i) const { return Vec2_<T>(a[0][i], a[1][i]); }

	/**
	Returns the i-th column as a Vec3
	*/
	Vec3_<T> column(int i) const { return Vec3_<T>(a[0][i], a[1][i], a[2][i]); }

	/**
	Returns the *translation* part of this matrix (the last column) as a Vec2
	*/
	Vec2_<T> translation() const { return Vec2_<T>(a[0][2], a[1][2]); }

	/**
	Sets the *translation* part of this matrix (the last column)
	*/
	Matrix3_& setTranslation(const Vec2_<T>& t) { a[0][2] = t.x; a[1][2] = t.y; return *this; }

	/**
	Returns the *rotation* part of this matrix (assuming there is no skew)
	*/
	T rotation() const { return atan2(a[1][0], a[1][1]); }

	/**
	Returns the inverse of this matrix
	*/
	Matrix3_ inverse() const
	{
		T d = a[0][0] * (a[1][1] * a[2][2] - a[2][1] * a[1][2]) + a[1][0] * (a[2][1] * a[0][2] - a[0][1] * a[2][2]) + a[2][0] * (a[0][1] * a[1][2] - a[1][1] * a[0][2]);

		Matrix3_ m(
			a[1][1] * a[2][2] - a[2][1] * a[1][2], a[2][1] * a[0][2] - a[0][1] * a[2][2], a[0][1] * a[1][2] - a[1][1] * a[0][2],
			a[2][0] * a[1][2] - a[1][0] * a[2][2], a[0][0] * a[2][2] - a[2][0] * a[0][2], a[1][0] * a[0][2] - a[0][0] * a[1][2],
			a[1][0] * a[2][1] - a[2][0] * a[1][1], a[2][0] * a[0][1] - a[0][0] * a[2][1], a[0][0] * a[1][1] - a[1][0] * a[0][1]);
		m *= T(1) / d;
		return m;
	}
	/**
	Returns the *determinant* of this matrix.
	*/
	T det() const
	{
		return a[0][0] * (a[1][1] * a[2][2] - a[2][1] * a[1][2]) + a[1][0] * (a[2][1] * a[0][2] - a[0][1] * a[2][2]) + a[2][0] * (a[0][1] * a[1][2] - a[1][1] * a[0][2]);
	}

	/**
	Returns the matrix Frobenius norm squared
	*/
	T normSq() const
	{
		T t = 0;
		for (int i = 0; i < 3; i++)
			for (int j = 0; j < 3; j++)
				t += sqr(a[i][j]);
		return t;
	}

	/**
	Returns the matrix Frobenius norm
	*/
	T norm() const { return sqrt(normSq()); }

private:
	T a[3][3];
};

typedef Matrix3_<float> Matrix3;
typedef Matrix3_<double> Matrix3d;

}
#endif
