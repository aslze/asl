
set( SRC
	unicode.cpp
)

add_executable( make_unicode ${SRC} )
target_link_libraries( make_unicode asls )
set_target_properties( make_unicode PROPERTIES FOLDER samples)