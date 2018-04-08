#include <asl/Array.h>
#include <asl/String.h>
#include <asl/Factory.h>

/*
This Windows import/export stuff is not normally necessary.
We use it in this sample so that classes registered in a library linked at build time
are added to the same factory used in the executable.
*/

#ifndef _WIN32
#define DEMO_API
#else
#ifdef lib_EXPORTS
#define DEMO_API __declspec(dllexport)
#else
#define DEMO_API __declspec(dllimport)
#endif
#endif


// interface

class Animal
{
public:
	virtual void speak() {};
	virtual ~Animal() {}
};

// make the factory exported or imported so that the same instance is used

template class DEMO_API asl::Factory<Animal>;
