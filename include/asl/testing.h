// Copyright(c) 1999-2019 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_TESTING_H
#define ASL_TESTING_H

#include <asl/String.h>

// set env: CTEST_OUTPUT_ON_FAILURE = 1

namespace asl {

/**
\defgroup Testing Testing
@{
*/

int addTest(const char* name, void (*func)());
/**
Runs the test named `name`.
*/
bool runTest(const char* name);
extern asl::String testResult;
struct TestInfo { const char* name; void (*func)(); };
extern TestInfo tests[255];
extern int numTests;
extern bool testFailed;

/** 
Add support for testing in this executable (use only once in the sources for one executable).
@hideinitializer
*/
#define ASL_TEST_ENABLE() \
namespace asl { \
asl::String testResult; \
TestInfo tests[255]; \
int numTests = 0; \
bool testsFailed = false;\
bool testFailed = false;\
int addTest(const char* name, void (*func)()) \
{ \
	TestInfo info = {name, func}; \
	tests[numTests++] = info; \
	return 0;\
} \
bool runTest(const char* name)\
{\
	testFailed = false;\
	for (int i = 0; i<numTests; i++)\
	{\
		if (strcmp(tests[i].name, name) == 0)\
		{\
			tests[i].func();\
			if(testFailed) testsFailed = true;\
			return !testFailed;\
		}\
	}\
	return false;\
}\
}

#define ASL_TEST_MAIN() \
int main() \
{ \
	for (int i = 0; i<asl::numTests; i++) { \
		printf("Test %s:\n", asl::tests[i].name); \
		printf("-> %s\n\n", asl::runTest(asl::tests[i].name)? "OK" : "FAILED"); \
	} \
	return asl::testFailed ? 1 : 0; \
}

/** 
Add support for testing in this executable and create a function `main()` that runs all tests.
@hideinitializer
*/
#define ASL_TEST_ENABLE_MAIN() \
	ASL_TEST_ENABLE(); \
	ASL_TEST_MAIN();

/** 
Create a test named Name.
@hideinitializer
*/
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

#define ASL_CHECK(x, op, y) if(!((x) op (y))) { testResult << String(0, "\n%s: %i\n\n* Expected '%s' %s '%s' but it is: %s\n\n", __FILE__, __LINE__, #x, #op, #y, *String(x)); }
#else

/** 
Check that the argument is true.
@hideinitializer
*/
#define ASL_ASSERT(x) if(!(x)) { printf("\n%s: %i\n\n* Failed: '%s'\n\n", __FILE__, __LINE__, #x); testFailed = true;}

/** 
Check a condition with the given operator and operands.
~~~
ASL_CHECK(sqrt(x), >=, 0);
~~~
@hideinitializer
*/
#define ASL_CHECK(x, op, y) if(!((x) op (y))) { printf("\n%s: %i\n\n* Expected '%s' %s '%s' but it is: %s\n\n", __FILE__, __LINE__, #x, #op, #y, *String(x)); testFailed = true;}
#endif

/** 
Check that `x` and `y` are approximately equal (within distance `d`).
@hideinitializer
*/
#define ASL_APPROX(x, y, d) ASL_CHECK(distance((x), (y)), <, (d))

/**@}*/
}

#endif


