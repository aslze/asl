#include <asl/Vec3.h>
#include <asl/Matrix4.h>
#include <asl/Uuid.h>
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
