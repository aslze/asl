#include <asl/SharedMem.h>

namespace asl {

SharedMem::SharedMem(const String& name, int size)
{
#ifdef _WIN32

	_handle = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, name);
	if(!_handle) // not existing: create
	{
		_handle = CreateFileMapping(
			INVALID_HANDLE_VALUE,
			NULL,                    // default security
			PAGE_READWRITE,          // read/write access
			0,                       // maximum object size (high-order DWORD)
			size,                    // maximum object size (low-order DWORD)
			name);
		
		if(!_handle)
		{
			_ptr = NULL;
			return;
		}
	}
   _ptr = (byte*) MapViewOfFile(_handle, FILE_MAP_ALL_ACCESS, 0, 0, size);

#elif !defined(__ANDROID__)
	_name = '/' + name;
	_size = size;
	_handle = shm_open(_name, O_RDWR | O_CREAT, 0600);
	if(_handle == -1)
	{
		_ptr = NULL;
		return;
	}
	if(ftruncate(_handle, (off_t)_size) == -1)
	{
		_ptr = NULL;
		shm_unlink(_name);
		return;
	}
	_ptr = (byte*)mmap(0, (size_t)size, PROT_READ | PROT_WRITE, MAP_SHARED, _handle, 0);
	if(_ptr == MAP_FAILED)
		_ptr = NULL;

#endif
}

SharedMem::~SharedMem()
{
#ifdef _WIN32
	UnmapViewOfFile(_ptr);
	CloseHandle(_handle);
#elif !defined(__ANDROID__)
	if(_ptr)
	{
		munmap(_ptr, (size_t)_size);
		shm_unlink(_name);
	}
#endif
}

byte* SharedMem::ptr()
{
	return _ptr;
}


}
