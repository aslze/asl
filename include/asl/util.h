// Copyright(c) 1999-2018 ASL author
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_UTIL_H
#define ASL_UTIL_H

#include <asl/String.h>
#include <asl/Array.h>

namespace asl {

template<class T, int N>
class Array_;

/**
\defgroup Global Global functions
@{
*/

ASL_API Array<byte> decodeBase64(const char* src, int n = -1);

/**
Decodes a base64 encoded string into a byte array.
*/
inline Array<byte> decodeBase64(const String& s)
{
	return decodeBase64((const char*)s, s.length());
}

ASL_API String encodeBase64(const byte* data, int n);

/**
Encodes a byte array as a string using base64 encoding.
*/
inline String encodeBase64(const Array<byte>& s)
{
	return encodeBase64(s.ptr(), s.length());
}

/**
Encodes a string as a string using base64 encoding.
*/
inline String encodeBase64(const String& s)
{
	return encodeBase64((const byte*)&s[0], s.length());
}


//template<int N>
//String encodeBase64(const Array_<byte,N>& src) { return encodeBase64((byte*)src, N); }

ASL_API String encodeHex(const byte* data, int n);

/**
Encodes a byte array as a string using hexadecimal
*/
inline String encodeHex(const Array<byte>& src) { return encodeHex(src.ptr(), src.length()); }

/**
Decodes a hexadecimal encoded string into a byte array
*/
ASL_API Array<byte> decodeHex(const String& src);

/**@}*/

}

#endif
