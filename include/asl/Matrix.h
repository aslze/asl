// Copyright(c) 1999-2023 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_MATRIX_H
#define ASL_MATRIX_H

#include <asl/Array2.h>

namespace asl {

/**
 * A matrix supporting basic arithmetic operations. With two predefined specializations: `Matrix` for doubles and `Matrixf` for floats.
 * 
 * ~~~
 * Matrix A = {   // this makes a 2x2 matrix
 *    { 1, -1 },
 *    { 2, 3 }
 * };
 * 
 * Matrix b = { 5, 1 }; // this makes a column matrix (a vector)
 * ~~~
 *
 * You can operate with matrices:
 *
 * ~~~
 * auto C = A.inverse() * A + b * b.transposed();
 * ~~~
 *
 * And you can solve linear systems, even using least-squares, if more equations than unknowns:
 * 
 * ~~~
 * auto x = solve(A, b); // solution to linear system A * x = b
 * ~~~
 * 
 * \ingroup Math3D
 */
template <class T>
class Matrix_ : public Array2<T>
{
public:

	Matrix_() {}

	/**
	Creates a matrix with size rows x cols
	*/
	ASL_EXPLICIT Matrix_(int rows, int cols = 1) : Array2<T>(rows, cols) {}

	/**
	Creates a matrix with size rows x cols and initializes all items with value
	*/
	ASL_EXPLICIT Matrix_(int rows, int cols, const T& value) : Array2<T>(rows, cols, value) {}

	/**
	Creates a matrix of rows x cols elements and copies them from the pointer p (row-wise)
	*/
	ASL_EXPLICIT Matrix_(int rows, int cols, const T* p) : Array2<T>(rows, cols, p) {}

	ASL_EXPLICIT Matrix_(int rows, int cols, const Array<T>& a) : Array2<T>(rows, cols, a) {}

	Matrix_(const Array<T>& a) : Array2<T>(a.length(), 1, a) {}

#ifdef ASL_HAVE_INITLIST
	/**
	Creates a matrix with size rows x cols and the given elements
	*/
	ASL_EXPLICIT Matrix_(int rows, int cols, std::initializer_list<T> a) : Array2<T>(rows, cols, a) {}

	/**
	Creates a matrix with given list of lists of elements
	*/
	Matrix_(std::initializer_list<std::initializer_list<T> > a) : Array2<T>(a) {}

	/**
	Creates a column matrix with given list of elements
	*/
	Matrix_(std::initializer_list<T> a) : Array2<T>((int)a.size(), 1, a) {}
#endif

	/**
	 * Returns the trace of this matrix
	*/
	T trace() const { if (this->_rows != this->_cols) return 0; T t = 0; for (int i = 0; i < this->rows(); i++) t += (*this)(i, i); return t; }

	void copy(const Matrix_& b)
	{
		this->resize(b.rows(), b.cols());
		for (int i = 0; i < length(); i++)
			this->_a[i] = b._a[i];
	}

	Matrix_& clear()
	{
		this->resize(0, 0);
		return *this;
	}

	T& operator[](int i) { return this->_a[i]; }

	const T& operator[](int i) const { return this->_a[i]; }

	int length() const { return this->_a.length(); }

