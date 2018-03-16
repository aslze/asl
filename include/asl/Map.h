// Copyright(c) 1999-2018 ASL author
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_MAP_H
#define ASL_MAP_H

#include <asl/Array.h>
#include <asl/String.h>

namespace asl {

template <class T>
inline int compare(const T& a, const T& b) { return (a<b)? -1 : (a == b) ? 0 : 1; }

inline int compare(const String& a, const char* b) {return a.compare(b);}
inline int compare(const String& a, const String& b) {return a.compare(b);}

//template <class T>
//inline int compare(const T* a, const T* b) {int c=a-b; return (c<0)? -1: (c>0)? 1: 0;}

/**
\defgroup Containers Containers
@{
*/

/**
An associative container linking keys of type `K` with values of type `T`.
By default, keys and values are Strings.

~~~
Map<char, String> morse;
morse['A'] = ".-";
morse['O'] = "...";
morse['S'] = "---";

String sos = morse['S'] + morse['O'] + morse['S'];
~~~

Maps can be initialized with short hand syntax (this is useful in Http::get() to specify HTTP request headers):

~~~
Map<> headers = Map<>("Content-Type", "text/html")("Content-Lenth", length);
~~~

And iterated with the `foreach2` macro:

~~~
foreach2(auto& name, auto& value, headers);
{
	request << name << ": " << value << "\r\n";
}
~~~
*/

template <class K=String, class T=String>
class Map
{
public:
	struct KeyVal
	{
		K key;
		T value;
		KeyVal() {}
		KeyVal(const K& n): key(n) {}
		KeyVal(const K& n, const T& v): key(n), value(v) {}
		KeyVal(const KeyVal& p): key(p.key), value(p.value) {}
		//KeyVal(KeyVal&& p): key(p.key), value(p.value) {}
		void operator=(const KeyVal& p) {key=p.key; value=p.value;}
	};
protected:
	Array<KeyVal> a;
	template<class K2, class T2>
	friend class Map;
	template<class T2>
	friend class Dic;

	T& operator()(int i) {return a[i].value;}
	const T& operator()(int i) const {return a[i].value;}
	const K& names(int i) const {return a[i].key;}
	int indexOf(const K& key) const;

public:
	Map() {}
	~Map() {}
	/** Constructs a Map from a Map of different key or value types. K2 and T2 must be convertible to K and T. */
	template<class K2, class T2>
	Map(const Map<K2,T2>& b)
	{
		foreach2(K2& k, const T2& v, b)
		{
			T _v = v;
			K _k = k;
			set(_k, _v);
		}
	}
	Map(const Map& b): a(b.a) {}
#ifdef ASL_HAVE_MOVE
	Map(Map&& b) : a(b.a)
	{
	}
	void operator=(Map&& b)
	{
		a = b.a;
	}
#endif
	Map(const K& k, const T& v)
	{
		set(k, v);
	}
#ifdef ASL_HAVE_INITLIST
	Map(std::initializer_list< KeyVal > b)
	{
		for (const KeyVal* p = b.begin(); p != b.end(); p++)
			set(p->key, p->value);
	}
#endif
	/** Returns the number of elements in this map */
	int length() const
	{
		return a.length();
	}
	/** Removes all elements */
	void clear()
	{
		a.clear();
	}
	/** Frees all values (with `delete`, so `T` must be a pointer type) and clears the dictionary. */
	void destroy()
	{
		for(int i=0; i<length(); i++)
			delete a[i].value;
		clear();
	}

	void reserve(int n)
	{
		a.reserve(n);
	}

	/** Detaches this Map from other ones possibly sharing it */
	Map& dup()
	{
		a.dup();
		return *this;
	}
	/** Returns an independent copy of this map */
	Map clone() const
	{
		Map b(*this);
		return b.dup();
	}
	void operator=(const Map& b) {a=b.a;}

	bool operator==(const Map& b) const
	{
		if (length() != b.length())
			return false;
		for(int i = 0; i<a.length(); i++)
			if (a[i].key != b.a[i].key || a[i].value != b.a[i].value)
				return false;
		return true;
	}

	bool operator!=(const Map&b) const { return !(*this == b); }

	/** Returns true if an element with key `key` exists */
	bool has(const K& key) const
	{
		return indexOf(key)>=0;
	}
	/**
	Returns a pointer to the element with key `key` or a null pointer if it is not found
	*/
	const T* find(const K& key) const
	{
		int i = indexOf(key);
		return (i >= 0) ? &a[i].value : NULL;
	}

	T* find(const K& key)
	{
		int i = indexOf(key);
		return (i >= 0) ? &a[i].value : NULL;
	}

	/** Returns an array containing all keys of this map */
	Array<K> keys() const
	{
		Array<K> k(a.length());
		for(int i=0; i<a.length(); i++)
			k[i] = a[i].key;
		return k;
	}
	/** Returns a reference to the element with key key */
	T& operator[](const K& key) const {return (*(Map*)this)[key];}
	T& operator[](const K& key);
	
