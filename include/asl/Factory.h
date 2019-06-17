// Copyright(c) 1999-2019 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_FACTORY
#define ASL_FACTORY

/* \file Factory.h */ 

#include <asl/Singleton.h>
#include <asl/Map.h>

namespace asl {

/**
A Factory allows creating objects given a class name as a String. They are used through pointers
to their base class and through virtual functions. A Factory is created for a given base class, and
subclasses must be registered with `ASL_FACTORY_REGISTER()` before use.

~~~
class Dog : public Animal {...};

ASL_FACTORY_REGISTER( Animal, Dog )
...
Animal* animal = Factory<Animal>::create("Dog");
~~~
\ingroup Factory
*/

template<class T, class K=void*>
class Factory : public Singleton< Factory<T,K> >
{
	Dic<T*(*)()> _constructors;
	Dic<T*(*)(K)> _constructors2; // constructors with an argument [deprecated]
	typedef Singleton< Factory<T,K> > S;
public:
	Factory(){}

	/** Constructs an object given a class name previously registered */
	static T* create(const String& name)
	{
		if(!S::instance()->_constructors.has(name))
			return 0;
		return S::instance()->_constructors[name]();
	}
	static T* create(const String& name, const K& k)
	{
		if(!S::instance()->_constructors2.has(name))
			return 0;
		return S::instance()->_constructors2[name](k);
	}
	/**
	Registers a class given a function that constructs and returns a new object.
	*/
	static int add(const String& className, T*(*f)())
	{
		//printf("Registering %s on %p\n", *name, Factory::instance());
		S::instance()->_constructors[className] = f;
		return 0;
	}	
	static int add(const String& name, T*(*f)(K))
	{
		S::instance()->_constructors2[name] = f;
		return 0;
	}
	static void add(void* other)
	{
		Dic<T*(*)()> ctors1 = S::instance()->_constructors;
		Dic<T*(*)()> ctors2 = ((Factory*)(((void*(*)())other)()))->_constructors;
		foreach2(String& k, T*(*f)(), ctors2)
		{
			ctors1[k] = f;
		}
	}
	/**
	Returns a list of the class names already registered
	*/
	static Array<String> catalog()
	{
		return S::instance()->_constructors.keys();
	}

	/**
	returns true if the given class name is registered
	*/
	static bool has(const String& clas)
	{
		return S::instance()->_constructors.has(clas);
	}
};

}

/**
Registers class `Class` in the factory for class `Base` for instantiation by name (Class must be a subclass of Base).
@hideinitializer
\ingroup Factory
*/
#define ASL_FACTORY_REGISTER(Base, Class) \
	Base* create##Class() {return new Class();} \
	static int Class##_1 = asl::Factory<Base>::add(#Class, create##Class);

/**
Registers class `Class` in the factory for class `Base` for instantiation by the alternative name `Name`.
@hideinitializer
\ingroup Factory
*/
#define ASL_FACTORY_REGISTER_AS(Base, Class, Name) \
	Base* create##Class() {return new Class();} \
	static int Class##_1 = asl::Factory<Base>::add(#Name, create##Class);

/**
Exports all registered classes derived from `Base` for instantiation from a dynamic library
@hideinitializer
\ingroup Factory
*/
#define ASL_FACTORY_EXPORT(Base) \
	extern "C" \
	ASL_EXPORT void* asl_get_##Base##_instance() {return asl::Factory<Base>::instance();}

/**
Imports all registered classes derived from `Base` from the given dynamic library. That library is a `Library`
object that must be open and persistent (e.g. created in the heap so it exists while its classes are used).
@hideinitializer
\ingroup Factory
*/
#define ASL_FACTORY_IMPORT(Base, lib) \
	void* p = (lib).get("asl_get_" #Base "_instance"); \
	if(p) asl::Factory<Base>::add(p);

#endif
