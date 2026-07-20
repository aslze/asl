/*
SHA1 based on code by Steve Reid <steve@edmweb.com> Public domain
SHA256 based on https://github.com/983/SHA-256 Public domain
*/

#define SHA1HANDSOFF

#include <stdio.h>
#include <string.h>
#include <asl/SHA1.h>

#ifdef _MSC_VER
#pragma warning(disable : 26451 26495)
#endif

namespace asl {

#define rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

/* blk0() and blk() perform the initial expand. */
#ifndef ASL_BIGENDIAN
#define blk0(i) (block->l[i] = (rol(block->l[i],24)&0xFF00FF00)|(rol(block->l[i],8)&0x00FF00FF))
#else
#define blk0(i) block->l[i]
#endif

#define blk(i) (block->l[i&15] = rol(block->l[(i+13)&15]^block->l[(i+8)&15]^block->l[(i+2)&15]^block->l[i&15],1))

/* (R0+R1), R2, R3, R4 are the different operations used in SHA1 */
#define R0(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk0(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R1(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R2(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0x6ED9EBA1+rol(v,5);w=rol(w,30);
#define R3(v,w,x,y,z,i) z+=(((w|x)&y)|(w&x))+blk(i)+0x8F1BBCDC+rol(v,5);w=rol(w,30);
#define R4(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0xCA62C1D6+rol(v,5);w=rol(w,30);

SHA1::SHA1()
{
	_state[0] = 0x67452301;
	_state[1] = 0xEFCDAB89;
	_state[2] = 0x98BADCFE;
	_state[3] = 0x10325476;
	_state[4] = 0xC3D2E1F0;
	_count[0] = _count[1] = 0;
	memset(_buffer, 0, sizeof(_buffer));
}

// Hash a single 512-bit block. This is the core of the algorithm.

void SHA1::transform(const byte buf[64])
{
	uint32_t a, b, c, d, e;
	union Char64Int16 {
		byte c[64];
		uint32_t l[16];
	};
	Char64Int16 block[1];
	memcpy(block, buf, 64);
	a = _state[0];
	b = _state[1];
	c = _state[2];
	d = _state[3];
	e = _state[4];
	/* 4 rounds of 20 operations each. Loop unrolled. */
	R0(a, b, c, d, e, 0);
	R0(e, a, b, c, d, 1);
	R0(d, e, a, b, c, 2);
	R0(c, d, e, a, b, 3);
	R0(b, c, d, e, a, 4);
	R0(a, b, c, d, e, 5);
	R0(e, a, b, c, d, 6);
	R0(d, e, a, b, c, 7);
	R0(c, d, e, a, b, 8);
	R0(b, c, d, e, a, 9);
	R0(a, b, c, d, e, 10);
	R0(e, a, b, c, d, 11);
	R0(d, e, a, b, c, 12);
	R0(c, d, e, a, b, 13);
	R0(b, c, d, e, a, 14);
	R0(a, b, c, d, e, 15);
	R1(e, a, b, c, d, 16);
	R1(d, e, a, b, c, 17);
	R1(c, d, e, a, b, 18);
	R1(b, c, d, e, a, 19);
	R2(a, b, c, d, e, 20);
	R2(e, a, b, c, d, 21);
	R2(d, e, a, b, c, 22);
	R2(c, d, e, a, b, 23);
	R2(b, c, d, e, a, 24);
	R2(a, b, c, d, e, 25);
	R2(e, a, b, c, d, 26);
	R2(d, e, a, b, c, 27);
	R2(c, d, e, a, b, 28);
	R2(b, c, d, e, a, 29);
	R2(a, b, c, d, e, 30);
	R2(e, a, b, c, d, 31);
	R2(d, e, a, b, c, 32);
	R2(c, d, e, a, b, 33);
	R2(b, c, d, e, a, 34);
	R2(a, b, c, d, e, 35);
	R2(e, a, b, c, d, 36);
	R2(d, e, a, b, c, 37);
	R2(c, d, e, a, b, 38);
	R2(b, c, d, e, a, 39);
	R3(a, b, c, d, e, 40);
	R3(e, a, b, c, d, 41);
	R3(d, e, a, b, c, 42);
	R3(c, d, e, a, b, 43);
	R3(b, c, d, e, a, 44);
	R3(a, b, c, d, e, 45);
	R3(e, a, b, c, d, 46);
	R3(d, e, a, b, c, 47);
	R3(c, d, e, a, b, 48);
	R3(b, c, d, e, a, 49);
	R3(a, b, c, d, e, 50);
	R3(e, a, b, c, d, 51);
	R3(d, e, a, b, c, 52);
	R3(c, d, e, a, b, 53);
	R3(b, c, d, e, a, 54);
	R3(a, b, c, d, e, 55);
	R3(e, a, b, c, d, 56);
	R3(d, e, a, b, c, 57);
	R3(c, d, e, a, b, 58);
	R3(b, c, d, e, a, 59);
	R4(a, b, c, d, e, 60);
	R4(e, a, b, c, d, 61);
	R4(d, e, a, b, c, 62);
	R4(c, d, e, a, b, 63);
	R4(b, c, d, e, a, 64);
	R4(a, b, c, d, e, 65);
	R4(e, a, b, c, d, 66);
	R4(d, e, a, b, c, 67);
	R4(c, d, e, a, b, 68);
	R4(b, c, d, e, a, 69);
	R4(a, b, c, d, e, 70);
	R4(e, a, b, c, d, 71);
	R4(d, e, a, b, c, 72);
	R4(c, d, e, a, b, 73);
	R4(b, c, d, e, a, 74);
	R4(a, b, c, d, e, 75);
	R4(e, a, b, c, d, 76);
	R4(d, e, a, b, c, 77);
	R4(c, d, e, a, b, 78);
	R4(b, c, d, e, a, 79);
	_state[0] += a;
	_state[1] += b;
	_state[2] += c;
	_state[3] += d;
	_state[4] += e;
	a = b = c = d = e = 0;
#ifdef SHA1HANDSOFF
	memset(block, '\0', sizeof(block));
#endif
}

void SHA1::add(const void* data, int len)
{
	int j = _count[0], i = 0;
	if ((_count[0] += (len << 3)) < j)
		_count[1]++;
	_count[1] += (len >> 29);
	j = (j >> 3) & 63;
	if ((j + len) > 63)
	{
		memcpy(&_buffer[j], data, (i = 64 - j));
		transform(_buffer);
		for ( ; i + 63 < len; i += 64)
		{
			transform(&((const byte*)data)[i]);
		}
		j = 0;
	}
	else i = 0;
	memcpy(&_buffer[j], &((const byte*)data)[i], len - i);
}


SHA1::Hash SHA1::end()
{
	Hash digest;
	byte finalcount[8];

	for (int i = 0; i < 8; i++)
		finalcount[i] = (byte)((_count[(i >= 4 ? 0 : 1)] >> ((3 - (i & 3)) * 8) ) & 255);  // Endian independent
	byte c = 0x80;
	add(&c, 1);
	while ((_count[0] & 504) != 448)
	{
		c = 0;
		add(&c, 1);
	}
	add(finalcount, 8);  // Should cause a SHA1.transform()
	for (int i = 0; i < 20; i++)
		digest[i] = (byte)((_state[i>>2] >> ((3 - (i & 3)) * 8) ) & 255);
	memset(this, '\0', sizeof(*this));
	memset(&finalcount, '\0', sizeof(finalcount));
	return digest;
}

// SHA256

inline uint32_t rotr(uint32_t x, int n)
{
	return (x >> n) | (x << (32 - n));
}

inline uint32_t step1(uint32_t e, uint32_t f, uint32_t g)
{
	return (rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25)) + ((e & f) ^ ((~e) & g));
}

