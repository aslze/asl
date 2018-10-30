// Copyright(c) 1999-2018 ASL author
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_VAR_H
#define ASL_VAR_H

#include <asl/String.h>
#include <asl/Array.h>
#include <asl/Map.h>
#include <asl/Pointer.h>
//#include <asl/HashMap.h>
//#define HDic HashDic
#define HDic Dic
#define ASL_VAR_STATIC
#define ASL_XDLCLASS "_"

namespace asl {

#ifndef ASL_VAR_STATIC
#define NEW_ARRAY(a) (a) = new Array<Var>
#define NEW_ARRAYC(a, x) (a) = new Array<Var>(x)
#define DEL_ARRAY(a) delete (a)
#define NEW_DIC(d) (d) = new HDic<Var>
#define NEW_DICC(d, x) (d) = new HDic<Var>(x)
#define DEL_DIC(d) delete (d)
#else
#define NEW_ARRAY(a) (a).construct()
#define NEW_ARRAYC(a, x) (a).construct(x)
#define DEL_ARRAY(a) (a).destroy()
#define NEW_DIC(d) (d).construct()
#define NEW_DICC(d, x) (d).construct(x)
#define DEL_DIC(d) (d).destroy()
#define NEW_STRING(s) (s).construct()
#define NEW_STRINGC(s, x) (s).construct(x)
#define DEL_STRING(s) (s).destroy()
#endif

#define VAR_SSPACE 8

/**
A Var is a type that can hold a value of one of several types, similarly to a `var` in JavaScript.
Upon assignment to a number, string, bool, array or dic, it will take its value and type.
If a Var is not initialized and operator `var["string"]` is used, it will be converted to a Dic and the
given element returned. If operator `var[int]` is used on it, it will be converted to an array and the
indexed element returned after an automatic resize to avoid overflow.

A Var can be constructed from a variable of type int, float, double, bool, String, Array or Dic, and will
take its value and type. A default-constructed Var has type NONE.

~~~
Var z;            // z.type() = NONE
Var a = 3;        // a.type() = INT
Var b = 3.5;      // b.type() = NUMBER
Var s = "x";      // s.type() = STRING
Var t = true;     // t.type() = BOOL
Var n = Var::NUL; // n.type() = NUL
~~~

An **array** can be constructed from an existing Array<T> or by specifying its elements:

~~~
Var pair;
pair.resize(2);  // pair.type() = ARRAY,  pair.length() = 2
pair[0] = 1;
pair[1] = -2;
~~~

Arrays can also be created in one statement with one of these **pseudo-literal syntaxes**:

~~~
Var coords = array<Var>(10, 25, -1);     // up to 6 elements
Var indices = (Var(), 1, 3, 0, 2, -1);   // any number of elements (but is slower)
Var numbers = Var::array({1, 3, 9, -2}); // on compilers supporting C++11 initializer lists
~~~

An **object** can be constructed from an existing Dic<T> or by adding elements to a Var with `var["key"]`:

~~~
Var person;
person["name"] = "John";  // person.type() = OBJ
person["age"] = 21;
~~~

Alternatively, objects can be created in one statement with a **pseudo-literal syntax**:

~~~
Var particle = Var("name", "particle1")
                  ("x", 15.0)
                  ("y", -1.25)
                  ("visible", true)
                  ("color", Var::array({255, 0, 255}));
~~~

Any combination of the above can be used to create complex structured vars.

A var can be checked for its type with the `is()` function or whether it contains a given key (if it is a Map),
or if it contains a given item (if it is an Array):

~~~
if(a.is(Var::NUMBER)) {...} // INTs are NUMBERs too!
if(particle.has("name")) {...} // particle is an OBJ and has a "name" property
if(particle.has("visible", Var::BOOL)) {...} // point is an OBJ and has a bool property named "visible"
if(indices.contains(3)) {...} // indices is an array and contains number 3
~~~

__Iteration__

When a Var contains an array or an object, its elements or properties can be iterated.

__Arrays__ can be iterated with `foreach` loops (or `for(item : array)` in C++11), or using array
indexing and the `length()` method:

~~~
Var numbers = array<Var>(10.1, 25, -1, 3.4);

foreach(Var& x, numbers)
{
    do_something_with(x);
}

for(auto& x : numbers)  // C++11
{
	do_something_with(x);
}

for(int i=0; i < numbers.length(); i++)
{
    do_something_with(numbers[i]);
}
~~~

__Object__ properties (keys and values) can be iterated with a `foreach2` loop, or with range-based for in C++11/17 (but
in this case don't forget to add `.object()`):

~~~
foreach2(String& key, Var& value, particle)   // the particle from above
{
    printf("%s : %s\n", *key, *value.toString());
}

for(auto& e : particle.object())   // C++11
{
	printf("%s : %s\n", *e.key, *e.value.toString());
}

for(auto& [key, value] : particle.object())   // C++17
{
	printf("%s : %s\n", *key, *value.toString());
}
~~~

Any Var can be converted to a String representation for example for printing on a console or to a file. This is done
with the `toString()` method. For instance, the following:

~~~
printf("%s\n", *particle.toString());
~~~

will print the string:

~~~
{color=[255,0,255],name=particle1,visible=Y,x=15,y=-1.25}
~~~

For a better representation that can be parsed back into a Var, you can use XDL (`Xdl::encode(var)`) or JSON (`Json::encode(var)`).

*/


class ASL_API Var
{
  public:
	enum Type {NONE, NUL, NUMBER, BOOL, INT, SSTRING, STRING=8, ARRAY, DIC, OBJ=10};
	bool isPod() const {return (_type & 8)==0;}
	Var(): _type(NONE) {}
	Var(Type t);
	Var(const Var& v)
	{
		memcpy(this, &v, sizeof(v));
		if(!isPod())
			copy(v);
	}
	void copy(const Var& v);
#ifdef ASL_HAVE_MOVE
	Var(Var&& v) {memcpy(this, &v, sizeof(Var)); v._type = NONE;}
	void operator=(Var&& v) {swap(*this, v);}
#endif
#ifdef ASL_HAVE_INITLIST

