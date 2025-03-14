// Copyright(c) 1999-2025 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_ARRAY_H
#define ASL_ARRAY_H

#include <asl/defs.h>
#include "foreach1.h"
#include <string.h>
#include <stdlib.h>

#ifdef ASL_HAVE_INITLIST
#include <initializer_list>
#endif

// Define ASL_DEBUG_ARRAY to emit an exception if array bounds exceeded (useful for debugging)

#ifdef _MSC_VER
#pragma warning(disable : 4251)
#pragma warning(push)
#pragma warning(disable : 4284 26451)
#endif

namespace asl {

class String;
template<class T>
class Array;
template<class T, int N>
class Array_;
class Var;

#ifdef ASL_NO_ATOMIC_OPS
#define ASL_RC_INIT() asl_construct(&d().rc);
#define ASL_RC_FREE() asl_destroy(&d().rc);
#else
#define ASL_RC_INIT()
#define ASL_RC_FREE()
#endif

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
	const Data& d() const { return *((const Data*)_a - 1); } // NOLINT
	Data&       d() { return *((Data*)_a - 1); } // NOLINT
	void alloc(int m);
	void free();
	ASL_EXPLICIT Array(T* p) {}
	/*ASL_EXPLICIT*/ operator void* () { return NULL; }
	ASL_EXPLICIT Array(const String& s) {}
	Array& operator=(int b) { return *this; }
	bool operator==(int x) const { return false; }
public:
	/**
	Creates an empty array
	*/
	Array() {alloc(0);}
	/**
	Creates an array of n elements
	*/
	ASL_EXPLICIT Array(int n) {alloc(n);}
	/**
	Creates an array of n elements and copies them from the pointer p
	*/
	ASL_EXPLICIT Array(const T* p, int n) { alloc(n); for (int i = 0; i < n; i++) _a[i] = p[i]; }
	/**
	Creates an array of n elements and gives them the value x
	*/
	ASL_EXPLICIT Array(int n, const T& x) { alloc(n); for (int i = 0; i<n; i++) _a[i] = x; }

	template<class K>
	Array(const Array<K>& b)
	{
		alloc(b.length());
		for(int i=0; i<length(); i++)
			_a[i]=(T)b[i];
	}
	Array(const Array& b)
	{
		_a = b._a;
		++d().rc;
	}
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
	~Array()
	{
		if (--d().rc == 0)
			free();
	}

	/**
	Returns a copy of this array with all element converted to another type
	*/
	template<class K>
	Array<K> with() const
	{
		Array<K> b(length());
		for (int i = 0; i < length(); i++)
			b[i] = (K)_a[i];
		return b;
	}

	struct Enumerator
	{
		Array<T>& a;
		int i, j;
		Enumerator(Array& a_): a(a_), i(0), j(a.length()) {}
		Enumerator(Array& a_, int i_, int j_): a(a_), i(i_), j(j_) {}
		Enumerator(const Array<T>& a_): a((Array<T>&)a_), i(0), j(a_.length()) {}
		bool operator!=(const Enumerator& e) const {return (bool)*this;}
		void operator++() {i++;}
		T& operator*() {return a[i];}
		int operator~() {return i;}
		T* operator->() {return &(a[i]);}
		operator bool() const {return i < j;}
		T& operator[](int k) {return a[i+k];}
		T& operator[](int k) const {return a[i+k];}
		int length() const {return j-i;}
	};

	int rc() const {return d().rc;}
	int cap() const { return d().s; }
	/**
	Returns the number of elements in the array
	*/
	int length() const {return d().n;}

	/**
	Reserves space for m elements without increasing actual length (to make appending faster)
	*/
	Array& reserve(int m);
	/**
	Resizes the array to m elements; up to m existing elements are preserved
	*/
	Array& resize(int m)
	{
		int n = d().n;
		reserve(m);
		if (m > n)
			asl_construct(_a + n, m - n);
		else if (m < n)
			asl_destroy(_a + m, n - m);
		d().n = m;
		return *this;
	}

