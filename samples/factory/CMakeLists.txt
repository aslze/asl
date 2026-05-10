
if(CMAKE_SIZEOF_VOID_P EQUAL 8 AND CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

add_library(alib SHARED lib.cpp)
target_link_libraries(alib LINK_PUBLIC asls)

add_library(pluginA SHARED pluginA.cpp)
target_link_libraries( pluginA LINK_PRIVATE asls alib)

add_library(pluginB SHARED pluginB.cpp)
target_link_libraries(pluginB LINK_PRIVATE asls alib)

add_executable(fdemo fdemo.cpp)
target_link_libraries(fdemo LINK_PRIVATE asls alib)

set_target_properties(fdemo alib pluginA pluginB PROPERTIES FOLDER samples/factory)

add_custom_command(TARGET pluginA POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:pluginA> $<TARGET_FILE_DIR:fdemo>)
add_custom_command(TARGET pluginB POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:pluginB> $<TARGET_FILE_DIR:fdemo>)
