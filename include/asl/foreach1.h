/*! \file */ 

#ifndef ASL_FOREACH_H
#define ASL_FOREACH_H

namespace asl {

struct EnumWrapper_ {
	mutable int more;
	EnumWrapper_() : more(1) {}
};

template <class C>
struct EnumWrapper : public EnumWrapper_ {
	typename C::Enumerator e;
	//const C c; // for temporaries. add only if very little performance impact
	ASL_EXPLICIT EnumWrapper(const C& a) : e(((C&)a).all()) { more = 1; }
	operator bool() const { return e; }
};

template <typename C>
inline EnumWrapper<C> newEnumerator(const C& e) { return EnumWrapper<C>(e); }

template <typename C>
inline EnumWrapper<C>& enumData(C*, const EnumWrapper_& e) { return *(EnumWrapper<C>*)&e; }

#undef foreach

template<class T>
const T* ref2nulp(const T& x) { return 0; } // avoid warning with temporary Enumerators

#define ASL_TY(x) (true?0:ref2nulp(x))

/**
\defgroup Containers Containers
@{
*/

/**
A for loop for containers resembling the `foreach` keyword of D. In each iteration, `variable` takes 
the value of *each* element in `set`. Variable can have a type declaration (equaling the type of elements
of `set`) and an even be a reference. In newer compilers just use `for(auto& x : a)`

~~~.cpp
foreach(int& x, array)
	x *= 2;              // multiplies each element of the array times 2
~~~

This will not work for temporary containers (i.e. returned by functions).
@hideinitializer
*/
#define foreach(variable, set) \
	for (const asl::EnumWrapper_& _b_ = asl::newEnumerator((set)); asl::enumData(ASL_TY(set), _b_) && _b_.more; ++asl::enumData(ASL_TY(set), _b_).e, _b_.more = 1-_b_.more) \
	for (variable = *asl::enumData(ASL_TY(set), _b_).e; _b_.more; _b_.more=0)

/**
A for loop for associative containers resembling the `foreach` keyword of D. Similar to `foreach` but 
using two variables: the first will take the value of each *key* and the second will take its associated
value. In newer compilers just use `for(auto& x : a)`, or `for(auto& [name, value] : a)` [C++17]
~~~.cpp
foreach2(String& name, float value, variables)
{
	cout << name << " has the value " value << endl;
}
~~~
@hideinitializer
*/

#define foreach2(key, variable, set) \
	for (const asl::EnumWrapper_& _b_ = asl::newEnumerator((set)); asl::enumData(ASL_TY(set), _b_) && _b_.more; ++asl::enumData(ASL_TY(set), _b_).e, _b_.more = -1-_b_.more) \
	for (variable (*asl::enumData(ASL_TY(set), _b_).e); _b_.more > 0; _b_.more-=2) \
	for (const key (~asl::enumData(ASL_TY(set), _b_).e); _b_.more; --_b_.more)

template<class T>
void quicksort(T* a, int n)
{
	if (n < 2)
		return;
	T p = a[n / 2];
	T* l = a;
	T* r = a + n - 1;
	while (l <= r) {
		while (*l < p)
			l++;
		while (p < *r)
			r--;
		if (l <= r)
			swap(*l++, *r--);
	}
	quicksort(a, int(r - a + 1));
	quicksort(l, int(a + n - l));
}

template<class T, class Less>
void quicksort(T* a, int n, const Less& less)
{
	if (n < 2)
		return;
	T p = a[n / 2];
	T* l = a;
	T* r = a + n - 1;
	while (l <= r) {
		while (less(*l, p))
			l++;
		while (less(p, *r))
			r--;
		if (l <= r)
			swap(*l++, *r--);
	}
	quicksort(a, int(r - a + 1), less);
	quicksort(l, int(a + n - l), less);
}

/**
Shuffles an array of elements in place (n items starting at a).
\deprecated Use Random::shuffle()
*/
template <typename T>
void shuffle(T* a, int n, Random& rnd = random)
{
	while (n) {
		int i = int(rnd(1.0) * n--);
		swap(a[n], a[i]);
	}
}


/**
Shuffles an array of elements in place.
\deprecated Use Random::shuffle()
*/
template <typename E>
void shuffle(E& a, Random& rnd = random)
{
	shuffle(&a[0], a.length(), rnd);
}

/**@}*/

}

#define ASL_FOREACH foreach

#endif
