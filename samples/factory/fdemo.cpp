#include <asl/Array.h>
#include <asl/File.h>
#include <asl/Directory.h>
#include <asl/Library.h>
#include <asl/Factory.h>
#include <stdio.h>

#include "Animal.h"

/*
This sample shows how to load classes at runtime in two different ways.

The example uses subclasses of class Animal (acting as an interface), defined in "Animal.h".
The sample will load all "plugin" libraries found in a directory, import their factories and
instantiate a class from each.

1) Using a factory. A Factory<Animal> can register and instantiate classes within a program
or library. But it can also export from a dynamic library for runtime importing. The factory
will contain locally defined classes (e.g. Cat, below) and classes defined in the runtime
found libraries. Those libraries (pluginA and pluginB here) register classes in a factory and
export that library.

2) Using Library::create(className), which is simpler. A subclass is exported from a library
with ASL_EXPORT_AS() and then loaded and instantiated at runtime. ASL_EXPORT_AS is used to
"rename" the class (in this case Cow to Animal), so that the program knows what name to
instantiate. A program does not normally know a priori that a library contains a Cow class,
only that it contains something derived from Animal.

What about classes defined in dynamic libraries linked at build time? On Windows by default they
will be ignored because they are registered to a different factory instance. To make them join the
same factory, the Factory<Animal> has to be marked for DLL import or export like we do here in
Animal.h.
*/

using namespace asl;


class Cat : public Animal
{
public:
	void speak()
	{
		printf("Meow! I am defined in the executable\n");
	}
};

ASL_FACTORY_REGISTER(Animal, Cat);



int main(int narg, char* argv[])
{
	// enumerate library files named as "plugin*" (.dll or .so, etc.)

	Array<File> files = Directory(".").files( ASL_LIB_PREFIX "plugin*." ASL_LIB_EXT);

	Array< Shared<Library> > libs;

	// load each file as a dynamic library and import its Animal factory into the local factory

	foreach(File& file, files)
	{
		libs << new Library(file.path());
		ASL_FACTORY_IMPORT(Animal, *libs.last());
	}

	// list registered classes

	Array<String> catalog = Factory<Animal>::catalog();

	printf("Classes: %s\n", *catalog.join(','));

	// instantiate an object of each class

	foreach(String& className, catalog)
	{
		Shared<Animal> animal = Factory<Animal>::create(className);
		animal->speak();
	}

	// now instantiate an object from the Animal class exported by the library (without factories)

	foreach(Shared<Library> lib, libs)
	{
		Shared<Animal> animal = (Animal*)lib->create("Animal");
		if (animal)
			animal->speak();
	}
}
