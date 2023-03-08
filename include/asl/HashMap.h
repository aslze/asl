// Copyright(c) 1999-2022 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_HASHMAP_H
#define ASL_HASHMAP_H

#include <asl/Array.h>
#include <asl/String.h>

namespace asl {

inline int hash(int x)
{
	return x;
}

inline int hash(const String& s)
{
	int h = 0, n = s.length();
	const char* p = s;
	for(int i=0; i<n; i++)
		h = 33*h + p[i];
	return h;
}

inline int hash(const Array<byte>& s)
{
	int h = 0, n = s.length();
	const byte* p = s.ptr();
	for (int i = 0; i<n; i++)
		h = 33 * h + p[i];
	return h;
}

template<typename T>
inline int hash(T* p)
{
	return ((int)(size_t(p) & 0xffffffff)) >> 2;
}

template<typename T>
inline int hash(const T& x)
{
	int h = 0;
	const byte* p = (const byte*)&x;
	for(int i=0; i<sizeof(x); i++)
		h = 33*h + p[i];
	return h;
}

#ifdef ASL_FILE_H
inline int hash(const File& f)
{
	return hash(f.path());
}
#endif

#define ASL_HMAP_SKIP (2 + (sizeof(AtomicCount)-1)/sizeof(void*))

inline int nextPoT(int n)
{
	n--;
	n |= n >> 1;
	n |= n >> 2;
	n |= n >> 4;
	n |= n >> 8;
	n |= n >> 16;
	return n + 1;
}

/**
This class implements a hash map, an unordered map of keys to values. It is similar to
class Map but elements will not keep a defined order. There must be a global function `hash(const K&)`
for the key type `K` (there is a default hash function that may fit). Inserting and finding elements is
usually faster than in a Map. The class has reference counting as all containers.

~~~
HashMap<String, float> constants;
constants["pi"] = 3.1415927;

float a = constants["pi"] * sqr(radius);

if(constants.has("pi"))
{...}
~~~

The contents can be iterated with range-based for in C++11:

~~~
for(auto& e : constants)
{
    cout << "Contstant " << *e.key << " has value: " << e.value << endl;
}
~~~

Or with the `foreach2` macro loop in older compilers:

~~~
foreach2(String& name, float value, constants)
{
    cout << "Contstant " << *name << " has value: " << value << endl;
}
~~~

\ingroup Containers
*/
template<class K, class T>
class HashMap
{
protected:
	struct KeyVal
	{
		K key;
		T value;
		KeyVal(const K& k): key(k) {}
		KeyVal(const K& k, const T& v): key(k), value(v) {}
	};

	struct KeyValN : public KeyVal
	{
		KeyValN* next;
		KeyValN(): next(0) {}
		KeyValN(const K& k): KeyVal(k), next(0) {}
		KeyValN(const K& k, const T& v) : KeyVal(k, v), next(0) {}
	};

public:
	Array<KeyValN*> a;
	int& _n() { return *(int*)&a[0]; }
	int _n() const { return *(const int*)&a[0]; }
	AtomicCount& _rc() { return *(AtomicCount*)&a[1]; }
public:
	HashMap(): a(256 + ASL_HMAP_SKIP)
	{
		for(int i=0; i<a.length(); i++)
			a[i] = 0;
		asl_construct((AtomicCount*)&a[1]);
		_rc() = 1;
	}

	HashMap(int n)
	{
		a.resize(nextPoT(n)+ASL_HMAP_SKIP);
		for(int i=0; i<a.length(); i++)
			a[i] = 0;
		asl_construct((AtomicCount*)&a[1]);
		_rc() = 1;
	}

	HashMap(const HashMap& b) : a(b.a)
	{
		++_rc();
	}
	
	HashMap& dup()
	{
		HashMap b(a.length() - ASL_HMAP_SKIP);
		foreach2(K& k, T& v, *this)
			b[k] = v;
		swap(a, b.a);
		return *this;
	}
	
	/**
	Returns an independent copy of this map
	*/
	HashMap clone() const
	{
		HashMap b(*this);
		return b.dup();
	}

	int binOf(const K& key) const
	{
		return (hash(key) & (a.length() - ASL_HMAP_SKIP - 1)) + ASL_HMAP_SKIP;
	}

	void operator=(const HashMap& b)
	{
		if (--_rc() == 0) {
			clear();
			asl_destroy((AtomicCount*)&a[1]);
		}
		a = b.a;
		++_rc();
	}

	~HashMap()
	{
		if(--_rc() == 0) {
			clear();
			asl_destroy((AtomicCount*)&a[1]);
		}
	}

	/**
	Clears the map removing all elements.
	*/
	void clear()
	{
		for(int i=ASL_HMAP_SKIP; i<a.length(); i++)
		{
			if(a[i])
			{
				KeyValN* p = a[i];
				KeyValN* next;
				do {
					next = p->next;
					delete p;
					p = next;
				}
				while(p);
			}
			a[i] = 0;
		}
		_n() = 0;
	}