	struct Obj { const char* key; const Var& value; };
	
	Var(const std::initializer_list<Obj> b)
	{
		_type = DIC;
		NEW_DIC(o);
		o->reserve((int)b.size());
		for (const Obj* p = b.begin(); p != b.end(); p++)
			o->set(p->key, p->value);
	}
	void operator=(std::initializer_list<Obj> b)
	{
		if (_type != DIC) {
			free();
			_type = DIC;
			NEW_DIC(o);
		}
		else
			o->clear();
		o->reserve((int)b.size());
		for (const Obj* p = b.begin(); p != b.end(); p++)
			o->set(p->key, p->value);
	}
	/**
	Constructs an array with an initializer list
	*/
	static Var array(std::initializer_list<Var> b)
	{
		return Var(Array<Var>(b));
	}
#endif

	Var(const String& v)
	{
		if(v.length() < VAR_SSPACE) {
			_type=SSTRING;
			strcpy(ss, v);
		}
		else {
			_type=STRING;
			NEW_STRINGC(s, v.length()+1);
			strcpy(&(*s)[0], &v[0]);
		}
	}
	template<class T>
	Var(const Array<T>& v);
	template<class T>
	Var(const HDic<T>& v);
	Var(const Array<Var>& v) {_type=ARRAY; NEW_ARRAYC(a, v);}
	Var(const HDic<Var>& v) {_type=DIC; NEW_DICC(o, v);}
	Var(double x); // : _type(NUMBER), d(x){}
	Var(int x): _type(INT), i(x){}
	Var(float x): _type(NUMBER) {d=x;}
	Var(unsigned x);
	Var(long x) : _type(INT), i((int)x){}
	Var(unsigned long x) : _type(INT), i((int)x){}
	Var(Long x);
	Var(ULong x);
	Var(bool x);
	Var(char x);
	Var(const char* x);
	Var(const String& x0, const Var& x1);
	~Var()
	{
		//if(_type != NONE)
		if(!isPod())
			free();
	}
	String toString() const;
	/** Returns a string representation of this var */
	String string() const { return toString(); }
	/** Returns the internal type of this var */
	Type type() const {return _type != SSTRING? _type : STRING;}

	operator double() const;
	operator float() const;
	operator int() const;
	operator unsigned() const;
	operator Long() const;
	operator ULong() const { return (ULong)Long(*this); }
	operator String() const;
	template<class T>
	operator Array<T>() const;
	template<class T>
	operator HDic<T>() const;

	/**
	Returns the internal Dic if this var is an object
	*/
	Dic<Var> object() const { return _type == OBJ ? *o : Dic<Var>(); }

	/**
	Returns the internal Array if this var is an array
	*/
	Array<Var> array() const { return _type == ARRAY ? *a : Array<Var>(); }

	/**
	Returns the boolean value of this var (similar to JS conversion)
	*/
	operator bool() const;
	operator const char*() const;
	const char* operator*() const {return (const char*)*this;}
	/** Returns this var if it is defined or `v` otherwise */
	const Var& operator|(const Var& v) const { return is(NONE) ? v : *this; }

