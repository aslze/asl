// Copyright(c) 1999-2022 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_MATRIX_H
#define ASL_MATRIX_H

#include <asl/Array2.h>

namespace asl {

/**
 * A matrix supporting basic arithmetic operations.
 */
template <class T>
class Matrix : public Array2<T>
{
public:

	Matrix() {}

	/**
	Creates an array with size rows x cols
	*/
	Matrix(int rows, int cols) : Array2<T>(rows, cols) {}

	/**
	Creates an array with size rows x cols and initializes all items with value
	*/
	Matrix(int rows, int cols, const T& value) : Array2<T>(rows, cols, value) {}

	/**
	Creates an array of rows x cols elements and copies them from the pointer p (row-wise)
	*/
	Matrix(int rows, int cols, const T* p) : Array2<T>(rows, cols, p) {}

	Matrix(int rows, int cols, const Array<T>& a) : Array2<T>(rows, cols, a) {}

	Matrix(const Array<T>& a) : Array2<T>(a.length(), 1, a) {}

#ifdef ASL_HAVE_INITLIST
	/**
	Creates an array with size rows x cols and the given elements
	*/
	Matrix(int rows, int cols, std::initializer_list<T> a) : Array2<T>(rows, cols, a) {}

	/**
	Creates an array with given list of lists of elements
	*/
	Matrix(std::initializer_list<std::initializer_list<T> > a) : Array2<T>(a) {}

	Matrix(std::initializer_list<T> a) : Array2<T>((int)a.size(), 1, a) {}
#endif

	Matrix& clear()
	{
		this->resize(0, 0);
		return *this;
	}

	T& operator[](int i) { return this->_a[i]; }

	const T& operator[](int i) const { return this->_a[i]; }

	static Matrix identity(int n)
	{
		Matrix I(n, n);
		for (int i = 0; i < n; i++)
			for (int j = 0; j < n; j++)
				I(i, j) = (i == j) ? T(1) : T(0);
		return I;
	}

	void swapRows(int i1, int i2)
	{
		for (int j = 0; j < this->cols(); j++)
			swap((*this)(i1, j), (*this)(i2, j));
	}


	/**
	Creates a copy of this array with items converted to type K
	*/
	template<class K>
	Matrix<K> with() const
	{
		return Matrix<K>(this->_rows, this->_cols, this->_a.template with<K>());
	}

	/**
	Returns an independent copy of this array
	*/
	Matrix clone() const
	{
		Matrix b;
		b._a = this->_a.clone();
		b._rows = this->_rows;
		b._cols = this->_cols;
		return b;
	}

	/**
	Returns a sub-matrix consisting of a rows [i1, i2) and columns [j1, j2)
	*/
	Matrix slice(int i1, int i2, int j1, int j2) const
	{
		Matrix b(i2-i1, j2-j1);
		for (int i = 0; i < b.rows(); i++)
			for (int j = 0; j < b.cols(); j++)
				b(i, j) = (*this)(i1 + i, j1 + j);
		return b;
	}
	/**
	 * Returns the i-th row
	 */
	Matrix row(int i) const
	{
		return slice(i, i + 1, 0, this->cols());
	}

	/**
	 * Returns the j-th column
	 */
	Matrix col(int j) const
	{
		return slice(0, this->rows(), j, j + 1);
	}

	/**
	 * Returns this matrix transposed
	 */
	Matrix transposed() const
	{
		Matrix b(this->_cols, this->_rows);
		for (int i = 0; i < b.rows(); i++)
			for (int j = 0; j < b.cols(); j++)
				b(i, j) = (*this)(j, i);
		return b;
	}

	/**
	 * Computes the inverse of this matrix, if it exists; the matrix must be square
	 */
	Matrix inverse() const;

	/**
	 * Computes the pseudoinverse of this matrix.
	 */
	Matrix pseudoinverse() const
	{
		Matrix aT = transposed();
		return (aT * *this).inverse() * aT;
	}

	/**
	 * Computes the product of this matrix and b
	 */
	Matrix operator*(const Matrix& b) const
	{
		const Matrix& a = *this;
		Matrix c(a.rows(), b.cols());
		if (a.cols() != b.rows())
			return c.clear();
		for (int i = 0; i < c.rows(); i++)
			for (int j = 0; j < c.cols(); j++)
			{
				c(i, j) = 0;
				for (int k = 0; k < a.cols(); k++)
					c(i, j) += a(i, k) * b(k, j);
			}
		return c;
	}
	