	/*
	Computes a fill factor that measures how full the hash map buckets are.
	*/
	float fillFactor() const
	{
		float y = 0;
		for(int i=ASL_HMAP_SKIP; i<a.length(); i++)
			if(a[i] != 0)
				y++;
		return y/a.length();
	}

#ifdef ASL_HMAP_STATS
	Map<int,int> stats() const
	{
		Map<int,int> m;
		for(int i=ASL_HMAP_SKIP; i<a.length(); i++)
		{
			if(a[i] != 0)
			{
				KeyValN* p = a[i];
				int count = 1;
				while (p = p->next)
					count++;
				if(!m.has(count))
					m[count]=1;
				else
					m[count]++;
			}
		}
		return m;
	}
#endif
	void rehash()
	{
		if (_n() < a.length() * 7 / 8 || a.length() > 280000)
			return;

		Array<KeyValN*> b((a.length() - ASL_HMAP_SKIP) * 8 + ASL_HMAP_SKIP);
		for (int i = 0; i<ASL_HMAP_SKIP; i++)
			b[i] = a[i];
		for (int i = ASL_HMAP_SKIP; i<b.length(); i++)
			b[i] = 0;
		int n = _n();

		for (int i = ASL_HMAP_SKIP; i < a.length(); i++)
		{
			if (a[i])
			{
				KeyValN* p = a[i];
				KeyValN* next;
				do {
					next = p->next;
					int bin = (hash(p->key) & (b.length() - ASL_HMAP_SKIP - 1)) + ASL_HMAP_SKIP;

					KeyValN* p2 = b[bin], *q2 = p2;
					while (p2) {
						q2 = p2;
						p2 = p2->next;
					}
					p->next = 0;
					if (!q2)
						b[bin] = p;
					else
						q2->next = p;
					p = next;
				} while (p);
			}
		}
		a = b;
//		_rc() = rc;
		_n() = n;
	}

	/**
	Returns a reference to the value associated to the given key,
	the key has to exist
	*/
	const T& operator[](const K& key) const
	{
		const T* p = find(key);
		if (p)
			return *p;
		else
		{
			static const T def = T();
			return def;
		}
	}

	/**
	Returns a reference to the value associated to the given key,
	creating one if the key does not exist.
	*/
	T& operator[](const K& key)
	{
		rehash();
		int bin = binOf(key);
		KeyValN* p = a[bin], *q = p;
		while(p)
		{
			if(p->key == key)
				return p->value;
			q = p;
			p = p->next;
		}
		p = new KeyValN(key);
		p->next = 0;
		if(!q)
			a[bin] = p;
		else
			q->next = p;
		++_n();
		return p->value;
	}
	
	/**
	Returns a pointer to the element with key `key` or a null pointer if it is not found
	*/
	T* find(const K& key)
	{
		KeyValN* p = a[binOf(key)];
		while (p)
		{
			if (p->key == key)
				return &p->value;
			p = p->next;
		}
		return NULL;
	}

	const T* find(const K& key) const
	{
		return const_cast<HashMap*>(this)->find(key);
	}

	/**
	Returns the value for the given key or the value `def` if it is not found
	*/
	const T& get(const K& key, const T& def) const
	{
		const T* p = find(key);
		return p ? *p : def;
	}

	HashMap& set(const K& key, const T& value)
	{
		(*this)[key] = value;
		return *this;
	}
	
	/**
	Removes the given key
	*/
	void remove(const K& key)
	{
		int bin = binOf(key);
		KeyValN* p = a[bin], *q = p;
		while(p)
		{
			if(p->key == key)
			{
				KeyValN* n = p->next;
				delete p;
				if(q!=p)
					q->next = n;
				else
					a[bin] = 0;
				--_n();
				return;
			}
			q = p;
			p = p->next;
		}
	}
	/**
	Checks if the given key exists in the map
	*/
	bool has(const K& key) const
	{
		KeyValN* p = a[binOf(key)];
		while(p)
		{
			if(p->key == key)
				return true;
			p = p->next;
		}
		return false;
	}
	/**
	Returns the number of elements in the map
	*/
	int length() const
	{
		return _n();
	}

	/**
	Returns true if both maps are equal (equal keys and values)
	*/
	bool operator==(const HashMap& b) const
	{
		if (length() != b.length())
			return false;
		Enumerator e1(this->all()), e2(b.all());
		for (; e1; ++e1, ++e2)
			if (~e1 != ~e2 || *e1 != *e2) return false;
		return true;
	}

	bool operator!=(const HashMap& b) const
	{
		return !(*this == b);
	}

	struct Enumerator
	{
		typedef typename HashMap<K,T>::KeyValN KeyValN;
		typename Array<KeyValN*>::Enumerator e;
		KeyValN* p;
		///Enumerator() {}
		Enumerator(const HashMap& m): e(m.a)
		{
			for(int i=0; i<ASL_HMAP_SKIP; ++i)
				++e;
			p = *e;
			while(p == 0 && e)
			{
				++e;
				if(e)
					p = *e;
			}
		}
		void operator++()
		{
			p = p->next;
			while(p == 0 && e)
			{
				++e;
				if(e)
					p = *e;
			}

		}
		T& operator*() {return p->value;}
		T* operator->() {return &(p->value);}
		const K& operator~() {return p->key;}
		operator bool() const {return p!=0 || e;}
		bool operator!=(const Enumerator& e) const { return (bool)*this; }
		Enumerator all() const { return Enumerator(*(HashMap*)this); }
	};
	
	Enumerator all() const { return Enumerator(*this); }

	struct FEnumerator : public Enumerator
	{
		FEnumerator() {}
		FEnumerator(const HashMap& m) : Enumerator(m) {}
		typename HashMap<K, T>::KeyVal& operator*() { return *(this->p); }
	};

	FEnumerator _all() const { return FEnumerator(*this); }
};


template<class K, class T>
typename HashMap<K, T>::FEnumerator begin(const HashMap<K, T>& a)
{
	return a._all();
}

template<class K, class T>
typename HashMap<K, T>::FEnumerator end(const HashMap<K, T>& a)
{
	return a._all();
}


template <class T>
class HashDic : public HashMap<String, T>
{
public:
	HashDic() {}
	HashDic(int n): HashMap<String,T>(n) {}
	HashDic(const HashDic& b): HashMap<String,T>(b)
	{
	}
	HashDic clone() const
	{
		HashDic b(*this);
		b.dup();
		return b;
	}
};

}
#endif