inline uint32_t step2(uint32_t a, uint32_t b, uint32_t c)
{
	return (rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22)) + ((a & b) ^ (a & c) ^ (b & c));
}

inline void update_w(uint32_t* w, int i, const byte* buffer)
{
	for (int j = 0; j < 16; j++)
	{
		if (i < 16)
		{
			w[j] = ((uint32_t)buffer[0] << 24) | ((uint32_t)buffer[1] << 16) | ((uint32_t)buffer[2] << 8) |
			       ((uint32_t)buffer[3]);
			buffer += 4;
		}
		else
		{
			uint32_t a = w[(j + 1) & 15];
			uint32_t b = w[(j + 14) & 15];
			uint32_t s0 = (rotr(a, 7) ^ rotr(a, 18) ^ (a >> 3));
			uint32_t s1 = (rotr(b, 17) ^ rotr(b, 19) ^ (b >> 10));
			w[j] += w[(j + 9) & 15] + s0 + s1;
		}
	}
}

void SHA256::transform()
{
	uint32_t* state = _state;

	static const uint32_t k[8 * 8] = {
		0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
		0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
		0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
		0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
		0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
		0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
		0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
		0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
	};

	uint32_t a = state[0];
	uint32_t b = state[1];
	uint32_t c = state[2];
	uint32_t d = state[3];
	uint32_t e = state[4];
	uint32_t f = state[5];
	uint32_t g = state[6];
	uint32_t h = state[7];

	uint32_t w[16];

	int i, j;
	for (i = 0; i < 64; i += 16)
	{
		update_w(w, i, _buffer);

		for (j = 0; j < 16; j += 4)
		{
			uint32_t temp;
			temp = h + step1(e, f, g) + k[i + j + 0] + w[j + 0];
			h = temp + d;
			d = temp + step2(a, b, c);
			temp = g + step1(h, e, f) + k[i + j + 1] + w[j + 1];
			g = temp + c;
			c = temp + step2(d, a, b);
			temp = f + step1(g, h, e) + k[i + j + 2] + w[j + 2];
			f = temp + b;
			b = temp + step2(c, d, a);
			temp = e + step1(f, g, h) + k[i + j + 3] + w[j + 3];
			e = temp + a;
			a = temp + step2(b, c, d);
		}
	}

	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;
	state[4] += e;
	state[5] += f;
	state[6] += g;
	state[7] += h;
}