	/**
	Returns an identity matrix of the given size
	*/
	static Matrix_ identity(int n)
	{
		Matrix_ I(n, n);
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
	Returns a copy of this matrix with elements converted to the given type
	*/
	template<class K>
	Matrix_<K> with() const
	{
		return Matrix_<K>(this->_rows, this->_cols, this->_a.template with<K>());
	}

	/**
	Returns an independent copy of this matrix
	*/
	Matrix_ clone() const
	{
		Matrix_ b;
		b._a = this->_a.clone();
		b._rows = this->_rows;
		b._cols = this->_cols;
		return b;
	}

	/**
	Returns a sub-matrix consisting of a rows [i1, i2) and columns [j1, j2)
	*/
	Matrix_ slice(int i1, int i2, int j1, int j2) const
	{
		Matrix_ b(i2-i1, j2-j1);
		for (int i = 0; i < b.rows(); i++)
			for (int j = 0; j < b.cols(); j++)
				b(i, j) = (*this)(i1 + i, j1 + j);
		return b;
	}
	/**
	 * Returns the i-th row
	 */
	Matrix_ row(int i) const
	{
		return slice(i, i + 1, 0, this->cols());
	}

	/**
	 * Returns the j-th column
	 */
	Matrix_ col(int j) const
	{
		return slice(0, this->rows(), j, j + 1);
	}

	/**
	 * Returns this matrix transposed
	 */
	Matrix_ transposed() const
	{
		Matrix_ b(this->_cols, this->_rows);
		for (int i = 0; i < b.rows(); i++)
			for (int j = 0; j < b.cols(); j++)
				b(i, j) = (*this)(j, i);
		return b;
	}

	/**
	 * Computes the inverse of this matrix, if it exists; the matrix must be square
	 */
	Matrix_ inverse() const;

	/**
	 * Computes the pseudoinverse of this matrix.
	 */
	Matrix_ pseudoinverse() const
	{
		Matrix_ aT = transposed();
		return (aT * *this).inverse() * aT;
	}

	/**
	 * Computes the product of this matrix and b
	 */
	Matrix_ operator*(const Matrix_& b) const
	{
		const Matrix_& a = *this;
		Matrix_ c(a.rows(), b.cols());
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
	 * Computes the product of this matrix *transposed* and b (same as `A.transposed() * B` but a bit faster)
	 */
	Matrix_ transposed(const Matrix_& b) const
	{
		const Matrix_& a = *this;
		Matrix_        c(a.cols(), b.cols());
		if (a.rows() != b.rows())
			return c.clear();
		for (int i = 0; i < c.rows(); i++)
			for (int j = 0; j < c.cols(); j++)
			{
				c(i, j) = 0;
				for (int k = 0; k < a.rows(); k++)
					c(i, j) += a(k, i) * b(k, j);
			}
		return c;
	}

	/**
	 * Computes the sum of this matrix and b
	 */
	Matrix_ operator+(const Matrix_& b) const
	{
		const Matrix_& a = *this;
		Matrix_ c(a.rows(), a.cols());
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
	Matrix_ operator-(const Matrix_& b) const
	{
		const Matrix_& a = *this;
		Matrix_ c(a.rows(), a.cols());
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
	Matrix_ operator*(T s) const
	{
		const Matrix_& a = *this;
		Matrix_ c(a.rows(), a.cols());
		for (int i = 0; i < c.rows(); i++)
			for (int j = 0; j < c.cols(); j++)
				c(i, j) = a(i, j) * s;
		return c;
	}

	/**
	 * Adds a matrix to this
	 */
	void operator+=(const Matrix_& b)
	{
		Matrix_& a = *this;
		if (a.rows() != b.rows() || a.cols() != b.cols())
			return;
		for (int i = 0; i < a.rows(); i++)
			for (int j = 0; j < a.cols(); j++)
				a(i, j) += b(i, j);
	}

	/**
	 * Subtracts a matrix from this
	 */
	void operator-=(const Matrix_& b)
	{
		Matrix_& a = *this;
		if (a.rows() != b.rows() || a.cols() != b.cols())
			return;
		for (int i = 0; i < a.rows(); i++)
			for (int j = 0; j < a.cols(); j++)
				a(i, j) -= b(i, j);
	}

	/**
	 * Multiplies this matrix by a scalar
	 */
	void operator*=(T s)
	{
		Matrix_& a = *this;
		for (int i = 0; i < a.rows(); i++)
			for (int j = 0; j < a.cols(); j++)
				a(i, j) *= s;
	}

	void negate()
	{
		Matrix_& a = *this;
		for (int i = 0; i < a.rows(); i++)
			for (int j = 0; j < a.cols(); j++)
				a(i, j) = -a(i, j);
	}

	/**
	 * Returns this matrix negated (multiplied by -1)
	 */
	Matrix_ operator-() const
	{
		const Matrix_& a = *this;
		Matrix_ c(a.rows(), a.cols());
		for (int i = 0; i < c.rows(); i++)
			for (int j = 0; j < c.cols(); j++)
				c(i, j) = -a(i, j);
		return c;
	}

	/**
	Returns the matrix Frobenius norm squared
	*/
	T normSq() const
	{
		T s = 0;
		const Matrix_& a = *this;
		for (int i = 0; i < a.rows(); i++)
			for (int j = 0; j < a.cols(); j++)
				s += sqr(a(i, j));
		return s;
	}

	/**
	Returns the matrix Frobenius norm
	*/
	T norm() const { return sqrt(normSq()); }

	friend Matrix_ operator*(T s, const Matrix_& b) { return b * s; }
};


typedef Matrix_<double> Matrixd;
typedef Matrix_<float> Matrix;

struct SolveParams
{
	int maxiter;
	double maxerr;
	double delta;
	SolveParams(int mi = 50, double me = 0.0001, double d = 0) : maxiter(mi), maxerr(me), delta(d) {}
};


template<class T>
Matrix_<T> solve_(Matrix_<T>& A, Matrix_<T>& b);

/**
 * Solves the matrix equation A*x=b and returns x; if b is a matrix (not a column) then the equation is solved
 * for each of b's columns and solutions returned as the columns of the returned matrix; if A is not square (more equations than unknowns),
 then a least-squares solution is computed
 * \ingroup Math3D
 */
template<class T>
Matrix_<T> solve(const Matrix_<T>& A, const Matrix_<T>& b)
{
	if (A.rows() != A.cols())
	{
		Matrix_<T> A2 = A.transposed(A), b2 = A.transposed(b);
		return solve_(A2, b2);
	}

	Matrix_<T> A2 = A.clone();
	Matrix_<T> b2 = b.clone();
	return solve_(A2, b2);
}

template<class T>
Matrix_<T> Matrix_<T>::inverse() const
{
	return solve(*this, Matrix_<T>::identity(this->rows()));
}


template<class T>
Matrix_<T> solve_(Matrix_<T>& A_, Matrix_<T>& b_)
{
	if (A_.rows() != A_.cols())
	{
		Matrix_<T> A2 = A_.transposed(A_), b2 = A_.transposed(b_);
		return solve_(A2, b2);
	}
	Matrix_<T> x(b_.rows(), b_.cols());
	int n = A_.rows();
	Array<int> _(n);
	Matrix_<T> A = b_.cols() == 1 ? A_ : A_.clone();
	Matrix_<T> b = b_;

	for (int j = 0; j < b_.cols(); j++)
	{
		if (j > 0)
			A.copy(A_);
		for (int i = 0; i < n; i++)
			_[i] = i;
		for (int k = 0; k < n - 1; k++)
		{
			T max = 0;
			int ipivot = 0;
			for (int i = k; i < n; i++)
			{
				if (max < fabs(A(_[i], k))) {
					max = fabs(A(_[i], k));
					ipivot = i;
				}
			}

			swap(_[k], _[ipivot]);

			for (int i = k + 1; i < n; i++)
			{
				int ii = _[i], kk =_[k];
				T f = -A(ii, k) / A(kk, k);
				for (int jj = k; jj < n; jj++)
					A(ii, jj) += A(kk, jj) * f;
				b(ii, j) += b(kk, j) * f;
			}
		}

		for (int k = n - 1; k >= 0; k--)
		{
			T sum = 0;
			int kk = _[k];
			for (int i = k + 1; i < n; i++)
				sum += A(kk, i) * x(i, j);
			x(k, j) = (b(kk, j) - sum) / A(kk, k);
		}
	}
	return x;
}

/**
* Solves a system of equations F(x)=[0], given by functor f, which returns a vector of function values for an input vector x;
* and using x0 as initial guess. If there are more equations than unknowns (f larger than x0) then a least-squares solution is
* sought.
* 
* Example:
* 
* Solve for (x, y) in:
* -   (x-1)<sup>2</sup> + (y-1)<sup>2</sup> - 1 = 0
* -   sin(x) + sin(y) - x = 0
* 
* ~~~
* auto x = solveZero([](const Matrixd& x) {  // unknowns are x[0], x[1]
* 	return Matrixd{
* 		sqr(x[0] - 1) + sqr(x[1] - 1) - 1,
* 		sin(x[0]) + sin(x[1]) - x[0]
* 	};
* }, { 0.0, 0.0 });  // initial estimation
* ~~~
* \ingroup Math3D
*/
template <class T, class F>
Matrix_<T> solveZero(F f, const Matrix_<T>& x0, const SolveParams& p = SolveParams())
{
	T dx = p.delta > 0 ? p.delta : sizeof(T) == sizeof(float) ? T(1e-5) : T(1e-6);
	Matrix_<T> x = x0.clone();
	int nf = f(x).rows(), nb = 0;
	T me = T(p.maxerr);
	Matrix_<T> J(nf, x.rows());
	T r = 0, r0 = T(1e38);
	for (int it = 0; it < p.maxiter; it++)
	{
		Matrix_<T> f1 = f(x);
		r = f1.norm();
		nb = (r > r0) ? nb + 1 : 0;
		if (r < me || nb > 3)
			break;
		r0 = r;

		for (int j = 0; j < J.cols(); j++)
		{
			T x0 = x[j];
			x[j] += dx;
			Matrix_<T> f2 = f(x);
			x[j] = x0;
			for (int i = 0; i < J.rows(); i++)
				J(i, j) = (f2[i] - f1[i]) / dx;
		}

		f1.negate();
		Matrix_<T> h = solve_(J, f1);
		if (h.norm() < T(me))
			break;

		x += h;
	}

	return x;
}

#ifdef ASL_HAVE_INITLIST
template <class T, class F>
Matrix_<T> solveZero(F f, const std::initializer_list<T>& x0, const SolveParams& p = SolveParams())
{
	return solveZero(f, Matrix_<T>(x0));
}
#endif

/**
 * Solves the equation f(x)=0 given by functor f using an iterative method starting at x0 as initial estimate.
 * \ingroup Math3D
 */
template<class T, class F>
T solveZero(F f, T x0, const SolveParams& p = SolveParams())
{
	T dx = p.delta > 0 ? p.delta : T(1e-3);
	T x1 = x0 + dx, x2 = x0;
	T y0 = f(x0);
	for (int i = 0; i < p.maxiter; i++)
	{
		T y1 = f(x1);
		if (fabs(y1) < T(0.01 * p.maxerr))
			break;
		x2 = x1 - y1 * (x1 - x0) / (y1 - y0);
		x0 = x1;
		y0 = y1;
		x1 = x2;
		if (fabs(x1 - x0) < T(0.01 * p.maxerr))
			break;
	}
	return x2;
}
}

#endif
