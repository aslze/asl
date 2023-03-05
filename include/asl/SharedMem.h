// Copyright(c) 1999-2023 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_SHAREDMEM
#define ASL_SHAREDMEM

#include <asl/String.h>

#ifdef _WIN32
#include <windows.h>
#endif

namespace asl {

/**
Provides shared memory among processes.

A SharedMemory object must be created with a name and a size. Then other processes can use the same memory
by creating an object with the same name and size. The function ptr() returns a pointer to the base of the
shared memory block or NULL y there was a problem creating the mapping.

~~~
SharedMem memo("mymemory", 16000);
char buffer[100]; // and fill it with data
if(memo.ptr())
	memcpy( memo.ptr(), buffer, 100 );
~~~

Then another process can do:

~~~
SharedMem memo("mymemory", 16000);

char buffer[100];
if(memo.ptr())
	memcpy( buffer, memo.ptr(), 100 );
~~~

To read from the memory written by the first process.

\deprecated This class was an experiment and might be removed sometime.

*/
class ASL_API SharedMem
{
public:
	/**
	Creates a shared memory object with the given name and size in bytes. If a segment with that name does not
	exist it will be created, otherwise a reference to it will be made.
	*/
	SharedMem(const String& name, int size);
	~SharedMem();
	/**
	Returns the base address of the mapping of this block in the current process or NULL on error.
	*/
	byte* ptr();
protected:
	SharedMem(const SharedMem& m) {}

#ifdef _WIN32
	HANDLE _handle;
#else
	int _handle;
#endif
	int _size;
	String _name;
	byte* _ptr;
};

}
#endif
