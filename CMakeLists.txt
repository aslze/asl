cmake_minimum_required(VERSION 2.8.12...3.10)

project(asl)

set(ASL_VERSION 1.11.12)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_MACOSX_RPATH ON)
set(CMAKE_DEBUG_POSTFIX d)

if(POLICY CMP0077)
	cmake_policy(SET CMP0077 NEW)
endif()

if(NOT MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
	if((CMAKE_COMPILER_IS_GNUCXX AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER "4.4") OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang" )
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
	endif()
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(CMAKE_SOURCE_DIR STREQUAL ${CMAKE_CURRENT_SOURCE_DIR} AND NOT ANDROID)
	set(ASL_BUILD_SHARED_HINT ON)
else()
	set(ASL_BUILD_SHARED_HINT OFF)
endif()

option(ASL_USE_LOCAL8BIT "Treat char strings as local 8 bit instead of UTF8")
option(ASL_TLS "TLS Sockets")
option(ASL_BUILD_STATIC "Build static library" ON)
option(ASL_BUILD_SHARED "Build shared library" ${ASL_BUILD_SHARED_HINT})
option(ASL_IPV6 "Expect also IPv6 when looking up DNS names")
option(ASL_SAMPLES "Build samples")
option(ASL_TESTS "Build tests")
option(ASL_SOCKET_LOCAL "Support Unix sockets on Windows")

if(ASL_TLS)
	if(TARGET MbedTLS::mbedtls)
		set(mbedTLS_LIB MbedTLS::mbedtls)
		set(mbedTLSx509_LIB MbedTLS::mbedx509)
		set(mbedTLScrypto_LIB MbedTLS::mbedcrypto)
	else()
		find_path(mbedTLS_DIR "include/mbedtls")
		if(mbedTLS_DIR STREQUAL mbedTLS_DIR-NOTFOUND )
			message(FATAL_ERROR "TLS needs the directory of mbedTLS" )
		endif()
		include_directories(${mbedTLS_DIR}/include)
		find_library(mbedTLS_LIB mbedtls ${mbedTLS_DIR}/lib)
		find_library(mbedTLSx509_LIB mbedx509 ${mbedTLS_DIR}/lib)
		find_library(mbedTLScrypto_LIB mbedcrypto ${mbedTLS_DIR}/lib)
	endif()
endif()

set(TARGETS "")

add_subdirectory(src)

if(ASL_SAMPLES)
	add_subdirectory(samples)
endif()

if(ASL_TESTS)
	enable_testing()
	add_subdirectory(tests)
endif()

file(WRITE "${PROJECT_BINARY_DIR}/ASLConfigVersion.cmake"
	"set(PACKAGE_VERSION ${ASL_VERSION})\n"
	"if(NOT CMAKE_GENERATOR STREQUAL \"${CMAKE_GENERATOR}\" OR (DEFINED CMAKE_GENERATOR_PLATFORM AND NOT CMAKE_GENERATOR_PLATFORM STREQUAL \"${CMAKE_GENERATOR_PLATFORM}\"))\n"
	"set(PACKAGE_VERSION_UNSUITABLE TRUE)\nendif()\n"
	"if(PACKAGE_FIND_VERSION VERSION_LESS PACKAGE_VERSION OR PACKAGE_FIND_VERSION VERSION_EQUAL PACKAGE_VERSION)\nset(PACKAGE_VERSION_COMPATIBLE TRUE)\nendif()\n"
)

export(TARGETS ${TARGETS} FILE ASLConfig.cmake)
install(EXPORT asl DESTINATION cmake FILE ASLConfig.cmake)

if(CMAKE_SOURCE_DIR STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
	export( PACKAGE ASL )
	find_package(Doxygen)
	if(DOXYGEN_FOUND AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/doc)
		set(ASL_DOC_DIR "${PROJECT_BINARY_DIR}/doc" CACHE PATH "Directory for documentation")
		configure_file(doc/doxy.ini doxy2.ini)
		add_custom_target(DOC
			${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/doxy2.ini
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/doc
			COMMENT "Generating API documentation with Doxygen" VERBATIM
		)
	endif()
endif()
