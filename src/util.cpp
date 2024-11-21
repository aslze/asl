#include <asl/util.h>
#include <asl/String.h>
#include <stdio.h>

#ifdef _WIN32
#include <wincrypt.h>
#else
#include <sys/time.h>
#include <unistd.h>
#endif

namespace asl {

void Random::getBytes(void* buffer, int n)
{
#ifdef _WIN32
	HCRYPTPROV cp;
	if (CryptAcquireContextW(&cp, NULL, NULL, PROV_RSA_FULL, 0))
	{
		if (CryptGenRandom(cp, n, (BYTE*)buffer))
		{
			CryptReleaseContext(cp, 0);
			return;
		}
	}
#else
	FILE* f = fopen("/dev/urandom", "rb");
	if (f)
	{
		if((int)fread(buffer, 1, n, f) == n) {
			fclose(f);
			return;
		}
		fclose(f);
	}
#endif
	Random random;
	for(int i=0; i<n; i++)
		((byte*)buffer)[i] = (byte)random(255);
}

#if 0 // linear congruential

const int Random::_size = 1;

unsigned Random::get()
{
	const unsigned a = 1103515245;
	const unsigned c = 12345;
	const ULong m = 1ull << 31;
	_state[0] = (((ULong)_state[0] * a + c) % m);
	return _state[0] & 0xffffffff;
}

ULong Random::getLong()
{
	return ((ULong)get() << 32) | (ULong)get();
}

#elif 0 // xoroshiro128+

const int Random::_size = 2;

inline ULong rotl(ULong x, int k)
{
	return (x << k) | (x >> (64 - k));
}

ULong Random::getLong()
{
	const ULong s0 = _state[0];
	ULong s1 = _state[1];
	const ULong result = s0 + s1;
	s1 ^= s0;
	_state[0] = rotl(s0, 24) ^ s1 ^ (s1 << 16); // a, b
	_state[1] = rotl(s1, 37); // c
	return result;
}

unsigned Random::get()
{
	return (getLong() >> 32) & 0xffffffff;
}

#else // xoshiro256**

const int Random::_size = 4;

inline ULong rotl(const ULong x, int k)
{
	return (x << k) | (x >> (64 - k));
}

ULong Random::getLong()
{
	const ULong result = rotl(_state[1] * 5, 7) * 9;
	const ULong t = _state[1] << 17;
	_state[2] ^= _state[0];
	_state[3] ^= _state[1];
	_state[1] ^= _state[2];
	_state[0] ^= _state[3];
	_state[2] ^= t;
	_state[3] = rotl(_state[3], 45);
	return result;
}

unsigned Random::get()
{
	return unsigned(getLong() >> 32);
}

#endif

Random::Random(bool autoseed, bool fast)
{
	if(!autoseed)
		seed(0);
	else
		init(fast);
}

void Random::seed(ULong s)
{
	s ^= 0x7a12345fb678ce93ull;
	for (int i = 0; i < _size; i++)
		_state[i] = s ^ (s << (i + 2));

	for (int i = 0; i < 6; i++)
		unsigned x = get();
}

void Random::init(bool fast)
{
	if (fast)
	{
		ULong s = (ULong)inow();
		for (int i = 0; i < (int)sizeof(s)/2; i++)
			s ^= ((s & (255ull << i)) ^ (255ull << i)) << 8 * (sizeof(s) - i - i - 1);
		seed(s);
	}
	else
		getBytes(_state, sizeof(_state));
}

Random random;

double now()
{
#ifdef _WIN32
	static double qpcPeriod = 0;
	if (qpcPeriod == 0)
	{
		LARGE_INTEGER n;
		QueryPerformanceFrequency(&n);
		qpcPeriod = 1.0 / n.QuadPart;
	}
	LARGE_INTEGER n;
	QueryPerformanceCounter(&n);
	return ((double)(n.QuadPart))*qpcPeriod;
	// return 0.001*GetTickCount();
#else
	timeval t;
	gettimeofday(&t, 0);
	return t.tv_sec + 1e-6*t.tv_usec;
#endif
}

Long inow()
{
#ifdef _WIN32
	FILETIME ft;
	GetSystemTimeAsFileTime(&ft);
	LARGE_INTEGER t;
	t.HighPart = ft.dwHighDateTime;
	t.LowPart = ft.dwLowDateTime;
	return t.QuadPart / 10;
#else
	timeval t;
	gettimeofday(&t, 0);
	return t.tv_sec * (Long)1000000 + t.tv_usec;
#endif
}

#ifdef _WIN32

void sleep(int s)
{
	Sleep(s * 1000);
}

void usleep(int us)
{
	Sleep(us / 1000);
}

void sleep(double s)
{
	Sleep((int)(1e3 * s));
}
#else

void sleep(int s)
{
	::sleep((unsigned)s);
}

void sleep(double s)
{
	usleep((int)(1e6 * s));
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

#ifdef ASL_B64_NOWS

ByteArray decodeBase64(const char* src0, int n)
{
	const byte* src = (const byte*)src0;
	int len = n < 0? (int)strlen(src0) : n;
	int len2 = len / 4 * 3;
	ByteArray result(len2);
	if (len & 3) {
		result.clear();
		return result;
	}
	if (src[len - 2] == '=')
		len2-= 2;
	else if (src[len - 1] == '=')
		len2--;
	result.resize(len2);

	byte* dest = result.data();
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

#else

ByteArray decodeBase64(const char* src0, int n)
{
	const byte* src = (const byte*)src0;
	int len = n < 0 ? (int)strlen(src0) : n;
	int len2 = len / 4 * 3;
	ByteArray result(len2);
	if (len < 4) {
		result.clear();
		return result;
	}
	int e = 0;
	const byte* p = &src[len - 1];
	while (p > src && !(myisalnum(*p) || *p == '/' || *p == '+'))
		if (*p-- == '=')
			e++;

	byte* dest = result.data();
	byte k[4];
	int i = 0;
	while (*src) {
		if (myisspace(*src)) { src++; continue; };
		byte b = base64_chars_inv[*src++];
		k[i++] = b;
		if (i == 4)
		{
			unsigned u = (k[0] << 18) | (k[1] << 12) | (k[2] << 6) | k[3];
			*dest++ = byte(u >> 16);
			*dest++ = byte((u >> 8) & 0xff);
			*dest++ = byte(u & 0xff);
			i = 0;
		}
	}
	result.resize(int(dest - result.data()) - e);
	return result;
}

#endif

String encodeHex(const byte* data, int n)
{
	String h(2*n, 2*n);
	for (int i = 0; i < n; i++)
		snprintf(&h[2*i], 3, "%02x", data[i]);
	return h;
}

ByteArray decodeHex(const String& s)
{
	ByteArray a(s.length() / 2);
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

 double ASL_API bigval = 1e300;

}
