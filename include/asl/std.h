// Copyright(c) 1999-2020 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_STDCAST_H
#define ASL_STDCAST_H

#include <asl/String.h>
#include <string>
#include <vector>

namespace asl {

template<>
struct Castable<std::string, asl::String> { typedef int is; };

template<>
struct Castable<asl::String, std::string> { typedef int is; };

template<class T, class K>
struct Castable<asl::Array<T>, std::vector<K> > { typedef int is; };

template<class T, class K>
struct Castable<std::vector<K>, asl::Array<T> > { typedef int is; };

template<>
inline std::string to(const asl::String& x)
{
	return std::string(*x, x.length());
}

template<>
inline asl::String to(const std::string& x, DummyType<asl::String>)
{
	return asl::String(x.c_str(), (int)x.size());
}

template<typename K, typename T>
asl::Array<K> to(const std::vector<T>& a, DummyType<asl::Array<K> > _)
{
	asl::Array<K> b((int)a.size());
	for (int i = 0; i < b.length(); i++)
		b[i] = to<K>(a[i]);
	return b;
}

template<typename K, typename T>
std::vector<K> to(const asl::Array<T>& a, DummyType<std::vector<K> > _)
{
	std::vector<K> b(a.length());
	for (int i = 0; i < a.length(); i++)
		b[i] = to<K>(a[i]);
	return b;
}

}

#endif
