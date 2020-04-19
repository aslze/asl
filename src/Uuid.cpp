#include <asl/Uuid.h>

namespace asl {

Uuid::Uuid(const String& s)
{
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
	return String(36, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
		_u[0], _u[1], _u[2], _u[3], _u[4], _u[5], _u[6], _u[7], _u[8], _u[9], _u[10], _u[11], _u[12], _u[13], _u[14], _u[15]);
}

UuidGenerator::UuidGenerator() : _random(true, false)
{
}

Uuid UuidGenerator::generate()
{
	Uuid u;
	for(int i=0; i<16; i++)
		u[i] = (byte)(_random.get() & 0xff);

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
