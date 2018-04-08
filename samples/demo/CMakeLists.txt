set(TARGET demo)

add_executable( ${TARGET} demo.cpp )
target_link_libraries( ${TARGET} asls )

set_target_properties(${TARGET} PROPERTIES FOLDER samples)

