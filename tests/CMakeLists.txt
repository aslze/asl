add_executable( unittests unittests.cpp unittests2.cpp unittests3.cpp unittests4.cpp)
target_link_libraries( unittests asls )

macro(TEST name)
	add_test(${name} "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/unittests" ${name})
endmacro()

set(TESTS
	Array
	Array2
	String
	Var
	JSON
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
	Vec2
	Vec3
	Matrix4
	Uuid
	StreamBuffer
	Function
	Matrix
	URL
)

foreach(T ${TESTS})
	TEST(${T})
endforeach()
