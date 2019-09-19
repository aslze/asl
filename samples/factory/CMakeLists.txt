
if(CMAKE_SIZEOF_VOID_P EQUAL 8 AND CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

add_library( lib SHARED lib.cpp )
target_link_libraries( lib LINK_PUBLIC asls )

add_library( pluginA SHARED pluginA.cpp )
target_link_libraries( pluginA LINK_PRIVATE asls lib )

add_library( pluginB SHARED pluginB.cpp )
target_link_libraries( pluginB LINK_PRIVATE asls lib )

add_executable( fdemo fdemo.cpp )
target_link_libraries( fdemo LINK_PRIVATE asls lib )

add_dependencies(fdemo lib pluginA pluginB)

set_target_properties(fdemo lib pluginA pluginB PROPERTIES FOLDER samples/factory)
