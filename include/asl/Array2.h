// Copyright(c) 1999-2021 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_ARRAY2_H
#define ASL_ARRAY2_H

#include <asl/Array.h>

namespace asl {

struct IndexIJEnumerator;

/**
A simple 2-dimensional dynamic array or matrix

~~~
Array2<int> a(3, 3);

Array2<float> b = {
	{ 1, 2, 3 },
	{ 3, -1.5f, 0 }
};

b(1, 1) = 7.25f;
~~~

*/
template <class T>
class Array2
{
protected:
	Array<T> _a;
	int _rows, _cols;
public:

	/**
	Creates an empty array
	*/
	Array2(): _rows(0), _cols(0) {}

	/**
	Creates an array with size rows x cols
	*/
	ASL_EXPLICIT Array2(int rows, int cols) :
		_a(rows * cols), _rows(rows), _cols(cols)
	{}

	/**
	Creates an array with size rows x cols and initializes all items with value
	*/
	ASL_EXPLICIT Array2(int rows, int cols, const T& value) :
		_a(rows * cols, value), _rows(rows), _cols(cols)
	{}

	/**
	Creates an array of rows x cols elements and copies them from the pointer p (row-wise)
	*/
	ASL_EXPLICIT Array2(int rows, int cols, const T* p) :
		_a(rows * cols, p), _rows(rows), _cols(cols)
	{}

	ASL_EXPLICIT Array2(int rows, int cols, const Array<T>& a) :
		_a(a), _rows(rows), _cols(cols)
	{}

#ifdef ASL_HAVE_INITLIST
	/**
	Creates an array with size rows x cols and the given elements
	*/
	ASL_EXPLICIT Array2(int rows, int cols, std::initializer_list<T> a) :
		_a(a), _rows(rows), _cols(cols)
	{}

	/**
	Creates an array with given list of lists of elements
	*/
	Array2(std::initializer_list<std::initializer_list<T> > a)
	{
		_rows = (int)a.size();
		if (_rows == 0)
			return;
		const std::initializer_list<T>* p = a.begin();
		_cols = (int)p->size();
		_a.resize(_rows * _cols);
		for (int i = 0; i < _rows; i++)
		{
			const T* q = p[i].begin();
			for (int j = 0; j < _cols; j++)
				(*this)(i, j) = q[j];
		}
	}
#endif

	/**
	Creates a copy of this array with items converted to type K
	*/
	template<class K>
	Array2<K> with() const
	{
		return Array2<K>(_rows, _cols, _a.template with<K>());
	}

	Array<T>& data() { return _a; }

	/**
	Returns the internal array holding all values (row-major)
	*/
	const Array<T>& data() const { return _a; }

	/**
	Returns true if both arrays are equal
	*/
	bool operator==(const Array2& b) const
	{
		return _rows == b._rows && _cols == b._cols && _a == b._a;
	}

	/**
	Returns true if both arrays are not equal
	*/
	bool operator!=(const Array2& b) const
	{
		return !(*this == b);
	}

	/**
	Returns an independent copy of this array
	*/
	Array2 clone() const
	{
		Array2 b;
		b._a = _a.clone();
		b._rows = _rows;
		b._cols = _cols;
		return b;
	}

	/**
	Returns a sub-array consisting of a rows [i1, i2) and columns [j1, j2)
	*/
	Array2 slice(int i1, int i2, int j1, int j2) const
	{
		Array2 b(i2-i1, j2-j1);
		for (int i = 0; i < b.rows(); i++)
			for (int j = 0; j < b.cols(); j++)
				b(i, j) = (*this)(i1 + i, j1 + j);
		return b;
	}

	/**
	Resizes the matrix to r x c
	*/
	Array2& resize(int r, int c)
	{
		_a.resize(r * c);
		_rows = r;
		_cols = c;
		return *this;
	}

	/**
	Returns the number of rows
	*/
	int rows() const {
		return _rows;
	}

	/**
	Returns the number of columns
	*/
	int cols() const {
		return _cols;
	}

	/**
	Returns the item at row i, column j
	*/
	T& operator()(int i, int j)
	{
		return _a[i * _cols + j];
	}

	const T& operator()(int i, int j) const
	{
		return _a[i * _cols + j];
	}

	/**
	Sets all array items to value x
	*/
	void set(const T& x)
	{
		for (int i = 0; i < _a.length(); i++)
			_a[i] = x;
	}

	IndexIJEnumerator indices() const;
};

struct IndexIJ { int i, j; };

struct IndexIJEnumerator
{
	IndexIJ i;
	int m, n;
	bool more;
	IndexIJEnumerator all() const { return *(IndexIJEnumerator*)this; }
	IndexIJEnumerator(int m, int n) : m(m), n(n) { i.i = 0; i.j = 0; more = m * n > 0; }
	void operator++() { if (++i.j >= n) { i.i++; i.j = 0; if (i.i == m) more = false; } }
	operator bool() const { return more; }
	//void operator++() { if (++i.j >= n) { i.i++; i.j = 0; }	}
	//operator bool() const { return i.j < n && i.i < m; }
	bool operator!=(const IndexIJEnumerator& e) const { return (bool)*this; }
	const IndexIJ& operator*() const { return i; }
};

template <class T>
inline IndexIJEnumerator Array2<T>::indices() const { return IndexIJEnumerator(rows(), cols()); }

#ifdef ASL_HAVE_RANGEFOR

inline IndexIJEnumerator begin(const IndexIJEnumerator& e) { return e.all(); }
inline IndexIJEnumerator end(const IndexIJEnumerator& e) { return e.all(); }

template<class T>
typename Array<T>::Enumerator begin(const Array2<T>& a)
{
	return a.data().all();
}

template<class T>
typename Array<T>::Enumerator end(const Array2<T>& a)
{
	return a.data().all();
}

#endif

}

#endif
