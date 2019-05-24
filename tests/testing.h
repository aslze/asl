#ifndef ASL_TESTING
#define ASL_TESTING

namespace asl {

int addTest(const char* name, void (*func)());
bool runTest(const char* name);

#define ASL_ENABLE_TESTING() \
namespace asl { \
struct TestInfo { const char* name; void (*func)(); }; \
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

}

#endif


