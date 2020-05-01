// Copyright(c) 1999-2020 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_SINGLETON
#define ASL_SINGLETON

#include <asl/defs.h>
#include <asl/Pointer.h>

namespace asl {

/**
This class can be used to create singletons from existing classes. These are classes of which
there can only be one instance and that instance is not explicitly created.

Can be used on existing classes withoud defining one:
~~~
Singleton<DeviceManager>::instance()->initialize();
~~~

Or declaring a typedef:
~~~
typedef Singleton<DeviceManager> TheDeviceManager;
TheDeviceManager::instance()->initialize();
~~~

Or defining a class as a subclass of a singleton of itself:
~~~
class DeviceManager : public Singleton<DeviceManager>
{...};

DeviceManager::instance()->initialize();
~~~

*/

template<class T>
class Singleton
{
public:
	/**
	Returns a pointer to an instance of class T, creating it on the first call.
	*/
	static T* instance()
	{
		static T theinstance;
		return &theinstance;
	}
protected:
	Singleton() {}
	Singleton(const Singleton&) {}
	void operator=(const Singleton&) {}
};

}

#endif
