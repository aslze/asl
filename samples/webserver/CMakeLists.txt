set(TARGET webserver)

set(SRC
	webserver.cpp
)

add_executable( ${TARGET} ${SRC} )
target_link_libraries( ${TARGET} asls )
set_target_properties( ${TARGET} PROPERTIES FOLDER samples)


add_custom_command(TARGET ${TARGET}
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/www.ini ${CMAKE_CURRENT_BINARY_DIR}
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/index.html ${CMAKE_CURRENT_BINARY_DIR}
)
