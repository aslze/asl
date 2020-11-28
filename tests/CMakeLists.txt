
set(EXE_PATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_executable( unittests unittests.cpp unittests2.cpp unittests3.cpp unittests4.cpp)
target_link_libraries( unittests asls )

macro(TEST name)
	add_test( ${name} ${EXE_PATH}/unittests ${name})
endmacro()

set(TESTS
	Array
	String
	Var
	XDL
	CmdArgs
	TabularDataFile
	IniFile
	Factory
	HashMap
	Map
	File
	StaticSpace
	Path
	Base64
	XML
	Process
	SHA1
	SmartObject
	Date
	AtomicCount
	Vec3
	Matrix4
	Uuid
	StreamBuffer
	Function
)

foreach(T ${TESTS})
	TEST(${T})
endforeach()
