// Copyright(c) 1999-2018 ASL author
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_ARRAY_H
#define ASL_ARRAY_H

// Define ASL_DEBUG_ARRAY to emit an exception if array bounds exceeded (useful for debugging)

#pragma warning( disable : 4284 )
#pragma warning( disable : 4251 )

namespace asl {
class String;
template<class T>
class Array;
}

#include <asl/defs.h>
#include "foreach1.h"
#include <string.h>
#include <stdlib.h>

#ifdef ASL_HAVE_INITLIST
#include <initializer_list>
#endif

namespace asl {

template<class T, int N>
class Array_;

/**
An Array is a contiguous and resizable array of any type of elements.

It can be used as a normal fixed-size array. The `length()` property holds its size:

~~~
Array<int> numbers(3);
numbers[0] = 4;
numbers[1] = 3;
numbers[2] = -2;

for(int i=0; i<numbers.length(); i++)
	cout << numbers[i] << endl;
~~~

Elements can be appended at the end at any moment or the array resized or cleared:

~~~
Array<String> names;
names << "John" << "Susan" << "David";  // -> length = 3
names.resize(5);  // -> length = 5
names.clear();    // -> length = 0
~~~

And you can concatenate the elements as a string if they are convertible to String:

~~~
String all = numbers.join(", "); // -> "4, 3, -2"
~~~

\ingroup Containers
*/


template <class T>
class Array
{
protected:
	T* _a;
	struct Data{int n, s; AtomicCount rc; int pad;}; // n=num. elems, s=allocated size
	Data& d() const {return *((Data*)_a-1);}
	void alloc(int m);
	void free();
	Array(T* p);
	operator void*() {}
	Array(const String& s);
	Array& operator=(int b) {}
public:
	/**
	Creates an empty array
	*/
	Array() {alloc(0);}
	/**
	Creates an array of n elements
	*/
	Array(int n) {alloc(n);}
	/**
	Creates an array of n elements and copies them from the pointer p
	*/
	Array(const T* p, int n) {alloc(n); for(int i=0; i<n; i++) _a[i]=p[i];}
	/**
	Creates an array of n elements and gives them the value x
	*/
	Array(int n, const T& x) { alloc(n); for (int i = 0; i<n; i++) _a[i] = x; }
	template<class K>
	Array(const Array<K>& b)
	{
		alloc(b.length());
		for(int i=0; i<length(); i++)
			_a[i]=(T)b[i];
	}
	Array(const Array& b) {_a=b._a; ++d().rc;}
#ifdef ASL_HAVE_MOVE
	Array(Array&& b)
	{
		_a = b._a;
		b._a = 0;
	}
	void operator=(Array&& b)
	{
		_a = b._a;
		b._a = 0;
	}
#endif
#ifdef ASL_HAVE_INITLIST
	Array(std::initializer_list<T> b)
	{
		int m = (int)b.size();
		const T* p = b.begin();
		alloc(m);
		for (int i = 0; i<m; i++)
			_a[i] = p[i];
	}
#endif
	~Array() {if(_a && --d().rc==0) free();}
	struct Enumerator
	{
		Array<T>& a;
		int i, j;
		Enumerator(Array& a_): a(a_), i(0), j(a.length()) {}
		Enumerator(Array& a_, int i_, int j_): a(a_), i(i_), j(j_) {}
		Enumerator(const Array& a_): a((Array&)a_), i(0), j(a_.length()) {}
		bool operator!=(const Enumerator& e) const {return (bool)*this;}
		void operator++() {i++;}
		T& operator*() {return a[i];}
		int operator~() {return i;}
		T* operator->() {return &(a[i]);}
		operator bool() const {return i < j;}
		T& operator[](int k) {return a[i+k];}
		const T& operator[](int k) const {return a[i+k];}
		int length() const {return j-i;}
		void operator=(const Enumerator& e) {for(int k=0; k<length(); k++) a[i+k] = e[k];}
	};
	int r() {return d().rc;}
	/**
	Returns the number of elements in the array
	*/
	int length() const {return d().n;}
	/**
	Resizes the array to a capacity of m elements. Up to m existing elements are preserved
	*/
	void resize(int m);
	/**
	Reserves space for m elements without changing actual length
	*/
	void reserve(int m) { int n = length(); resize(m); resize(n); }
	/**
	Removes all elements in the array
	*/
	void clear() { resize(0); }
	/* Frees all elements (with delete) and clears the array. Elements must be pointers */
	void destroy() {for(int i=0; i<length(); i++) delete _a[i]; clear();}
	/**
	Returns a pointer to the base of the array
	*/
	operator const T*() const {return &_a[0];}
	operator T*() {return &_a[0];}
	const T* ptr() const {return &_a[0];}
	T* ptr() {return &_a[0];}
	//operator bool() const { return d().n != 0; }
	operator const void*() const { return d().n == 0 ? 0 : this; }
	//explicit operator bool() const { return d().n != 0; }
	bool operator!() const { return d().n == 0; }
	/**
	Tests for equality of all elements of both arrays
	*/
	bool operator==(const Array& b) const
	{
		if(length() != b.length())
			return false;
		for(int i=0; i<length(); i++)
			if(b._a[i] != _a[i])
				return false;
		return true;
	}
	bool operator!=(const Array&b) const { return !(*this == b); }
	/**
	Returns the element at index i
	*/
	const T& operator[](int i) const {
#ifdef ASL_DEBUG_ARRAY
		if(i>=length()) {_a[i]=_a[0];return *(const T*)0;}
#endif
		return _a[i];
	}
	/**
	Returns the element at index i
	*/
	T& operator[](int i) {
#ifdef ASL_DEBUG_ARRAY
		if(i>=length()) {_a[i]=_a[0]; return *(T*)0;}
#endif
		return _a[i];
	}

