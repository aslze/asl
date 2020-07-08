// Copyright(c) 1999-2020 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_ARRAY2_H
#define ASL_ARRAY2_H

#include <asl/Array.h>

namespace asl {

template <class T>
class Array2
{
	asl::Array<T> _a;
	int _rows, _cols;
public:
	Array2(): _rows(0), _cols(0) {}

	Array2(int r, int c) :
		_a(r* c), _rows(r), _cols(c)
	{}

	void resize(int r, int c)
	{
		_a.resize(r * c);
		_rows = r;
		_cols = c;
	}

	int rows() const {
		return _rows;
	}

	int cols() const {
		return _cols;
	}

	T& operator()(int i, int j)
	{
		return _a[i * _cols + j];
	}

	const T& operator()(int i, int j) const
	{
		return _a[i * _cols + j];
	}
};

}

#endif
