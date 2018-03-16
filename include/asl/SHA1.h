// Copyright(c) 1999-2018 ASL author
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_SHA1_H
#define ASL_SHA1_H

#include <asl/defs.h>
#include <asl/Array_.h>

#if (defined( _MSC_VER ) && _MSC_VER < 1600)
typedef unsigned int uint32_t;
#else
#include <stdint.h>
#endif

namespace asl {

class SHA1
{
public:
	typedef Array_<byte, 20> Hash;
	
	SHA1();
	static Hash hash(const byte* data, int len);
	static Hash hash(const char* data) { return hash((const byte*)data, (int)strlen(data)); }
	static Hash hash(const Array<byte>& data);
	static Hash hash(const String& data);
private:
	void transform(const byte buffer[64]);
	void update(const byte* data, int len);
	Hash end();
	uint32_t state[5];
	int count[2];
	byte buffer[64];
};

}
#endif