	/**
	Returns a reference to the last element
	*/
	const T& last() const {
		return _a[length()-1];
	}
	
	/**
	Returns a reference to the last element
	*/
	T& last() {
		return _a[length()-1];
	}
	/**
	Returns the index of the first element with value x. The search starts at position j.
	The value -1 is returned if no such element is found
	*/
	int indexOf(const T& x, int j=0) const
	{
		for(int i=j; i<length(); i++) {if(_a[i]==x) return i;}
		return -1;
	}
	/**
	Returns true if the array contains an element equal to x
	*/
	bool contains(const T& x) const {return indexOf(x) >= 0;}
	/**
	Makes this array independent of others
	*/
	Array& dup()
	{
		if(d().rc==1) return *this;
		Array b(d().n);
		for(int i=0; i<d().n; i++)
			b._a[i]=_a[i];
		(*this)=b;
		return *this;
	}
	/**
	Returns an independent copy of this array
	*/
	Array clone() const
	{
		Array b(*this);
		return b.dup();
	}
	/**
	Copies another array's contents into this array
	*/
	void copy(const Array& b)
	{
		int n = b.length();
		resize(n);
		for (int i = 0; i<n; i++)
			_a[i] = b._a[i];
	}

#ifdef ASL_HAVE_INITLIST

	Array& operator=(std::initializer_list<T> b)
	{
		int n = (int)b.size();
		const T* p = b.begin();
		resize(n);
		for (int i = 0; i<n; i++)
			_a[i] = p[i];
		return *this;
	}

#endif
	/**
	Assigns array b into this array by reference.
	*/
	Array& operator=(const Array& b)
	{
		if(this==&b) return *this;
		if(--d().rc==0) free();
		_a=b._a;
		++d().rc;
		return *this;
	}
	/**
	Adds element x at the end of the array
	*/
	Array& operator<<(const T& x)
	{
		insert(-1, x);
		return *this;
	}
	/**
	The same as `<<`, useful to create pseudo-array-literals
	*/
	Array& operator,(const T& x) { return (*this) << x; }
	/**
	Removes the element at position i.
	*/
	void remove(int i)
	{
		asl_destroy(_a+i);
		memmove(_a+i, _a+i+1, (d().n-i-1)*sizeof(T));
		d().n --;
		resize(d().n);
		return;
	}
	/**
	Removes n elements starting at position i.
	*/
	void remove(int i, int n)
	{
		asl_destroy(_a+i, n);
		memmove(_a+i, _a+i+n, (d().n-i-n)*sizeof(T));
		d().n -= n;
		resize(d().n);
		return;
	}
	/*
	Removes n elements starting at position i.
	*/
	void remove(Enumerator& i, int n=1)
	{
		asl_destroy(_a+i.i, n);
		memmove(_a+i.i, _a+i.i+n, (d().n-i.i-n)*sizeof(T));
		d().n -= n;
		--i.i;
		--i.j;
		resize(d().n);
		return;
	}
	/**
	Removes the first element with value x starting at index i0.
	Returns true if an elemento was found and removed.
	*/
	bool removeOne(const T& x, int i0=0)
	{
		int i = indexOf(x);
		if(i != -1) {
			remove(i);
			return true;
		}
		return false;
	}
	/**
	Inserts x at position k
	*/
	void insert(int k, const T& x);
	/**
	Returns the elements of the array in reversed order
	*/
	Array reversed() const
	{
		Array b(length());
		for (int i=0; i<length(); i++)
			b[i] = _a[length()-i-1];
		return b;
	}
	/**
	Returns a section of the array, from element i1 up to but not including element i2.
	If i2 is omitted the subarray will take elements up te the last
	*/
	Array slice(int i1, int i2=0) const
	{
		if(i2==0)
			i2=length();
		Array b(i2-i1);
		for (int i=i1; i<i2; i++)
			b[i-i1] = _a[i];
		return b;
	}
	/**
	Same as slice() \deprecated
	*/
	Array sslice(int i1, int i2 = 0) const
	{
		return slice(i1, i2);
	}
	/**
	Adds all elements from array b at the end of the array
	*/
	Array& append(const Array& b)
	{
		int n=length();
		resize(length()+b.length());
		for (int i=0; i<b.length(); i++)
			_a[n+i] = b[i];
		return *this;
	}
	/**
	Adds n elements from array pointed by p at the end of this array
	*/
	Array& append(const T* p, int n)
	{
		int m=length();
		resize(m + n);
		for (int i=0; i<n; i++)
			_a[m+i] = p[i];
		return *this;
	}

#ifdef ASL_HAVE_INITLIST
	Array& append(const std::initializer_list<T>& b)
	{
		int n = (int)b.size();
		const T* p = b.begin();
		int m = length();
		resize(m + n);
		for (int i = 0; i<n; i++)
			_a[m + i] = p[i];
		return *this;
	}
	// Array& operator<<(const std::initializer_list<T>& b) { append(b); return *this; }
#endif
	// Array& operator<<(const Array& b) { append(b); return *this; }

