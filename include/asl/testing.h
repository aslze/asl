// Copyright(c) 1999-2024 aslze
// Licensed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef ASL_TESTING_H
#define ASL_TESTING_H

#include <asl/String.h>

// set env: CTEST_OUTPUT_ON_FAILURE = 1

namespace asl {


template<class T>
inline String asString(const T& x) { return String(x); }

/**
\defgroup Testing Testing
@{
*/

int addTest(const char* name, void (*func)());
/**
Runs the test named `name`.
*/
bool runTest(const char* name);
/**
Runs all tests and returns true if all succeded; prints results to console or appends them to `testResult` on Android
*/
bool runAllTests();
extern asl::String testResult;
struct TestInfo { const char* name; void (*func)(); };
extern TestInfo tests[255];
extern int numTests;
extern bool testFailed;
extern int failedTests;

#ifndef __ANDROID__
#define ASL_PRINT_TEST_RESULT(name, result)
#else
#define ASL_PRINT_TEST_RESULT(name, result) asl::testResult << name << ": " << (result ? "OK\n" : "FAILED\n");
#endif

/** 
Add support for testing in this executable (use only once in the sources for one executable).
@hideinitializer
*/
#define ASL_TEST_ENABLE() \
namespace asl { \
asl::String testResult; \
TestInfo tests[255]; \
int numTests = 0; \
int failedTests = 0;\
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
			if(testFailed) failedTests++;\
			return !testFailed;\
		}\
	}\
	return false;\
}\
bool runAllTests()\
{\
	for (int i = 0; i<asl::numTests; i++) { \
		const char* name = asl::tests[i].name;\
		bool result = asl::runTest(name); \
		printf("Test %s:\n----------------> %s\n\n", name, result? "OK" : "FAILED"); \
		ASL_PRINT_TEST_RESULT(name, result); \
	}\
	return asl::failedTests == 0;\
}\
}

#define ASL_TEST_MAIN() \
int main() \
{ \
	bool ok = asl::runAllTests(); \
	printf("\n%s: %i tests failed of %i\n", ok ? "OK" : "Error", asl::failedTests, asl::numTests); \
	return asl::failedTests; \
}

/** 
Add support for testing in this executable and create a function `main()` that runs all tests.
@hideinitializer
*/
#define ASL_TEST_ENABLE_MAIN() \
ASL_TEST_ENABLE() \
ASL_TEST_MAIN()

/** 
Create a test named Name.
@hideinitializer
*/
#define ASL_TEST(Name) \
void asl_test_##Name();\
int asl_xx_##Name = asl::addTest(#Name, &asl_test_##Name); \
void asl_test_##Name()


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

template<class T>
inline String asString(const Vec3_<T>& x)
{
	return String::f("(%g, %g, %g)", x.x, x.y, x.z);
}
#endif

#ifdef ASL_VECTOR2_H
template<class T>
double distance(const Vec2_<T>& a, const Vec2_<T>& b)
{
	return (a - b).length();
}

template<class T>
inline String asString(const Vec2_<T>& x)
{
	return String::f("(%g, %g)", x.x, x.y);
}
#endif

#ifdef ASL_MATRIX4_H
template<class T>
double distance(const Matrix4_<T>& a, const Matrix4_<T>& b)
{
	Vec3_<T> vx(1,0,0), vy(0,1,0), vz(0,0,1);
	return Vec3_<T>((a*vx - b*vx).length(), (a*vy - b*vy).length(), (a*vz - b*vz).length()).length();
}
#endif

#ifdef ASL_QUATERNION_H
template<class T>
double distance(const Quaternion_<T>& a, const Quaternion_<T>& b)
{
	return min((-a + b).length(), (a + b).length());
}
#endif

#ifdef ASL_VAR_H
template<>
inline String asString(const Var& x) { return x.toString(); }
#endif

#undef ASL_ASSERT

#ifdef __ANDROID__
#define ASL_ASSERT(x) if(!(x)) { asl::testResult << asl::String::f("\n%s: %i\nFailed: '%s'\n\n", __FILE__, __LINE__, #x); }

#define ASL_EXPECT(x, op, y) if(!((x) op (y))) { asl::testResult << asl::String::f("\n%s: %i\n\n* Expected '%s' %s '%s' but it is: %s\n\n", __FILE__, __LINE__, #x, #op, *asl::asString(y), *asl::asString(x)); }
#else

/** 
Check that the argument is true.
@hideinitializer
*/
#define ASL_ASSERT(x) if(!(x)) { printf("\n%s(%i): error: '%s'\n\n", __FILE__, __LINE__, #x); asl::testFailed = true; }

/** 
Check a condition with the given operator and operands.
~~~
ASL_EXPECT(sqrt(x), >=, 0);
~~~
@hideinitializer
*/

#define ASL_EXPECT(x, op, y) if(!((x) op (y))) { printf("\n%s(%i): error: Expected '%s' %s '%s' but it is %s\n\n", __FILE__, __LINE__, #x, #op, *asl::asString(y), *asl::asString(x)); asl::testFailed = true; }

#endif

#define ASL_CHECK(x, op, y) ASL_EXPECT(x, op, y)

#define ASL_APPROX(x, y, d) ASL_EXPECT(asl::distance((x), (y)), <, (d))

/** 
Check that `x` and `y` are approximately equal (within distance `d`).
@hideinitializer
*/
#define ASL_EXPECT_NEAR(x, y, d) ASL_EXPECT(asl::distance((x), (y)), <, (d))

/**@}*/
}

#endif
