// Copyright(c) 1999-2020 aslze
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
	const byte* p = s;
	for (int i = 0; i<n; i++)
		h = 33 * h + p[i];
	return h;
}

template<typename T>
inline int hash(T* p)
{
	return ((int)p) >> 2;
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

The contents can be iterated with a `foreach2` loop just like a Map:

~~~
foreach2(String& name, float value, constants)
{
    cout << "Contstant " << *name << " has value: " << value << endl;
}
~~~

Or with range-based for, in C++11:

~~~
for(auto& e : constants)
{
	cout << "Contstant " << *e.key << " has value: " << e.value << endl;
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
		KeyVal* next;
		KeyVal() {next=0;}
		KeyVal(const K& n): key(n) {}
		KeyVal(const K& n, const T& v): key(n), value(v) {}
		KeyVal(const KeyVal& p): key(p.key), value(p.value) {}
		void operator=(const KeyVal& p) {key=p.key; value=p.value;}
	};

	public:
	Array< KeyVal* > a;
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
		--b._rc();
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
				KeyVal* p = a[i];
				KeyVal* next;
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
				KeyVal* p = a[i];
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

		Array<KeyVal*> b((a.length() - ASL_HMAP_SKIP) * 8 + ASL_HMAP_SKIP);
		for (int i = 0; i<ASL_HMAP_SKIP; i++)
			b[i] = a[i];
		for (int i = ASL_HMAP_SKIP; i<b.length(); i++)
			b[i] = 0;
		int n = _n();

		for (int i = ASL_HMAP_SKIP; i < a.length(); i++)
		{
			if (a[i])
			{
				KeyVal* p = a[i];
				KeyVal* next;
				do {
					next = p->next;
					int bin = (hash(p->key) & (b.length() - ASL_HMAP_SKIP - 1)) + ASL_HMAP_SKIP;

					KeyVal* p2 = b[bin], *q2 = p2;
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
	creating one if the key does not exist.
	*/
	const T& operator[](const K& key) const
	{
		const_cast<HashMap*>(this)->rehash();
		int bin = binOf(key);
		KeyVal* p = a[bin], *q = p;
		while(p)
		{
			if(p->key == key)
				return p->value;
			q = p;
			p = p->next;
		}
		p = new KeyVal(key);
		p->next = 0;
		if(!q)
			a[bin] = p;
		else
			q->next = p;
		++const_cast<HashMap*>(this)->_n();
		return p->value;
	}

	T& operator[](const K& key)
	{
		rehash();
		int bin = binOf(key);
		KeyVal* p = a[bin], *q = p;
		while(p)
		{
			if(p->key == key)
				return p->value;
			q = p;
			p = p->next;
		}
		p = new KeyVal(key);
		p->next = 0;
		if(!q)
			a[bin] = p;
		else
			q->next = p;
		++_n();
		return p->value;
	}
	
	/**
	Returns the value for the given key or the value `def` if it is not found
	*/
	const T& get(const K& key, const T& def) const
	{
		return has(key)? (*this)[key] : def;
	}
	
	/**
	Removes the given key
	*/
	void remove(const K& key)
	{
		int bin = binOf(key);
		KeyVal* p = a[bin], *q = p;
		while(p)
		{
			if(p->key == key)
			{
				KeyVal* n = p->next;
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
		KeyVal* p = a[binOf(key)];
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

	struct Enumerator
	{
		typedef typename HashMap<K,T>::KeyVal KeyVal;
		typename Array<KeyVal*>::Enumerator e;
		KeyVal* p;
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
		Enumerator all() {return *this;}
	};
	Enumerator all() {return Enumerator(*this);}

	struct FEnumerator : public Enumerator
	{
		FEnumerator() {}
		FEnumerator(const HashMap& m) : Enumerator(m) {}
		typename Enumerator::KeyVal& operator*() { return *this->p; }
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
};

}
#endif
