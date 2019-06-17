// Copyright(c) 1999-2019 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_TESTING_H
#define ASL_TESTING_H

#include <asl/String.h>

// set env: CTEST_OUTPUT_ON_FAILURE = 1

namespace asl {

int addTest(const char* name, void (*func)());
bool runTest(const char* name);
extern asl::String testResult;
struct TestInfo { const char* name; void (*func)(); };
extern TestInfo tests[255];
extern int numTests;
extern bool testFailed;

#define ASL_ENABLE_TESTING() \
namespace asl { \
asl::String testResult; \
TestInfo tests[255]; \
int numTests = 0; \
bool testFailed = false;\
int addTest(const char* name, void (*func)()) \
{ \
	TestInfo info = {name, func}; \
	tests[numTests++] = info; \
	return 0;\
} \
bool runTest(const char* name)\
{\
	for (int i = 0; i<numTests; i++)\
	{\
		if (strcmp(tests[i].name, name) == 0)\
		{\
			tests[i].func();\
			return true;\
		}\
	}\
	return false;\
}\
}

#define ASL_TEST(Name) \
void asl_test##Name();\
int asl_xx##Name = asl::addTest(#Name, &asl_test##Name); \
void asl_test##Name()


template<class T>
double distance(const T& a, const T& b)
{
	return fabs(a - b);
}

#ifdef ASL_VECTOR3_H
template<class T>
double distance(const Vec3_<T>& a, const Vec3_<T>& b)
{
	return (a - b).length();
}
#endif

#ifdef ASL_MATRIX4_H
template<class T>
double distance(const Matrix4_<T>& a, const Matrix4_<T>& b)
{
	Vec3_<T> vx(1,0,0), vy(0,1,0), vz(0,0,1);
	return Vec3_<T>(!(a*vx - b*vx), !(a*vy - b*vy), !(a*vz - b*vz)).length();
}
#endif

#ifdef ASL_QUATERNION_H
template<class T>
double distance(const Quaternion_<T>& a, const Quaternion_<T>& b)
{
	return min((-a + b).length(), (a + b).length());
}
#endif


#undef ASL_ASSERT

#ifdef __ANDROID__
#define ASL_ASSERT(x) if(!(x)) { testResult << String(0, "\n%s: %i\nFailed: '%s'\n\n", __FILE__, __LINE__, #x); }
#else
#define ASL_ASSERT(x) if(!(x)) { printf("\n%s: %i\n\n* Failed: '%s'\n\n", __FILE__, __LINE__, #x); testFailed = true;}
#endif

#define ASL_APPROX(x, y, d) ASL_ASSERT(distance((x), (y)) < (d))

}

#endif


