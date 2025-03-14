// Copyright(c) 1999-2024 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_ARRAYX_H
#define ASL_ARRAYX_H

#include <asl/defs.h>
#include <asl/Array.h>
#include <asl/String.h>

namespace asl {

class String;

/**
This class represents a fixed-length array. It is similar to Array but the array length is fixed
and must be known at compile time. It is much faster to create and copy than a dynamic Array.

~~~
Array_<int,2> getMinMax()
{
	return array_(min, max); // or C++11: return {min, max};
}
~~~
\ingroup Containers
*/

template <class T, int N>
class Array_
{
protected:
	T _a[N];
	operator void* () { return NULL; }
public:
	Array_() {}

	template<class K>
	Array_(const Array_<K, N>& b)
	{
		for(int i = 0; i < N; i++)
			_a[i] = (T)b[i];
	}
	Array_(const Array_& b)
	{
		for(int i = 0; i < N; i++)
			_a[i] = b._a[i];
	}
#ifdef ASL_HAVE_INITLIST
	Array_(std::initializer_list<T> b)
	{
		ASL_ASSERT(b.size() == N);
		const T* p = b.begin();
		for (int i = 0; i < N; i++)
			_a[i] = p[i];
	}
#endif
	~Array_()
	{}

	template<class T2>
	Array_<T2, N> with() const
	{
		Array_<T2, N> b;
		for (int i = 0; i < N; i++)
			b[i] = (T)_a[i];
		return b;
	}

	Array<T> array() const
	{
		return Array<T>(&_a[0], N);
	}

	struct Enumerator
	{
		Array_<T,N>& a;
		int i, j;
		Enumerator(Array_& a_): a(a_), i(0), j(N) {}
		Enumerator(Array_& a_, int i_, int j_): a(a_), i(i_), j(j_) {}
		Enumerator(const Array_& a_): a((Array_&)a_), i(0), j(N) {}
		bool operator!=(const Enumerator& e) const { return (bool)*this; }
		void operator++() {i++;}
		T& operator*() {return a[i];}
		int operator~() {return i;}
		T* operator->() {return &(a[i]);}
		operator bool() const {return i < j;}
		T& operator[](int k) {return a[i+k];}
		const T& operator[](int k) const {return a[i+k];}
		int length() const {return j-i;}
	};
	/** Returns the number of elements in the array */
	int length() const {return N;}

	/** Returns a pointer to the base of the array */
	operator const T*() const {return &_a[0];}
	operator T*() {return &_a[0];}

	const T* ptr() const { return &_a[0]; }
	T* ptr() { return &_a[0]; }

	const T* data() const { return &_a[0]; }
	T* data() { return &_a[0]; }

	/** Tests for equality of all elements of both arrays*/
	bool operator==(const Array_& b) const
	{
		for (int i = 0; i < N; i++)
			if(_a[i] != b._a[i])
				return false;
		return true;
	}

	bool operator!=(const Array_& b) const { return !(*this == b); }

	bool operator<(const Array_& b) const
	{
		for (int i = 0; i < N; i++)
			if (_a[i] < b._a[i])
				return true;
		return false;
	}

	/** Returns the element at index i */
	const T& operator[](int i) const
	{
		return _a[i];
	}
	/** Returns the element at index i */
	T& operator[](int i)
	{
		return _a[i];
	}

	/** Returns a reference to the last element */
	const T& last() const
	{
		return _a[N-1];
	}
	
	/** Returns a reference to the last element */
	T& last()
	{
		return _a[N-1];
	}
	/** Returns the index of the first element with value x. The search starts at position j.
	The value -1 is returned if no such element is found*/
	int indexOf(const T& x, int j=0) const
	{
		for(int i = j; i < N; i++) {if(_a[i]==x) return i;}
		return -1;
	}
	/** Returns true if the array contains an element equal to x */
	bool contains(const T& x) const {return indexOf(x) >= 0;}

	/** Assigns array b into this array by reference. */
	Array_& operator=(const Array_& b)
	{
		if(this == &b) return *this;
		for(int i = 0; i < N; i++)
			_a[i] = b[i];
		return *this;
	}
	Array_ reversed() const
	{
		Array_ b;
		for (int i = 0; i < N; i++)
			b[i] = _a[N - i - 1];
		return b;
	}
	/* Returns a section of the array, M elements from element i1 */
	template<int M>
	Array_<T, M> slice(int i1) const
	{
		Array_<T, M> b;
		for (int i = 0; i < M; i++)
			b[i] = _a[i + i1];
		return b;
	}
	/** Sorts the array using the elements' < operator "in place" */
	Array_& sort()
	{
		quicksort(_a, N);
		return *this;
	}
	/** Sorts the array using the elements' < operator "in place" */
	template<class Less>
	Array_& sort(Less f)
	{
		quicksort(_a, N, f);
		return *this;
	}
	/**
	Returns a string representation of the array, formed by joining its
	elements with the given separator string sep. The elements need to be
	convertible to String
	*/
	String join(const String& sep) const;

	Enumerator all() {return Enumerator(*this);}
	Enumerator all() const {return Enumerator(*this);}
};

template<class T, int N>
String Array_<T, N>::join(const String& sep) const
{
	String s = _a[0];
	for(int i=1; i<N; i++) {s += sep; String v=_a[i]; s += (v);}
	return s;
}

#ifdef ASL_HAVE_RANGEFOR

template<class T, int N>
typename Array_<T, N>::Enumerator begin(const Array_<T, N>& a)
{
	return a.all();
}

template<class T, int N>
typename Array_<T, N>::Enumerator end(const Array_<T, N>& a)
{
	return a.all();
}

#endif


/**
Creates an array with the 2 elements given as arguments (there are overloads from 1 to 6 elements)
\ingroup Containers
\deprecated
*/
template <class T>
Array_<T,2> array_(const T& a0, const T& a1)
{
	Array_<T,2> a;
	a[0]=a0;
	a[1]=a1;
	return a;
}

/*
Creates an array with the 3 elements given as arguments
\deprecated
*/
template <class T>
Array_<T,3> array_(const T& a0, const T& a1, const T& a2) {
	Array_<T,3> a;
	a[0]=a0;
	a[1]=a1;
	a[2]=a2;
	return a;
}

/*
Creates an array with the 4 elements given as arguments
\deprecated
*/
template <class T>
Array_<T,4> array_(const T& a0, const T& a1, const T& a2, const T& a3) {
	Array_<T,4> a;
	a[0]=a0;
	a[1]=a1;
	a[2]=a2;
	a[3]=a3;
	return a;
}

/*
Creates an array with the 5 elements given as arguments
\deprecated
*/
template <class T>
Array_<T,5> array_(const T& a0, const T& a1, const T& a2, const T& a3, const T& a4) {
	Array_<T,5> a;
	a[0]=a0;
	a[1]=a1;
	a[2]=a2;
	a[3]=a3;
	a[4]=a4;
	return a;
}

}
#endif
