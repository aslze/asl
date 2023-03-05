// Copyright(c) 1999-2023 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_SHARED_H
#define ASL_SHARED_H

#include "Pointer.h"

namespace asl {

/*
EXPERIMENTAL
Use with caution. This may change.
*/

struct SmartObject_
{
	AtomicCount rc;
	SmartObject_() {}
	virtual ~SmartObject_() {}
	virtual SmartObject_* clone() const { return new SmartObject_(*this); }
};


class ASL_API SmartObject
{
public:
	SmartObject_* _p;
protected:
	operator int() const { return 0; }
	
public:
	typedef SmartObject_ NType;
	typedef NType* Ptr;

	SmartObject(){
		_p = new SmartObject_;
	}
	SmartObject(const SmartObject& n)
	{
		_p = n._p;
		if (_p)
			++_p->rc;
	}
	void unref()
	{
		if(_p && --_p->rc == 0) {
			delete _p;
		}
	}
	SmartObject& operator=(const SmartObject& n)
	{
		unref();
		_p = n._p;
		++_p->rc;
		return *this;
	}
	~SmartObject()
	{
		unref();
	}

	SmartObject(SmartObject_* p) : _p(p)
	{
		if (_p)
			++_p->rc;
	}

	SmartObject clone() const
	{
		return SmartObject(_p->clone());
	}

	bool isnull() const
	{
		return _p == 0;
	}

	bool operator!() const
	{
		return _p == 0;
	}

	ASL_EXPLICIT operator bool() const
	{
		return _p != 0;
	}

	template<class T>
	T as()
	{
		return T(dynamic_cast<typename T::NType*>(_p));
	}
	template<class T>
	const T as() const
	{
		return T(dynamic_cast<typename T::NType*>(_p));
	}

	template<class T>
	bool is() const
	{
		return dynamic_cast<typename T::NType*>(_p) != 0;
	}

	bool is(const SmartObject& o) const
	{
		return _p == o._p;
	}

	bool operator==(const SmartObject& o) const
	{
		return _p == o._p;
	}

	bool operator!=(const SmartObject& o) const
	{
		return !is(o);
	}

	bool operator<(const SmartObject& o) const
	{
		return _p < o._p;
	}

	Ptr ptr() const { return _p; }
	Ptr ptr() { return _p; }
};

#define ASL_SMART_CLASS(C, B) class C; struct C##_;  \
	struct C##_ : public B##_

#define ASL_SMART_INIT(...) Parent(new NType(__VA_ARGS__))

#define ASL_SMART_DEF(C, B) typedef C##_ NType; typedef B Parent; \
	typedef C##_* Ptr; Ptr _() { return (Ptr)_p; } \
	const Ptr _() const { return (Ptr)_p; } C() : ASL_SMART_INIT() {}\
	C(SmartObject_* p) : Parent(p) {} \
	C clone() const { return C(_()->clone()); } \
	const Ptr ptr() const { return _(); } \
	Ptr ptr() { return _(); }

#define ASL_SMART_DECL(C, B) typedef C##_ NType; typedef B Parent; \
	typedef C##_* Ptr; Ptr _() { return (Ptr)_p; } \
	const Ptr _() const { return (Ptr)_p; } C(); \
	C(SmartObject_* p); \
	C clone() const; \
	const Ptr ptr() const { return _(); } \
	Ptr ptr() { return _(); }

#define ASL_SMART_INNER_DEF(C) virtual SmartObject_* clone() const { return new C##_(*this); }

#define ASL_SMART_INNER_DECL(C) virtual SmartObject_* clone() const;

#define ASL_SMART_INNER_IMPL(C) SmartObject_* C##_::clone() const { return new C##_(*this); } \
	C::C(SmartObject_* p) : Parent(p) {}

}
#endif
