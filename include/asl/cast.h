// Copyright(c) 1999-2020 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_CAST_H
#define ASL_CAST_H

namespace asl {

template<class T>
struct DummyType {};

template<class K, class T>
K to(const T& x, DummyType<K> _)
{
	return /*(K)*/ x;
}

template<class K, class T>
K to(const T& x) { return to(x, DummyType<K>()); }

template<class To, class From>
struct Castable {};

#ifndef ASL_NO_CAST

template <class T>
struct Caster
{
	const T& x;
	Caster(const T& x): x(x) {}
	template <class K, typename U = typename Castable<K,T>::is>
	operator K() const { return to<K,T>(x); }
};

/**
Converts the argument to the type needed, if a conversion is defined
*/
template <class T>
Caster<T> cast(const T& x) { return Caster<T>(x); }

#define ASL_DEF_AUTOCAST(Class) \
template <class T1, typename U = typename Castable<T1, Class>::is> \
operator T1() const { return to<T1>(*this); } \
template <class T1, typename U = typename Castable<Class, T1>::is> \
Class(const T1& x) { asl_construct_copy(this, to<Class>(x)); }

#else
#define ASL_DEF_CASTABLE(Class)
#endif


}

#endif