	template<class T>
	void read(const String& key, T& x) const { if (has(key)) x = (*this)[key]; }
	/**
	Returns the bool value of this var
	\deprecated Use `bool` conversion
	*/
	bool isTrue() const;
	/**
	Returns true if this var has a value (its type is not NONE)
	*/
	bool ok() const { return _type != NONE; }

	void operator=(const Var& x);
	void operator=(double x);
	void operator=(int x);
	void operator=(Long x);
	void operator=(ULong x) { (*this) = (Long)x; }
	void operator=(float x);
	void operator=(unsigned x);
	void operator=(long x) { *this = (int)x; }
	void operator=(unsigned long x) { *this = (unsigned int)x; }
	void operator=(bool x);
	void operator=(const char* x);
	void operator=(const String& x);
	template<class T>
	void operator=(const Array<T>& x);
	template<class T>
	void operator=(const HDic<T>& x);
	/** Appends `x` to this var if this var is an array (useful for Var construction) */
	Var& operator,(const Var& x) {return (*this) << (Var)x;}
	/** Appends `x` to this var if this var is an array */
	Var& operator<<(const Var& x);
	/** Appends `x` to this var if this var is an array */
	template<class T>
	Var& operator<<(const T& x) {return (*this) << (Var)x;}
	/** Resizes this var to `n` elements if this var is an array, converting if was NONE */
	void resize(int n)
	{
		if(_type==NONE) {
			_type=ARRAY;
			NEW_ARRAY(a);
		}
		else if(_type==ARRAY)
			a->resize(n);
	}
	/** Returns the element at index `i` if this var is an array */
	Var& operator[](int i) const;
	/** Returns the element at index `i` if this var is an array */
	Var& operator[](int i);
	/** Returns the property named `key` if this var is an object */
	Var& operator[](const String& key);
	/** Returns the property named `key` if this var is an object */
	Var& operator[](const String& key) const;
	
	Var operator()(const String& key) const { return has(key) ? (*this)[key] : Var(); }

	/** Sets the value of property `key` of this var to `v` (Useful for Var construction) */
	template <class T>
	Var& operator()(const char* key, const T& v) {(*this)[String(key)]=v; return *this;}
	template <class T>
	Var& operator()(const String& key, const T& v) {(*this)[key]=v; return *this;}
	/** Returns the property named `key` if this var is an object */
	Var& operator[](const char* key) {return (*this)[String(key)];}
	/** Returns the property named `key` if this var is an object */
	Var& operator[](const char* key) const {return (*this)[String(key)];}

	/** Returns the length of this var if it is an array or a dic */
	int length() const;
	bool operator==(const Var& other) const
	{
		if(_type == STRING && other._type == SSTRING)
			return !strcmp((*s).ptr(), other.ss);
		else if(_type == SSTRING && other._type == STRING)
			return !strcmp(ss, (*other.s).ptr());
		else if(_type != other._type) return false;
		switch(_type){
			case NUMBER: return d==other.d;
			case INT: return i==other.i;
			case BOOL: return b==other.b;
			case STRING: return !strcmp((*s).ptr(), (*other.s).ptr());
			case SSTRING: return !strcmp(ss, other.ss);
			case ARRAY: return *a==*other.a;
			case DIC: return *o == *other.o;
			case NUL: return true;
			default: return false;
		}
		return false;
	}
	template<class T>
	bool operator!=(const T& other) const
	{
		return !(*this == other);
	}
	bool operator==(bool other) const
	{
		return _type == BOOL && b==other;
	}
	bool operator==(int other) const
	{
		switch(_type){
		case INT: return i==other;
		case NUMBER: return d==other;
		default: return false;
		}
		return false;
	}
	bool operator==(double other) const
	{
		switch(_type){
		case NUMBER: return d==other;
		case INT: return i==other;
		default: return false;
		}
		return false;
	}
	bool operator==(const char* other) const
	{
		switch(_type){
		case STRING: return !strcmp((*s).ptr(), other);
		case SSTRING: return !strcmp(ss, other);
		default: return false;
		}
		return false;
	}
	bool operator==(const String& other) const
	{
		switch(_type){
		case STRING: return !strcmp((*s).ptr(), &other[0]);
		case SSTRING: return !strcmp(ss, other);
		default: return false;
		}
		return false;
	}

