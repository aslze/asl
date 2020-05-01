// Copyright(c) 1999-2020 aslze
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

template<class T>
class Factory : public Singleton< Factory<T> >
{
	Dic<T*(*)()> _constructors;
	Dic<> _classInfo;
	typedef Singleton< Factory<T> > S;
public:
	Factory(){}

	/** Constructs an object given a class name previously registered */
	static T* create(const String& name)
	{
		if(!S::instance()->_constructors.has(name))
			return 0;
		return S::instance()->_constructors[name]();
	}
	/**
	Registers a class given a function that constructs and returns a new object.
	*/
	static int add(const String& className, T*(*f)())
	{
		S::instance()->_constructors[className] = f;
		return 0;
	}

	static void add(void* other)
	{
		Factory* fact = (Factory*)(((void*(*)())other)());
		S::instance()->_constructors.add(fact->_constructors);
		S::instance()->_classInfo.add(fact->_classInfo);
	}

	/**
	Returns a list of the class names already registered
	*/
	static Array<String> catalog()
	{
		return S::instance()->_constructors.keys();
	}

	/**
	Returns true if the given class name is registered
	*/
	static bool has(const String& clas)
	{
		return S::instance()->_constructors.has(clas);
	}
	/**
	Associates an information string with a registered class
	*/
	static int setClassInfo(const String& clas, const Dic<>& info)
	{
		S::instance()->_classInfo[clas] = info.join(',', '=');
		return 0;
	}
	/**
	Returns the information associated with a class
	*/
	static const Dic<> classInfo(const String& clas)
	{
		return S::instance()->_classInfo[clas].split(',', '=');
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


#define ASL_FACTORY_SET_INFO(Base, Class, Info) \
	static int Base##_Class##_i = Factory<Base>::setClassInfo(#Class, (Info));

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
