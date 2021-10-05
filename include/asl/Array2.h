// Copyright(c) 1999-2021 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_ARRAY2_H
#define ASL_ARRAY2_H

#include <asl/Array.h>

namespace asl {

/**
A simple 2-dimensional array or matrix
*/
template <class T>
class Array2
{
	asl::Array<T> _a;
	int _rows, _cols;
public:

	/**
	Creates an empty array
	*/
	Array2(): _rows(0), _cols(0) {}

	/**
	Creates an array with size rows x cols
	*/
	Array2(int rows, int cols) :
		_a(rows * cols), _rows(rows), _cols(cols)
	{}

	/**
	Creates an array with size rows x cols and initializes all items with value
	*/
	Array2(int rows, int cols, const T& value) :
		_a(rows * cols, value), _rows(rows), _cols(cols)
	{}

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
	Resizes the matrix to the r x c
	*/
	void resize(int r, int c)
	{
		_a.resize(r * c);
		_rows = r;
		_cols = c;
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

	void set(const T& x)
	{
		for (int i = 0; i < _a.length(); i++)
			_a[i] = x;
	}
};

}

#endif
