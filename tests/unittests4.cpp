#include <asl/Vec3.h>
#include <asl/Matrix4.h>
#include <asl/Quaternion.h>
#include <asl/Uuid.h>
#include <asl/StreamBuffer.h>
#include <stdio.h>
#include "testing.h"

using namespace asl;

#define EPS 1e-5
#define EPSf 1e-5f


ASL_TEST(Vec3)
{
	Vec3 a(1, 2.5f, 3), b(1, 0, 0);
	ASL_APPROX(a*b, 1);

	Vec3d a2 = a, b2 = b;
	Vec3 a3 = a2;
	ASL_ASSERT(!((a2 + b2) - Vec3d(2, 2.5, 3)) < EPS);
}

ASL_TEST(Matrix4)
{
	Matrix4 m1 = Matrix4::rotateAxis(Vec3(1, 0, 0), (float)PI / 2);
	Matrix4 m2 = Matrix4::rotateX((float)PI / 2);
	Vec3 a(0, 1, 0);
	ASL_ASSERT(!(m1 * a - Vec3(0, 0, 1)) < EPSf);
	ASL_ASSERT(!(m2 * a - Vec3(0, 0, 1)) < EPSf);
}


ASL_TEST(Uuid)
{
	Uuid u1 ("93efe45f-97b8-487f-a1a1-a08838ca3598");
	Uuid u2 ("93efe45F-97b8-487F-A1a1-a08838Ca3598");
	Uuid u3 = Uuid::generate();
	ASL_ASSERT(u1 == u2);
	ASL_ASSERT(*u1 == "93efe45f-97b8-487f-a1a1-a08838ca3598");
	ASL_ASSERT(u3 != u2);
}

ASL_TEST(StreamBuffer)
{
	StreamBuffer b;
	b.setEndian(ENDIAN_LITTLE);
	b << 'a' << 4 << 3.5;

	ASL_ASSERT(b.length() == 13);
	ASL_ASSERT(b[0] == 'a' && b[1] == 0x04 && b[2] == 0 && b[3] == 0 && b[4] == 0)

	StreamBufferReader c(b.ptr(), b.length());
	signed char a;
	int x;
	double y;
	c >> a >> x >> y;

	ASL_ASSERT(a == 'a' && x == 4 && y == 3.5);

	StreamBuffer b2;
	b2.setEndian(ENDIAN_BIG);
	b2 << 'a' << 4 << 3.5;

	a = ' ';
	x = 0;
	y = 0;

	ASL_ASSERT(b2.length() == 13);
	ASL_ASSERT(b2[0] == 'a' && b2[1] == 0 && b2[2] == 0 && b2[3] == 0 && b2[4] == 4)

	StreamBufferReader c2(b2.ptr(), b2.length(), ENDIAN_BIG);
	c2 >> a >> x >> y;

	ASL_ASSERT(a == 'a' && x == 4 && y == 3.5);
}
