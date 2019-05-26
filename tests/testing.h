#ifndef ASL_TESTING_H
#define ASL_TESTING_H

#include <asl/String.h>

namespace asl {

int addTest(const char* name, void (*func)());
bool runTest(const char* name);
extern asl::String testResult;
struct TestInfo { const char* name; void (*func)(); };
extern TestInfo tests[255];
extern int numTests;

#define ASL_ENABLE_TESTING() \
namespace asl { \
asl::String testResult; \
TestInfo tests[255]; \
int numTests = 0; \
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

#define ASL_APPROX(x, y) (fabs((x) - (y)) < 0.0001)

#ifdef __ANDROID__
#undef ASL_ASSERT
#define ASL_ASSERT(x) if(!(x)) { testResult << String(0, "\n%s: %i\nFailed: '%s'\n\n", __FILE__, __LINE__, #x); }
#endif

}

#endif


