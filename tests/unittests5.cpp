#include <asl/Vec3.h>
#include <asl/Matrix4.h>
#include <asl/String.h>
#include <asl/std.h>
#include <asl/testing.h>
#include <stdio.h>

using namespace asl;

#ifndef ASL_NO_CAST

ASL_TEST(Cast)
{
	String s1 = "Luke";
	std::string ss1 = s1;
	ASL_ASSERT(ss1 == "Luke");

	String s2 = ss1;
	ASL_ASSERT(s2 == "Luke");

	s1 += "!";
	ss1 = cast(s1);
	ASL_ASSERT(ss1 == "Luke!");

	s2 = ss1;
	ASL_ASSERT(s2 == "Luke!");

	Array<int> a1;
	a1 << 1 << 2;
	std::vector<int> v = a1;
	ASL_ASSERT(v.size() == 2 && v[0] == 1 && v[1] == 2);

	Array<int> a2 = v;
	ASL_ASSERT(a2.length() == 2 && a2[0] == 1 && a2[1] == 2);

	a2 << 3;
	v = a2;
	ASL_ASSERT(v.size() == 3 && v[0] == 1 && v[1] == 2 && v[2] == 3);

	a1 = v;
	ASL_ASSERT(a1.length() == 3 && a1[0] == 1 && a1[1] == 2 && a1[2] == 3);
}

#endif
