#include <asl/Vec3.h>
#include <asl/Matrix4.h>
#include <asl/Uuid.h>
#include <asl/StreamBuffer.h>
#include <stdio.h>

using namespace asl;

#define APPROX(x, y) (fabs(x - y) < 0.001)

void testVec3()
{
	Vec3 a(1, 2.5f, 3), b(1, 0, 0);
	ASL_ASSERT(a*b == 1);

	Vec3d a2 = a, b2 = b;
	Vec3 a3 = a2;
	ASL_ASSERT(a2 + b2 == Vec3d(2, 2.5, 3));
}


void testUuid()
{
	Uuid u1 ("93efe45f-97b8-487f-a1a1-a08838ca3598");
	Uuid u2 ("93efe45F-97b8-487F-A1a1-a08838Ca3598");
	Uuid u3 = Uuid::generate();
	ASL_ASSERT(u1 == u2);
	ASL_ASSERT(*u1 == "93efe45f-97b8-487f-a1a1-a08838ca3598");
	ASL_ASSERT(u3 != u2);
}

void testStreamBuffer()
{
	StreamBuffer b;
	b.setEndian(StreamBuffer::LITTLEENDIAN);
	b << 'a' << 4 << 3.5;

	ASL_ASSERT(b.length() == 13);
	ASL_ASSERT(b[0] == 'a' && b[1] == 0x04 && b[2] == 0 && b[3] == 0 && b[4] == 0)

	StreamBufferReader c(b.ptr(), b.length());
	char a;
	int x;
	double y;
	c >> a >> x >> y;

	ASL_ASSERT(a == 'a' && x == 4 && y == 3.5);

	StreamBuffer b2;
	b2.setEndian(StreamBuffer::BIGENDIAN);
	b2 << 'a' << 4 << 3.5;

	a = ' ';
	x = 0;
	y = 0;

	ASL_ASSERT(b2.length() == 13);
	ASL_ASSERT(b2[0] == 'a' && b2[1] == 0 && b2[2] == 0 && b2[3] == 0 && b2[4] == 4)

	StreamBufferReader c2(b2.ptr(), b2.length(), StreamBufferReader::BIGENDIAN);
	c2 >> a >> x >> y;

	ASL_ASSERT(a == 'a' && x == 4 && y == 3.5);
}