	/**
	Returns the concatenation of this array and b
	*/
	Array concat(const Array& b) const
	{
		Array a(clone());
		return a.append(b);
	}
	
	/**
	Returns the concatenation of this array and b
	*/
	Array operator|(const Array& b) const
	{
		return concat(b);
	}

	/**
	Sorts the array using the elements' < operator "in place"
	*/
	Array& sort()
	{
		quicksort(_a, length());
		return *this;
	}
	/**
	Sorts the array using the elements' < operator "in place"
	*/
	template<class Less>
	Array& sort(Less f)
	{
		quicksort(_a, length(), f);
		return *this;
	}
/*	/// Adds all elements from array b at the end of the array
	Array& operator<<(const Array& b)
	{
		append(b);
		return *this;
	}*/
	/**
	Returns a string representation of the array, formed by joining its
	elements with commas. The elements need to be convertible to String
	*/
	///operator String() const;
	/**
	Returns a string representation of the array, formed by joining its
	elements with the given separator string sep. The elements need to be
	convertible to String
	*/
	String join(const String& sep) const;
	/**
	Returns an array formed by applying a function to each item
	*/
	template<class F>
	Array map(F f) const
	{
		Array b(length());
		for (int i = 0; i < length(); i++)
			b[i] = f(_a[i]);
		return b;
	}
	/**
	Removes items that meet a predicate
	*/
	template<class F>
	Array& removeIf(F f)
	{
		for (int i = 0; i < length(); i++)
			if (f(_a[i])) {
				remove(i);
				--i;
			}
		return *this;
	}

	/**
	Removes the last item in the array
	*/
	void removeLast()
	{
		if (length() > 0)
			remove(length() - 1);
	}