	/** Returns the element with key `key` or the value `def` if key is not found */
	const T& get(const K& key, const T& def) const
	{
		const T* p = find(key);
		return p ? *p : def;
	}

	/**
	Adds an element with the given key and value, useful for the short hand initializer style shown in the class overview.
	*/
	Map& operator()(const K& key, const T& value)
	{
		return set(key, value);
	}

	Map& set(const K& key, const T& value)
	{
		int i=indexOf(key);
		if(i >= 0)
		{
			a[i].value = value;
			return *this;
		}
		a.insert(-i-1, KeyVal(key, value));
		return *this;
	}
	/** Removes the element named key */
	bool remove(const K& key)
	{
		int i = indexOf(key);
		if(i >= 0)
		{
			a.remove(i);
			return true;
		}
		else
			return false;
	}
	/** Adds all elements from dictionary d to this */
	void add(const Map& d)
	{
		for(int i=0; i<d.length(); i++)
		{
			(*this)[d[i].key] = d[i].value;
		}
	}

	struct Enumerator
	{
		Map<K,T>* d;
		int i;
		Enumerator(){}
		Enumerator(Map& _d): d(&_d), i(0) {}
		//Enumerator(const Enumerator& e) : d(e.d), i(e.i) {}
		//void operator=(const Enumerator& e) {memcpy(this, &e, sizeof(e));}
		void operator++() {i++;}
		T& operator*() {return (*d)(i);}
		T* operator->() {return &((*d)(i));}
		const K& operator~() const {return d->names(i);}
		operator bool() const {return i < d->length();}
	};
	/** Returns an enumerator for this map */
	Enumerator all() {return Enumerator(*this);}
	Enumerator all() const { return Enumerator(*(Map*)this); }

	/**
	Joins the contents of a Dic<> into a string, using `s1` as element
	separator (often a comma) and `s2` as key-value separator (usually an '=').
	*/
	String join(const String& s1, const String& s2) const
	{
		int i = 0, n = length();
		String out;
		foreach2(String& k, const String& v, *this)
		{
			out << k;
			out << s2;
			out << v;
			if (i++ < n - 1)
				out << s1;
		}
		return out;
	}
};

template <class K, class T>
int Map<K,T>::indexOf(const K& key) const
{
	int min=0, max=(length()-1), mid=max, cmp;
	if(length()==0)
		return -1;
	do
	{
		cmp = compare(a[mid].key, key);
		if(cmp < 0) min = mid;
		else if(cmp > 0) max = mid;
		else return mid;
		mid = (max+min) >> 1;
	}
	while(max-min > 1);
	if(min == max)
		return (cmp < 0)? -max-2 : -1;
	cmp = compare(a[min].key, key);
	if(cmp == 0) return min;
	else if(cmp < 0) return -max-1;
	else return -min-1;
}

template<class K, class T>
T& Map<K,T>::operator[](const K& key)
{
	int i = indexOf(key);
	if(i >= 0)
		return a[i].value;
	else
	{
		a.insert(-i-1, KeyVal(key));
		return a[-i-1].value;
	}
}

/**
Dic is a particular case of Map in which keys are strings. This is a very common case.
Dic only needs one template argument: the type of the values
*/

template <class T=String>
class Dic : public Map<String, T>
{
	typedef typename Map<String,T>::KeyVal KeyVal;
public:
	Dic() {}
	template<class K2, class T2>
	Dic(const Map<K2,T2>& b)
	{
		foreach2(K2& k, const T2& v, b)
			(*this)[k] = v;
	}
	template<class T2>
	Dic(const Dic<T2>& b)
	{
		(*(Map<String,T>*)this)=b;
	}
	Dic(const String& k, const T& v)
	{
		(*this)[k] = v;
	}
#ifdef ASL_HAVE_INITLIST
	struct KV { const char* key; const T& value; };
	Dic(std::initializer_list< KeyVal > b) :
		Map < String, T>(b) {}
	/*{
		for (int i = 0; i < (int)b.size(); i++)
		{
			const KV& kv = b.begin()[i];
			(*this)[kv.key] = kv.value;
		}
	}*/
	void operator=(std::initializer_list< KV > b)
	{
		this->clear();
		for (int i = 0; i < (int)b.size(); i++)
		{
			const KV& kv = b.begin()[i];
			(*this)[kv.key] = kv.value;
		}
	}
#endif

	Dic& dup() {this->a.dup(); return *this;}
	Dic clone() const
	{
		Dic b(*this);
		return b.dup();
	}
};



/**
Parses a string and creates a `Dic` using `sep1` as pair separator, and `sep2` as key/value separator.
It is the opposite of `join()`.
*/
inline Dic<String> split(const String& s, const String& sep1, const String& sep2)
{
	Dic<String> dic;
	Array<String> pairs = s.split(sep1);
	for(int i=0; i<pairs.length(); i++)
	{
		int j = pairs[i].indexOf(sep2);
		if(j>0)
			dic[pairs[i].substring(0, j)] = pairs[i].substring(j+sep2.length());
	}
	return dic;
}

/**@}*/

}
#endif