	/**
	 * Computes the sum of this matrix and b
	 */
	Matrix operator+(const Matrix& b) const
	{
		const Matrix& a = *this;
		Matrix c(a.rows(), a.cols());
		if (a.rows() != b.rows() || a.cols() != b.cols())
			return c.clear();
		for (int i = 0; i < c.rows(); i++)
			for (int j = 0; j < c.cols(); j++)
				c(i, j) = a(i, j) + b(i, j);
		return c;
	}
	
	/**
	 * Computes the subtraction of this matrix and b
	 */
	Matrix operator-(const Matrix& b) const
	{
		const Matrix& a = *this;
		Matrix c(a.rows(), a.cols());
		if (a.rows() != b.rows() || a.cols() != b.cols())
			return c.clear();
		for (int i = 0; i < c.rows(); i++)
			for (int j = 0; j < c.cols(); j++)
				c(i, j) = a(i, j) - b(i, j);
		return c;
	}

	/**
	 * Computes the product of this matrix by scalar s
	 */
	Matrix operator*(T s) const
	{
		const Matrix& a = *this;
		Matrix c(a.rows(), a.cols());
		for (int i = 0; i < c.rows(); i++)
			for (int j = 0; j < c.cols(); j++)
				c(i, j) = a(i, j) * s;
		return c;
	}

	void operator+=(const Matrix& b)
	{
		Matrix& a = *this;
		if (a.rows() != b.rows() || a.cols() != b.cols())
			return;
		for (int i = 0; i < a.rows(); i++)
			for (int j = 0; j < a.cols(); j++)
				a(i, j) += b(i, j);
	}

	void operator-=(const Matrix& b)
	{
		Matrix& a = *this;
		if (a.rows() != b.rows() || a.cols() != b.cols())
			return;
		for (int i = 0; i < a.rows(); i++)
			for (int j = 0; j < a.cols(); j++)
				a(i, j) -= b(i, j);
	}

	void operator*=(T s)
	{
		Matrix& a = *this;
		for (int i = 0; i < a.rows(); i++)
			for (int j = 0; j < a.cols(); j++)
				a(i, j) *= s;
		return c;
	}

	Matrix operator-() const
	{
		const Matrix& a = *this;
		Matrix c(a.rows(), a.cols());
		for (int i = 0; i < c.rows(); i++)
			for (int j = 0; j < c.cols(); j++)
				c(i, j) = -a(i, j);
		return c;
	}

	T norm() const
	{
		T s = 0;
		for (int i = 0; i < c.rows(); i++)
			for (int j = 0; j < c.cols(); j++)
				s += sqr(*this(i, j));
		return sqrt(s);
	}

	friend Matrix operator*(T s, const Matrix& b) { return b * s; }
};

template<class T>
Matrix<T> solve_(Matrix<T>& A, Matrix<T>& b);

/**
 * Solves the matrix equation A*x=b and returns x; if b is a matrix (not a column) then the equation is solved
 * for each of b's columns and solutions returned as the columns of the returned matrix.
 */
template<class T>
Matrix<T> solve(const Matrix<T>& A, const Matrix<T>& b)
{
	Matrix<T> A2 = A.clone();
	Matrix<T> b2 = b.clone();
	return solve_(A2, b2);
}

template<class T>
Matrix<T> Matrix<T>::inverse() const
{
	return solve(*this, Matrix<T>::identity(this->rows()));
}

template<class T>
Matrix<T> solve_(Matrix<T>& A_, Matrix<T>& b_)
{
	Matrix<T> x(b_.rows(), b_.cols());
	int n = A_.rows();

	for (int j = 0; j < b_.cols(); j++)
	{
		Matrix<T> A = A_.clone();
		Matrix<T> b = b_.clone();
		for (int k = 0; k < n - 1; k++)
		{
			T max = 0;
			int ipivot = 0;
			for (int i = k; i < n; i++)
			{
				if (max < fabs(A(i, k))) {
					max = fabs(A(i, k));
					ipivot = i;
				}
			}

			A.swapRows(k, ipivot);
			swap(b(k, j), b(ipivot, j));

			for (int i = k + 1; i < n; i++)
			{
				T f = -A(i, k) / A(k, k);
				for (int jj = k; jj < n; jj++)
					A(i, jj) += A(k, jj) * f;
				b(i, j) += b(k, j) * f;
			}
		}

		for (int k = n - 1; k >= 0; k--)
		{
			T sum = 0;
			for (int i = k + 1; i < n; i++)
				sum += A(k, i) * x(i, j);
			x(k, j) = (b(k, j) - sum) / A(k, k);
		}
	}
	return x;
}

}

#endif
