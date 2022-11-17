// Copyright(c) 1999-2022 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_Pointer_H
#define ASL_Pointer_H

#include "atomic.h"

namespace asl {

template <class T>
class Shared
{
public:
	Shared()
	{
		_p = 0;
	}
	Shared(const Shared<T>& p) : _p(p._p)
	{
		ref();
	}
	Shared(T* p)
	{
		_p = new Core(p);
	}
	~Shared()
	{
		unref();
	}
	Shared& operator=(const Shared& r)
	{
		if (_p != r._p)
		{
			Core* t = _p;
			_p = r._p;
			ref();
			if (t)
				t->unref();
		}
		return *this;
	}
	Shared& operator=(T* r)
	{
		{
			Core* t = _p;
			_p = new Core(r);
			if (t)
				t->unref();
		}
		return *this;
	}
	T& operator*() const
	{
		return *_p->p;
	}
	T* operator->() const
	{
		return _p->p;
	}
	operator T*() const
	{
		return _p->p;
	}
	operator void*() const
	{
		return _p->p;
	}
	operator bool() const
	{
		return _p != 0 && _p->p != 0;
	}
	bool operator!() const
	{
		return _p == 0 || _p->p == 0;
	}
	bool operator==(T* r) const
	{
		return (_p && _p->p == r);
	}
	bool operator!=(T* r) const
	{
		return (_p == 0 || _p->p != r);
	}
	template <typename R>
	bool operator==(R r) const
	{
		return (_p == (const T*)(r) );
	}
	template <typename R>
	bool operator!=(R r) const
	{
		return (_p != (const T*)(r) );
	}
	bool operator<(const Shared &r) const
	{
		return _p < r._p;
	}
private:

	struct Core {
		T* p;
		AtomicCount rc;
		Core() : p(0), rc(1) {}
		Core(T* r) : p(r), rc(1) {}
		void ref(){
			++rc;
		}
		void unref(){
			if(--rc<=0){
				delete p;
				p=0;
				delete this;
			}
		}
	};

	Core* _p;

	void ref()
	{
		if (_p)
			_p->ref();
	}

	void unref()
	{
		if (_p)
			_p->unref();
	}
};

template <class T>
class Pointer
{
	Pointer(const Pointer& p): _p(0)
	{
	}
	Pointer& operator=(const Pointer& r)
	{
		_p = 0;
		return *this;
	}
public:
	Pointer(): _p(0)
	{
	}
	Pointer(T* p): _p(p)
	{
	}
	~Pointer()
	{
		delete _p;
	}
	Pointer& operator=(T* r)
	{
		delete _p;
		_p = r;
		return *this;
	}
	T& operator*() const
	{
		return *_p;
	}
	T* operator->() const
	{
		return _p;
	}
	operator T*() const
	{
		return _p;
	}
	operator void*() const
	{
		return _p;
	}
	operator bool() const
	{
		return _p != 0;
	}
	bool operator!() const
	{
		return _p == 0;
	}
	template <typename R>
	bool operator==(R r) const
	{
		return (_p == (const T*)(r) );
	}
	template <typename R>
	bool operator!=(R r) const
	{
		return (_p != (const T*)(r) );
	}
	bool operator<(const Pointer &r) const
	{
		return _p < r._p;
	}
private:

	T* _p;
};

template <class T>
class Pointer<T[]>
{
	Pointer(const Pointer& p): _p(0)
	{
	}
	Pointer& operator=(const Pointer& r)
	{
		_p = 0;
		return *this;
	}
public:
	Pointer(): _p(0)
	{
	}
	Pointer(T* p): _p(p)
	{
	}
	~Pointer()
	{
		delete [] _p;
	}
	Pointer& operator=(T* r)
	{
		delete [] _p;
		_p = r;
		return *this;
	}
	T& operator[](int i)
	{
		return _p[i];
	}
	const T& operator[](int i) const
	{
		return _p[i];
	}
	operator bool() const
	{
		return _p != 0;
	}
	operator T*() const
	{
		return _p;
	}
	operator void*() const
	{
		return _p;
	}
	bool operator!() const
	{
		return _p == 0;
	}
	template <typename R>
	bool operator==(R r) const
	{
		return (_p == (const T*)(r) );
	}
	template <typename R>
	bool operator!=(R r) const
	{
		return (_p != (const T*)(r) );
	}
	bool operator<(const Pointer &r) const
	{
		return _p < r._p;
	}
private:

	T* _p;
};

template <class T>
struct StaticSpace
{
	void construct() const
	{
		asl_construct((T*)_space);
	}
	void construct(const T& x) const
	{
		asl_construct_copy((T*)_space, x);
	}
	void destroy() const
	{
		asl_destroy((T*)_space);
	}

	T& operator*() { return *(T*)(_space); }
	T* operator->() { return (T*)(_space); }
	const T& operator*() const { return *(const T*)(_space); }
	const T* operator->() const { return (const T*)(_space); }
private:
	byte _space[sizeof(T)];
};

}
#endif
