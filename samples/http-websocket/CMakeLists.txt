set(TARGET http)

set(SRC
	server.cpp
)

add_executable( ${TARGET} ${SRC} )
target_link_libraries( ${TARGET} asls )
set_target_properties( ${TARGET} PROPERTIES FOLDER samples)


add_custom_command(TARGET ${TARGET}
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/httpws.ini ${CMAKE_CURRENT_BINARY_DIR}
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/httpws ${CMAKE_CURRENT_BINARY_DIR}/httpws
)
