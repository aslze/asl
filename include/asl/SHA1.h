// Copyright(c) 1999-2026 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_SHA1_H
#define ASL_SHA1_H

#include <asl/defs.h>
#include <asl/Array_.h>

#if (defined( _MSC_VER ) && _MSC_VER < 1600)
typedef unsigned int uint32_t;
typedef asl::ULong uint64_t;
#else
#include <stdint.h>
#endif

namespace asl {

/**
 * SHA1 hash computation.
 * 
 * ```
 * SHA1::Hash h = SHA1::hash(data);
 * ```
 */
class ASL_API SHA1
{
public:
	typedef Array_<byte, 20> Hash;
	
	SHA1();
	static Hash hash(const void* data, int len)
	{
		SHA1 sha;
		sha.add(data, len);
		return sha.end();
	}
	static Hash hash(const char* data) { return hash(data, (int)strlen(data)); }
	static Hash hash(const ByteArray& data) { return hash(data.data(), data.length()); }
	static Hash hash(const String& data) { return hash((byte*)*data, data.length()); }
	void        add(const void* data, int len);
	Hash        end();

private:
	void transform(const byte buffer[64]);
	uint32_t _state[5];
	int _count[2];
	byte _buffer[64];
};

/**
 * SHA256 hash computation.
 *
 * ```
 * SHA256::Hash h = SHA256::hash(data);
 * ```
 */
class ASL_API SHA256
{
public:
	typedef Array_<byte, 32> Hash;

	SHA256();
	static Hash hash(const void* data, int len)
	{
		SHA256 sha;
		sha.add(data, len);
		return sha.end();
	}
	static Hash hash(const char* data) { return hash(data, (int)strlen(data)); }
	static Hash hash(const ByteArray& data) { return hash(data.data(), data.length()); }
	static Hash hash(const String& data) { return hash((byte*)*data, data.length()); }
	void        add(const void* data, int len);
	Hash        end();

private:
	void     transform();
	void     add(byte b);
	uint32_t _state[8];
	byte     _buffer[64];
	int      _count;
	uint64_t _nbits;
};

}
#endif
