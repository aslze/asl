#include <asl/Uuid.h>

namespace asl {

Uuid::Uuid(const String& s)
{
	memset(_u, 0, 16);
	if (s.length() != 36)
		return;
	for (int i = 0, j = 0; i < 16; i++, j+=2)
	{
		_u[i] = (byte)s.substr(j, 2).hexToInt();
		if (i == 3 || i == 5 || i == 7 || i == 9)
			j++;
	}
}

String Uuid::operator*() const
{
	return String::f("%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
		_u[0], _u[1], _u[2], _u[3], _u[4], _u[5], _u[6], _u[7], _u[8], _u[9], _u[10], _u[11], _u[12], _u[13], _u[14], _u[15]);
}

UuidGenerator::UuidGenerator() : _random1(true, false), _random2(_random1)
{
	int n = 40 + int(inow() % 0x0f);
	for (int i = 0; i < n; i++)
		_random2.get();
}

Uuid UuidGenerator::generate()
{
	const int n = sizeof(Uuid) / sizeof(ULong);
	ULong x[n];
	for(int i = 0; i < n; i++)
		x[i] = _random1.getLong() ^ _random2.getLong();

	Uuid u;
	memcpy(&u, &x, sizeof(Uuid));
	
	u[6] = (u[6] & ~0xf0) | 0x40; // version 4
	u[8] = (u[8] & ~0xc0) | 0x80; // variant 1
	return u;
}

Uuid Uuid::generate()
{
	static UuidGenerator gen;
	return gen.generate();
}

}
