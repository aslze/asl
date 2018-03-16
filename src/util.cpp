#include <asl/util.h>
#include <asl/String.h>

namespace asl {

#if 0 // linear congruential

static Long random_state = 1;
int random()
{
	const int a = 1103515245;
	const int c = 12345;
	const Long m = 1 << 31;
	random_state = (((Long)random_state * a + c) % m);
	return random_state & 0x7fffffff;
}

void random_init(Long n)
{
	random_state = (n>=0)? n : (Long)(1e9 * fract(1e-6 * now()));
}

#else // xoroshiro128+

inline ULong rotl(ULong x, int k)
{
	return (x << k) | (x >> (64 - k));
}

static ULong random_state[2] = { 1234567890U, 2876543210U };

int random()
{
	const ULong s0 = random_state[0];
	ULong s1 = random_state[1];
	const ULong result = s0 + s1;
	s1 ^= s0;
	random_state[0] = rotl(s0, 55) ^ s1 ^ (s1 << 14); // a, b
	random_state[1] = rotl(s1, 36); // c
	return (result >> 32) & 0x7fffffff;
}

void random_init(Long n)
{
	random_state[0] = (n >= 0) ? n : (Long)(1e9 * fract(1e-6 * now()));
	random_state[1] = (n >= 0) ? n : (Long)(2e9 * fract(1e-6 * now()));
}

#endif

static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const byte base64_chars_inv[] = { 
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0,  0, 0, 0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 62,  0,  0,  0, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61,  0, 0, 0, 65, 0, 0,
	0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 0, 0,  0, 0, 0,
	0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 0, 0,  0, 0, 0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0,  0, 0, 0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0,  0, 0, 0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0,  0, 0, 0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0,  0, 0, 0
};

// to build above table:
// byte base64_chars_inv_(char c) { return max(0, String(base64_chars).indexOf(c); }

String encodeBase64(const byte* data, int n)
{
	int len = 4 * ((n + 2) / 3);
	String output(len, len);

	char* dest = &output[0];
	
	for (int i = 0; i < n; i += 3)
	{
		unsigned a = data[i];
		unsigned b = (i + 1 < n)? data[i + 1] : 0;
		unsigned c = (i + 2 < n)? data[i + 2] : 0;
		unsigned u = (a << 16) | (b << 8) | c;
		
		*dest++ = base64_chars[(u >> 18) & 0x3f];
		*dest++ = base64_chars[(u >> 12) & 0x3f];
		*dest++ = base64_chars[(u >> 6) & 0x3f];
		*dest++ = base64_chars[u & 0x3f];
	}
	
	if (n % 3 > 0)
		output[len - 1] = '=';
	if (n % 3 == 1)
		output[len - 2] = '=';
	
	output[len] = '\0';
	return output;
}

Array<byte> decodeBase64(const char* src0, int n)
{
	const byte* src = (const byte*)src0;
	int len = n < 0? (int)strlen(src0) : n;
	int len2 = len / 4 * 3;
	Array<byte> result(len2);
	if (len & 3) {
		result.clear();
		return result;
	}
	if (src[len - 2] == '=')
		len2-= 2;
	else if (src[len - 1] == '=')
		len2--;
	result.resize(len2);

	byte* dest = result.ptr();
	//byte* end = dest + len2;
	while (*src) {
		byte a = base64_chars_inv[*src++];
		byte b = base64_chars_inv[*src++];
		byte c = base64_chars_inv[*src++];
		byte d = base64_chars_inv[*src++];
		unsigned u = (a << 18) | (b << 12) | (c << 6) | d;
		*dest++ = byte(u >> 16);
		*dest++ = byte((u >> 8) & 0xff);
		*dest++ = byte(u & 0xff);
	}
	return result;
}


String encodeHex(const byte* data, int n)
{
	String h(2*n, 2*n);
	for (int i = 0; i < n; i++)
		sprintf(&h[2*i], "%02x", data[i]);
	return h;
}

Array<byte> decodeHex(const String& s)
{
	Array<byte> a(s.length()/2);
	for (int i = 0; i < s.length(); i += 2)
		a[i/2] = (byte)s.substring(i, i + 2).hexToInt();
	return a;
}

void asl_die(const char* msg, int line)
{
	fprintf(stderr, "Fatal Error: %s : %i\n", msg, line);
	exit(1);
}

void asl_error(const char* msg)
{
	fprintf(stderr, "Error: %s\n", msg);
}

// deprecated

void os_error(const char* msg)
{
	fprintf(stderr, "Error: %s\n", msg);
#if 0 // def _WIN32
	LPVOID message;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&message, 0, NULL);
	MessageBoxA(NULL, (const char*)message, msg, MB_OK);
	LocalFree(message);
#endif
	exit(1);
}

double inf = 1e300;

float infinity()
{
	return float(inf);
}

}
