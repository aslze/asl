// Copyright(c) 1999-2022 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_LIB_H
#define ASL_LIB_H

#include <asl/defs.h>
#include <asl/String.h>

#ifdef _WIN32
#include <windows.h>
/**
Extension of dynamic/shared libraries (OS-dependent: "dll", "so" or "dylib")
@hideinitializer
\ingroup Library
*/
#define ASL_LIB_EXT "dll"

/**
Standard prefix of libraries (OS-dependent: "lib" on Unix)
@hideinitializer
\ingroup Library
*/
#define ASL_LIB_PREFIX ""
#define ASL_PATH_SEP '\\'
#else
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#define HMODULE void*
#ifdef __APPLE__
 #define ASL_LIB_EXT "dylib"
 #define ASL_LIB_PREFIX "lib"
#else
 #define ASL_LIB_EXT "so"
 #define ASL_LIB_PREFIX "lib"
#endif
#define ASL_PATH_SEP '/'

#endif

namespace asl {

/**
This class represents a dynamically loadable library (a DLL on Windows,
a shared library on Linux). It can be used to load functions at runtime, or
to create objects of classes defined in the library. To create and use objects
they need to be derived from known base class (acting as an interface declaration).

~~~
Library lib("mathtools");  // will append .dll on Windows, .so on Linux or .dylib on Mac
typedef double (*Function)(double);
Function tanh = lib.get("tanh");
double y = tanh(1.57);
~~~

Make sure the object is *alive* while functions from it are used. Keep a (smart) pointer
instead of a value object if needed.

Use `ASL_EXPORT_CLASS` to export classes from a library so that they can be easily
imported in applications with `library.create("ClassName")`.

Exporting and importing classes needs a public header:

~~~
class Animal
{
public:
	virtual void speak() {}
};
~~~

One implementation built into dynamic library `cat.dll`:

~~~
class Cat : public Animal
{
public:
	void speak() { printf("Miaow!\n"); }
};

ASL_EXPORT_CLASS(Cat)
~~~

Then another program at runtime can load the library and create an object of class `Cat`. It only needs the public base class
header ("Animal.h"), not the `Cat` subclass.

~~~
Library lib("plugins/cat");
Animal* cat = lib.create("Cat");
cat->speak();
~~~

The class can be exported with an alternative name, for example to have many plugins export classes with the same
name so that clients don't need knowledge about what classes are exported.

~~~
ASL_EXPORT_CLASS_AS(Cat, Animal)
Animal* cat = lib.create("Animal");
~~~

\ingroup Library
*/

class Library
{
	HMODULE _lib;
 public:
	Library()
	{
		_lib=0;
	}

	/**
	Creates and opens a dynamic library
	*/
	Library(const String& name)
	{
		_lib=0;
		open(name);
	}
	~Library()
	{
		close();
	}
	
	/**
	Opens a dynamic library
	*/
	void open(String file, bool tryprefix=true)
	{
		if(file.indexOf('.', 2) < 0)
			file += ASL_LIB_EXT;
#ifdef _WIN32
		file.replaceme('/', '\\');
		_lib = LoadLibrary(file);
#else
		_lib = dlopen(file, RTLD_LAZY | RTLD_GLOBAL);
#endif
		if(!_lib && tryprefix)
		{
			if(file.contains(ASL_PATH_SEP))
			{
				int i=file.lastIndexOf(ASL_PATH_SEP);
				file = file.substring(0, i+1) + "lib" + file.substring(i+1);
			}
			else
				file = "lib" + file;
			open(file, false);
		}
	}

	/**
	Closes the library
	*/
	void close()
	{
		if(_lib)
#ifdef _WIN32
			FreeLibrary(_lib);
#else
			dlclose(_lib);
#endif
		_lib=0;
	}
	
	/**
	Returns true if the library was loaded correctly
	*/
	bool loaded() const
	{
		return _lib!=0;
	}

	operator bool() const
	{
		return loaded();
	}
	
	/**
	Gets the address of the given symbol name in the library
	*/
	void* get(const char* sym)
	{
#ifdef _WIN32
		return (void*)GetProcAddress(_lib, sym);
#else
		return dlsym(_lib, sym);
#endif
	}
	
	/**
	Creates an object of class 'className' exported in the library with the `ASL_EXPORT_CLASS` macro
	Returns a null pointer if that class is not exported
	*/
	void* create(const String& className)
	{
		void* p = get("new_" + className);
		if(!p)
			return 0;
		return ((void*(*)())p)();
	}
};

/**
Export a class to be instantiated with dynamic runtime loading with `Library::create("className")`.
The argument must be an identifier (no "::") and it will be used when importing.
@hideinitializer
\ingroup Library
*/
#define ASL_EXPORT_CLASS(Class) \
extern "C" {\
	ASL_EXPORT void* new_##Class() {return new Class;}\
}

/**
Export a class to be instantiated with dynamic runtime loading but giving a specific name for importing.
The arguments must be identifiers (no "::") and the second will be used when importing.
@hideinitializer
\ingroup Library
*/
#define ASL_EXPORT_CLASS_AS(Class, Name) \
extern "C" {\
	ASL_EXPORT void* new_##Name() {return new Class;}\
}

}
#endif