	/** Checks if this var's type is `t`. */
	bool is(Type t) const
	{
		return _type == t || (t==NUMBER && _type == INT) ||
			(t==STRING && _type == SSTRING) || (t==SSTRING && _type == STRING);
	}
	/** Checks if this var is an object of class `clas`. */
	bool is(const char* clas) const {return _type == DIC && (*o)[ASL_XDLCLASS] == clas;}
	/** Checks if this var is an object and has a property named `k`. */
	bool has(const String& k) const
	{
		return (_type==DIC)? o->has(k) : false;
	}
	/** Checks if this var is an object and has a property named `k` of type `t`. */
	bool has(const String& k, Type t) const
	{
		return (_type==DIC)? o->has(k) && (*o)[k].is(t) : false;
	}
	/** Checks if this var is an array and contains an element with value `x`. */
	bool contains(const Var& x) const
	{
		return (_type==ARRAY)? a->contains(x) : false;
	}
	/** Clears the contents if this var is an array or an object. */
	void clear()
	{
		if (_type==ARRAY)
			a->clear();
		else if (_type==DIC)
			o->clear();
	}

	/**
	Returns an independent copy of this Var (for arrays and objects)
	*/
	Var clone() const;
	struct Enumerator
	{
		Var& v;
#ifndef ASL_VAR_STATIC
		HDic<Var>::Enumerator* e;
#else
		StaticSpace< HDic<Var>::Enumerator > e;
#endif
		int i;
		Enumerator(const Var& x) : v(*(Var*)&x), i(0)
		{
			if(x._type==DIC)
#ifndef ASL_VAR_STATIC
				e=new HDic<Var>::Enumerator(*x.o);
#else
				e.construct(*x.o);
#endif
		}
		~Enumerator()
		{
			if(v._type==DIC)
#ifndef ASL_VAR_STATIC
				delete e;
#else
				e.destroy();
#endif
		}
		void operator++() {i++; if(v._type==DIC) ++*e;}
		Var& operator*() {if(v._type==ARRAY) return (*v.a)[i]; else if(v._type==DIC) return **e; else return v;}
		String operator~() {return ~*e;}
		operator bool() const {return i < v.length();}
		bool operator!=(const Enumerator& e) const { return (bool)*this; }
	};
	/** Returns an enumerator for this var's contents */
	Enumerator all() const {return Enumerator(*this);}

	friend struct Enumerator;

 protected:
	 Type _type;
	 union{double d; int i; bool b;
#ifndef ASL_VAR_STATIC
	Array<Var>* a;
	HDic<Var>* o;
	Array<char>* s;
#else
	StaticSpace< Array<Var> > a;
	StaticSpace< HDic<Var> > o;
	StaticSpace< Array<char> > s;
#endif
	char ss[VAR_SSPACE];};
	void free();
	friend class XdlEncoder;
};

template<class T>
Var::Var(const Array<T>& v)
{
	_type=ARRAY;
	NEW_ARRAY(a);
	a->resize(v.length());
	for(int i=0; i<v.length(); i++)
		(*a)[i] = v[i];
}

template<class T>
Var::operator Array<T>() const
{
	Array<T> a2;
	if(_type==ARRAY)
	{
		a2.resize(a->length());
		for(int i=0; i < a2.length(); i++)
			a2[i]=(*a)[i];
	}
	return a2;
}

template<class T>
Var::operator HDic<T>() const
{
	HDic<T> a2;
	if (_type == DIC)
	{
		foreach2(String& k, Var& v, *o)
			a2[k] = v;
	}
	return a2;
}

template<class T>
Var::Var(const HDic<T>& x)
{
	_type=DIC;
	NEW_DIC(o);
	foreach2(String& k, T& v, x)
		o->set(k, v);
}

template<class T>
void Var::operator=(const Array<T>& x)
{
	free();
	_type=ARRAY;
	NEW_ARRAY(a);
	a->resize(x.length());
	for(int i=0; i<x.length(); i++)
		(*a)[i] = x[i];
}

template<class T>
void Var::operator=(const HDic<T>& x)
{
	free();
	_type=DIC;
	NEW_DIC(o);
	foreach2(String& k, T& v, x)
		o->set(k, v);
}

#ifdef ASL_HAVE_RANGEFOR

inline Var::Enumerator begin(const Var& a)
{
	return a.all();
}

inline Var::Enumerator end(const Var& a)
{
	return a.all();
}

#endif

}

#undef NEW_ARRAY
#undef NEW_ARRAYC
#undef DEL_ARRAY
#undef NEW_DIC
#undef NEW_DICC
#undef DEL_DIC

#endif
