
SET(EXE_PATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

ADD_EXECUTABLE( unittests unittests.cpp unittests2.cpp unittests3.cpp)
TARGET_LINK_LIBRARIES( unittests asls )

MACRO(TEST name)
	ADD_TEST( ${name}-test ${EXE_PATH}/unittests ${name})
ENDMACRO()

SET(TESTS
	Array String Var XDL CmdArgs TabularDataFile IniFile
	Factory HashMap Map File StaticSpace Path
	Base64 XML
	Process
	SHA1
	SmartObject
	Date
	AtomicCount
)

FOREACH(T ${TESTS})
	TEST(${T})
ENDFOREACH()