SHA256::SHA256()
{
	_state[0] = 0x6a09e667;
	_state[1] = 0xbb67ae85;
	_state[2] = 0x3c6ef372;
	_state[3] = 0xa54ff53a;
	_state[4] = 0x510e527f;
	_state[5] = 0x9b05688c;
	_state[6] = 0x1f83d9ab;
	_state[7] = 0x5be0cd19;
	_nbits = 0;
	_count = 0;
	_buffer[0] = 0;
}

void SHA256::add(byte b)
{
	_buffer[_count++] = b;
	_nbits += 8;

	if (_count == 64)
	{
		_count = 0;
		transform();
	}
}

void SHA256::add(const void* src, int n)
{
	const byte* bytes = (const byte*)src;

	for (int i = 0; i < n; i++)
	{
		add(bytes[i]);
	}
}

SHA256::Hash SHA256::end()
{
	uint64_t nbits = _nbits;

	add(0x80);

	while (_count != 56)
		add(0);

	for (int i = 7; i >= 0; i--)
	{
		byte b = byte((nbits >> 8 * i) & 0xff);
		add(b);
	}

	Hash hash;

	byte* ptr = (byte*)&hash;

	for (int i = 0; i < 8; i++)
	{
		for (int j = 3; j >= 0; j--)
		{
			*ptr++ = (_state[i] >> j * 8) & 0xff;
		}
	}

	return hash;
}

}
