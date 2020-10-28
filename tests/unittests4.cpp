#include <asl/Vec3.h>
#include <asl/Matrix4.h>
#include <asl/Quaternion.h>
#include <asl/Uuid.h>
#include <asl/StreamBuffer.h>
#include <stdio.h>
#include <asl/testing.h>

using namespace asl;

#define EPS 1e-6
#define EPSf 1e-5f


ASL_TEST(Vec3)
{
	Vec3 a(1, 2.5f, 3), b(1, 0, 0);
	ASL_APPROX(a*b, 1.0f, EPS);

	Vec3d a2 = a, b2 = b;
	Vec3 a3 = a2;
	ASL_APPROX(a2 + b2, Vec3d(2, 2.5, 3), EPS);
}

ASL_TEST(Matrix4)
{
	Matrix4d m1 = Matrix4d::rotate(Vec3d(1, 0, 0), PI / 2);
	Matrix4d m2 = Matrix4d::rotateX(PI / 2);
	Vec3d a(0, 1, 0);
	ASL_APPROX(m1 * a, Vec3d(0, 0, 1), EPS);
	ASL_APPROX(m2 * a, Vec3d(0, 0, 1), EPS);

	Quaterniond q1 = Quaterniond::fromAxisAngle(Vec3d(1, 0.5f, -1.25f), 0.25f);
	Matrix4d mrot = Matrix4d::rotate(Vec3d(1, 0.5f, -1.25f), 0.25f);
	Quaterniond q2 = mrot.rotation();

	ASL_APPROX(q1, q2, EPS);
	ASL_APPROX(q1.matrix(), mrot, EPS);
	ASL_APPROX(q1.matrix(), q2.matrix(), EPS);

	for (double x = -1; x <= 1; x+=0.25)
		for (double y = -1; y <= 1; y += 0.25)
			for (double z = -1; z <= 1; z += 0.25)
				for (double a = -2*PI; a <= 2*PI; a += PI / 8)
				{
					if (x == 0 && y == 0 && z == 0)
						continue;
					Quaterniond q1 = Quaterniond::fromAxisAngle(Vec3d(x, y, z), a);
					Matrix4d m = q1.matrix();
					Quaterniond q2 = m.rotation();
					ASL_APPROX(q1, q2, EPS);
					ASL_APPROX(q1.matrix(), q2.matrix(), EPS);
				}

	Matrix4d mi = m1.inverse();
	ASL_APPROX(mi * m1, Matrix4d::identity(), EPS);
	ASL_APPROX(mi.inverse(), m1, EPS);
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
	b.setEndian(StreamBuffer::LITTLEENDIAN);
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