	/**
	Removes all elements in the array
	*/
	void clear() { resize(0); }
	/* Frees all elements (with delete) and clears the array; elements must be pointers
	\deprecated Delete items explicitly or use smart pointers
	*/
	ASL_DEPRECATED(void destroy(), "") { for(int i=0; i<length(); i++) delete _a[i]; clear(); }
	/**
	Returns a pointer to the base of the array
	\deprecated This conversion will not be implicit! Use .data()
	*/
	ASL_DEPRECATED(operator const T*() const, "Use .data()") { return &_a[0]; }
	/**
	Returns a pointer to the base of the array
	\deprecated This conversion will not be implicit! Use .data()
	*/
	ASL_DEPRECATED(operator T*(), "Use .data()") { return &_a[0]; }
	/**
	Returns a pointer to the first element
	\deprecated Use .data()
	*/
	const T* ptr() const { return &_a[0]; }
	/**
	Returns a pointer to the first element
	\deprecated Use .data()
	*/
	T* ptr() { return &_a[0]; }

	/**
	Returns a pointer to the first element
	*/
	T* data() { return &_a[0]; } // NOLINT

	/**
	Returns a pointer to the first element
	*/
	const T* data() const { return &_a[0]; } // NOLINT

	bool operator!() const { return d().n == 0; }

#ifdef ASL_HAVE_EXPLICIT
	//ASL_EXPLICIT operator bool() const { return !!*this; }
#endif
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

	bool operator<(const Array& b) const
	{
		int n = min(length(), b.length());
		bool eq = true;
		for (int i = 0; i < n; i++)
			if (_a[i] < b._a[i])
				return true;
			else if (_a[i] != b._a[i])
				eq = false;
		return eq? length() < b.length() : false;
	}