	Enumerator all() {return Enumerator(*this);}
	Enumerator all() const {return Enumerator(*this);}
	Enumerator slice_(int i, int j=0) {if(j==0) j=length();return Enumerator(*this, i, j);}
	/*
	template<class E>
	Array(const E& e_) {
		printf("Array<E>\n");
		alloc(0);
		E e(e_);
		while(e) {*this << *e; ++e;}
	}*/
	/* // TODO: check compat
	template<int N>
	Array(const Array_<T,N> & b)
	{
		alloc(N);
		for(int i=0; i<N; i++)
			_a[i]=b[i];
	}
	*/
};

template <class T>
void Array<T>::resize(int m)
{
	//if(length()==m)
	//	return;
	int s=d().s;
	int s1 = (m > s)? max(8*s/4, m) : s;
	T* b = _a;
	int n=d().n;
	if(s1 != s && s*sizeof(T) < 2048)
	{
		char* p = (char*) malloc( s1*sizeof(T)+sizeof(Data) );
		if(!p)
			ASL_BAD_ALLOC();
		b = (T*) ( p + sizeof(Data) );
		int i = min(m,n), j = sizeof(T), k = i*j;
		memcpy(b, _a, k);
	}
	else if(s1 != s)
	{
		char* p = (char*) realloc( (char*)_a-sizeof(Data), s1*sizeof(T)+sizeof(Data) );
		if(!p)
			ASL_BAD_ALLOC();
		b = (T*) ( p + sizeof(Data) );
		_a = b;
		s1 = s;
		d().s = s1;
	}
	if(n<m) asl_construct(b+n, m-n);
	else asl_destroy(_a+m, n-m);
	if(s1 != s)
	{
		int rc = d().rc;
		char* p = (char*)_a - sizeof(Data);
		::free(p);
		_a = b;
		d().rc = rc;
		d().s = s1;
	}
	d().n = m;
}

template <class T>
void Array<T>::insert(int k, const T& x)
{
	Data* h = &d();
	int n = h->n;
	int s = h->s;
	if(k==-1)
		k = n;
	if(n < s) {}
	else
	{
		int s1 = 2*s;
		/*if(s < 512/sizeof(T) || s > 200000/sizeof(T))
		{
			char* p = (char*) malloc( s1*sizeof(T) + sizeof(Data) );
			if(!p)
				ASL_BAD_ALLOC();
			T* b = (T*) ( p + sizeof(Data) );
			memcpy(b, _a, n*sizeof(T));//n was k
			int rc=h->rc;
			::free( (char*)_a - sizeof(Data) );
			_a = b;
			h = &d();
			h->rc=rc;
			h->s=s1;
		}
		else*/
		{
			char* p = (char*) realloc( (char*)_a-sizeof(Data), s1*sizeof(T)+sizeof(Data) );
			if(!p)
				ASL_BAD_ALLOC();
			T* b = (T*) ( p + sizeof(Data) );
			_a = b;
			h = &d();
			h->s=s1;
		}
	}
	if(k<n) {
		memmove((char*)_a+(k+1)*sizeof(T), (void*)(_a+k), (n-k)*sizeof(T));
	}
	asl_construct_copy(_a+k, x);
	h->n = n+1;
}

template<class T>
void Array<T>::alloc(int m)
{
	int s=max(m, 3);
	char* p = (char*) malloc( s*sizeof(T)+sizeof(Data) );
	if(!p)
		ASL_BAD_ALLOC();
	_a = (T*) ( p + sizeof(Data) );
	d().s = s;
	d().n = m;
	d().rc=1;
	asl_construct(_a, m);
}

template<class T>
void Array<T>::free()
{
	asl_destroy(_a, d().n);
	::free( (char*)_a - sizeof(Data) );
	_a=0;
}

#ifdef ASL_HAVE_RANGEFOR

template<class T>
typename Array<T>::Enumerator begin(const Array<T>& a)
{
	return a.all();
}

template<class T>
typename Array<T>::Enumerator end(const Array<T>& a)
{
	return a.all();
}

#endif

#ifdef ASL_HAVE_INITLIST

template <class T>
Array<T> array(std::initializer_list<T> b)
{
	return Array<T>(b);
}

#endif

/*
Creates an array with the element given as argument
*/
template <class T>
Array<T> array(const T& a0)
{
	Array<T> _a(1);
	_a[0]=a0;
	return _a;
}

/**
Creates an array with the 2 elements given as arguments (there are overloads from 1 to 6 elements)
\ingroup Containers
*/
template <class T>
Array<T> array(const T& a0, const T& a1)
{
	Array<T> _a(2);
	_a[0]=a0;
	_a[1]=a1;
	return _a;
}

/*
Creates an array with the 3 elements given as arguments
*/
template <class T>
Array<T> array(const T& a0, const T& a1, const T& a2) {
	Array<T> _a(3);
	_a[0]=a0;
	_a[1]=a1;
	_a[2]=a2;
	return _a;
}

/*
Creates an array with the 4 elements given as arguments
*/
template <class T>
Array<T> array(const T& a0, const T& a1, const T& a2, const T& a3) {
	Array<T> _a(4);
	_a[0]=a0;
	_a[1]=a1;
	_a[2]=a2;
	_a[3]=a3;
	return _a;
}


/*
Creates an array with the 5 elements given as arguments
*/
template <class T>
Array<T> array(const T& a0, const T& a1, const T& a2, const T& a3, const T& a4) {
	Array<T> _a(5);
	_a[0]=a0;
	_a[1]=a1;
	_a[2]=a2;
	_a[3]=a3;
	_a[4]=a4;
	return _a;
}

template <class T>
Array<T> array(const T& a0, const T& a1, const T& a2, const T& a3, const T& a4, const T& a5) {
	Array<T> _a(6);
	_a[0] = a0;
	_a[1] = a1;
	_a[2] = a2;
	_a[3] = a3;
	_a[4] = a4;
	_a[5] = a5;
	return _a;
}

template<class T>
static asl::Array<T> deg2rad(const asl::Array<T>& a)
{
	asl::Array<T> b(a.length());
	for (int i = 0; i < a.length(); i++)
		b[i] = asl::deg2rad(a[i]);
	return b;
}

template<class T>
static asl::Array<T> rad2deg(const asl::Array<T>& a)
{
	asl::Array<T> b(a.length());
	for (int i = 0; i < a.length(); i++)
		b[i] = asl::rad2deg(a[i]);
	return b;
}

}
#endif
