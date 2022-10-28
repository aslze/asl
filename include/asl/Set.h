// Copyright(c) 1999-2022 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_SET_H
#define ASL_SET_H

#define MAP HashMap

#include <asl/HashMap.h>

namespace asl {

/**
A set is a container of unique elements in any order. The type of items must have a corresponding global hash() function.

~~~
Set<int> numbers;

numbers << 3 << -4 << 1 << 3; // number 3 (repeated) will only appear once in the set

if(numbers.contains(10)) -> false (10 is not in the set)

foreach(int x, numbers)
{
	cout << x << endl;
}
~~~

You can perform operations between sets:

~~~
Set<int> even = {0, 2, 4, 6, 8, 10};
Set<int> powers = {1, 2, 4, 8, 16, 32};

Set<int> evenOrPower = even + powers; // -> {0, 1, 2, 4, 6, 8, 10, 16, 32}
Set<int> evenNotPower = even - powers; // -> {0, 6, 10}

if(even.contains(powers)) // false, not all powers numbers are in the even set.

if(even.containsAny(powers)) // true, even numbers have (some) numbers from the powers set.
~~~
\ingroup Containers
*/
template <class T>
class Set: public MAP<T,int>
{
public:
	Set(int size): MAP<T,int>(size) {}
	Set() {}
	Set(const Array<T>& a)
	{
		foreach(const T& x, a)
			(*this)[x] = 1;
	}

#ifdef ASL_HAVE_INITLIST
	Set(std::initializer_list<T> b)
	{
		int m = (int)b.size();
		const T* p = b.begin();
		for (int i = 0; i<m; i++)
			*this << p[i];
	}
#endif
	/**
	Returns the set items as an array
	*/
	Array<T> array() const
	{
		Array<T> a;
		a.reserve(this->length());
		foreach(const T& x, *this)
			a << x;
		return a;
	}
	operator Array<T>() const
	{
		return array();
	}

	/**
	Adds an item to the set
	*/
	Set& operator<<(const T& x)
	{
		(*this)[x] = 1;
		return *this;
	}
	Set& operator>>(T& x)
	{
		this->remove(x);
		return *this;
	}
	/**
	Adds all items in set s into this set
	*/
	Set& operator<<(const Set& s)
	{
		foreach(const T& x, s)
			(*this)[x] = 1;
		return *this;
	}
	/**
	Returns true if both sets have the same items
	*/
	bool operator==(const Set& s) const
	{
		if(this->length() != s.length())
			return false;
		Enumerator e1 = this->all(), e2 = s.all();
		for(; e1; ++e1, ++e2)
			if(*e1 != *e2) return false;
		return true;
	}
	/**
	Returns true if both sets don't have the same items
	*/
	bool operator!=(const Set& s) const {return !(s==*this);}
	/**
	Checks if this set contains a given item
	*/
	bool contains(const T& x) const
	{
		return this->has(x);
	}
	/**
	Checks if this set contains all items from another set
	*/
	bool contains(const Set& s) const
	{
		foreach(const T& x, s)
			if(!contains(x)) return false;
		return true;
	}
	/**
	Checks if this set contains at least one item from another set
	*/
	bool containsAny(const Set& s) const
	{
		foreach(const T& x, s)
			if(contains(x)) return true;
		return false;
	}
	/**
	Returns the items from this set which do not belong to another set
	*/
	Set notIn(const Set& s) const
	{
		Set b;
		const Set& a=*this;
		foreach(const T& x, a) if(!s.contains(x)) b << x;
		return b;
	}
	/**
	Returns the items from this set which also belong to another set
	*/
	Set in(const Set& s) const
	{
		Set b;
		const Set& a=*this;
		foreach(const T& x, a) if(s.contains(x)) b << x;
		return b;
	}
	/**
	Returns the items from this set which also belong to another set (intersection)
	*/
	Set operator&(const Set& s) const {return in(s);}
	/**
	Returns the items from this set which do not belong to another set (subtraction)
	*/
	Set operator-(const Set& s) const {return notIn(s);}
	/**
	Returns all items from this and another set (union)
	*/
	Set operator+(const Set& s) const
	{
		Set b;
		b << *this << s;
		return b;
	}
	bool empty() const {return this->length()==0;}
	
	struct Enumerator : public MAP<T,int>::Enumerator
	{
		Enumerator(){}
		Enumerator(Set& s) : MAP<T, int>::Enumerator(s) {}
		Enumerator(const Set& s) : MAP<T, int>::Enumerator((Set&)s) {}
		T& operator*() { return (T&)~(*this); }
		T* operator->() { return &(~(*this)); }
	};

	Enumerator all() { return Enumerator(*this); }
	Enumerator all() const { return Enumerator(*this); }
};
#undef MAP


#ifdef ASL_HAVE_RANGEFOR

template<class T>
typename Set<T>::Enumerator begin(const Set<T>& a)
{
	return a.all();
}

template<class T>
typename Set<T>::Enumerator end(const Set<T>& a)
{
	return a.all();
}

template<class T>
Array<T> array(const Set<T>& s)
{
	return s.array();
}

#endif

}
#endif