	/**
	Returns the element at index i
	*/
	const T& operator[](int i) const {
#ifdef ASL_DEBUG_ARRAY
		if(i>=length()) {_a[i]=_a[0];return *(const T*)0;}
#endif
		return _a[i]; // NOLINT
	}
	/**
	Returns the element at index i
	*/
	T& operator[](int i) {
#ifdef ASL_DEBUG_ARRAY
		if(i>=length()) {_a[i]=_a[0]; return *(T*)0;}
#endif
		return _a[i]; // NOLINT
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
	Returns the index of the first element with value x; The search starts at position j;
	The value -1 is returned if no such element is found
	*/
	int indexOf(const T& x, int j=0) const
	{
		int n = length();
		for(int i=j; i<n; i++) {if(_a[i]==x) return i;}
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
		return (*this = clone());
	}
	/**
	Returns an independent copy of this array
	*/
	Array clone() const
	{
		Array b(_a, length());
		return b;
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

	/**
	Copies n elements pointed to by p (the array is resized to n)
	*/
	void copy(const T* p, int n)
	{
		resize(n);
		for (int i = 0; i < n; i++)
			_a[i] = p[i];
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
		if (_a == b._a)
			return *this;
		if(--d().rc==0) free();
		_a = b._a;
		++d().rc;
		return *this;
	}
	
	template<class K>
	Array& operator=(const Array<K>& b)
	{
		int n = b.length();
		resize(n);
		for (int i = 0; i<n; i++)
			_a[i] = (T)b[i];
		return *this;
	}

	Array& operator=(const Var& b);

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
	Removes n elements (1 be default) starting at position i.
	*/
	Array& remove(int i, int n = 1)
	{
		int m = d().n;
		if (i + n > m)
			return *this;
		asl_destroy(_a+i, n);
		memmove((char*)(_a + i), _a + i + n, (m - i - n)*sizeof(T));
		d().n -= n;
		resize(m - n);
		return *this;
	}
	/**
	Removes the first element with value x starting at index i0;
	Returns true if an element was found and removed.
	*/
	bool removeOne(const T& x, int i0=0)
	{
		int i = indexOf(x, i0);
		if(i != -1) {
			remove(i);
			return true;
		}
		return false;
	}
	/**
	Inserts x at position k
	*/
	Array& insert(int k, const T& x);
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
	Returns a section of the array, from element i1 up to but not including element i2;
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
#endif

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

	/**
	Sorts the array by the elements' f comparable property (in ascending order by default)
	*/
	template<class F>
	Array& sortBy(F f, bool ascending = true)
	{
		if (ascending)
		{
			IsLess<T, F> p(f);
			quicksort(_a, length(), p);
		}
		else
		{
			IsMore<T, F> p(f);
			quicksort(_a, length(), p);
		}
		return *this;
	}

	/**
	Returns a string representation of the array, formed by joining its
	elements with the given separator string sep; The elements need to be
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
	Returns an array of another type formed by applying a function to each item
	*/
	template<class K, class F>
	Array<K> map_(F f) const
	{
		Array<K> b(length());
		for (int i = 0; i < length(); i++)
			b[i] = f(_a[i]);
		return b;
	}
	/**
	Returns an array containing the items in this array satisfying a condition
	*/
	template<class F>
	Array filter(F f) const
	{
		Array b;
		b.reserve(length());
		for (int i = 0; i < length(); i++)
			if(f(_a[i]))
				b << _a[i];
		return b;
	}
	/**
	Removes items that meet a predicate
	*/
	template<class F>
	Array& removeIf(F f)
	{
		int n = length();
		for (int i = 0, j = 0; i < length(); i++)
			if (f(_a[i])) {
				asl_destroy(&_a[i]);
				n--;
			}
			else
				memcpy((char*)&_a[j++], &_a[i], sizeof(T));
		d().n = n;
		return *this;
	}

	/**
	Removes the last item in the array
	*/
	Array& removeLast()
	{
		if (length() > 0)
			remove(length() - 1);
		return *this;
	}

	Enumerator all() {return Enumerator(*this);}
	Enumerator all() const {return Enumerator(*this);}
	Enumerator slice_(int i, int j=0) {if(j==0) j=length();return Enumerator(*this, i, j);}
	Enumerator slice_(int i, int j = 0) const {if(j==0) j=length();return Enumerator(*this, i, j);}
};

/**
An alias for Array<byte>
\ingroup Containers
*/
typedef Array<byte> ByteArray;

template <class T>
Array<T>& Array<T>::reserve(int m)
{
	int s=d().s;
	if (m <= s)
		return *this;
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
		memcpy((char*)b, _a, k);
	}
	else if(s1 != s)
	{
		dup();
		int   rc = d().rc;
		ASL_RC_FREE();
		char* p = (char*) realloc( (char*)_a-sizeof(Data), s1*sizeof(T)+sizeof(Data) );
		if(!p)
			ASL_BAD_ALLOC();
		b = (T*) ( p + sizeof(Data) );
		_a = b;
		ASL_RC_INIT();
		d().rc = rc;
		d().s = s1;
		d().n = n;
		s1 = s;
	}
	if(s1 != s)
	{
		dup();
		int rc = d().rc;
		ASL_RC_FREE();
		char* p = (char*)_a - sizeof(Data);
		::free(p);
		_a = b;
		ASL_RC_INIT();
		d().rc = rc;
		d().s = s1;
		d().n = n;
	}
	return *this;
}

template <class T>
Array<T>& Array<T>::insert(int k, const T& x)
{
	Data* h = &d();
	int n = h->n;
	int s = h->s;
	if (k < 0)
		k = n;
	if (n < s) {}
	else
	{
		s++;
		if (n == 2147483647)
			ASL_BAD_ALLOC();
		dup();
		int rc = d().rc;
		ASL_RC_FREE();
		int s1 = s < 1073741823 ? 2 * s : 2147483647;
		char* p = (char*)realloc((char*)_a - sizeof(Data), s1 * sizeof(T) + sizeof(Data));
		if(!p)
			ASL_BAD_ALLOC();
		T* b = (T*) ( p + sizeof(Data) );
		_a = b;
		ASL_RC_INIT();
		h = &d();
		h->s=s1;
		h->rc = rc;
	}
	if (k < n) {
		memmove((char*)(_a + k + 1), (char*)(_a + k), (n - k) * sizeof(T));
	}
	asl_construct_copy(_a + k, x);
	h->n = n+1;
	return *this;
}

template<class T>
void Array<T>::alloc(int m)
{
	int s=max(m, 3);
	char* p = (char*) malloc( s*sizeof(T)+sizeof(Data) );
	if(!p)
		ASL_BAD_ALLOC();
	_a = (T*) ( p + sizeof(Data) );
	ASL_RC_INIT();
	d().s = s;
	d().n = m;
	d().rc=1;
	asl_construct(_a, m);
}

template<class T>
void Array<T>::free()
{
	asl_destroy(_a, d().n);
	ASL_RC_FREE();
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
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#undef ASL_RC_INIT
#undef ASL_RC_FREE

#endif
