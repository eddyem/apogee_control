cmake_minimum_required(VERSION 2.4.7)
set(CMAKE_COLOR_MAKEFILE ON)

set(SRC ${CMAKE_SOURCE_DIR}/src)
set(LIBSRC ${SRC}/libapogee.cpp)
set(BIN_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/bin")
set(INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/include")

if(DEFINED EBUG)
	add_definitions(-DEBUG)
endif()
if(DEFINED VERBOSE OR DEFINED VERB OR DEFINED V)
	set(CMAKE_VERBOSE_MAKEFILE ON)
endif()

set(APOGEE_VERSION "3.0")
set(APOGEE_SOVERSION "3")

find_path(APOGEE_LIB_INCLUDE_DIR NAMES Alta.h
			PATH_SUFFIXES libapogee-3.0/apogee
		)

find_library(APOGEE_LIB NAMES apogee apogeeu
				PATHS   /lib
				/usr/lib
				/usr/local/lib
				/opt/lib
			)

if(${APOGEE_LIB} STREQUAL "APOGEE_LIB-NOTFOUND"
	OR ${APOGEE_LIB_INCLUDE_DIR} STREQUAL "APOGEE_LIB_INCLUDE_DIR-NOTFOUND")
	message(FATAL_ERROR "You need first to install libapogee")
endif()

add_definitions("-O3 -g -fpic -Wall -Werror -W -I${APOGEE_LIB_INCLUDE_DIR}")

add_library(apogeeu SHARED ${LIBSRC})

set_target_properties(apogeeu PROPERTIES VERSION ${APOGEE_VERSION} SOVERSION ${APOGEE_SOVERSION})

target_link_libraries(apogeeu ${APOGEE_LIB})

add_executable(test ${SRC}/test.c)
target_link_libraries(test apogeeu)

add_executable(test_wheel ${SRC}/test_wheel.c)
target_link_libraries(test_wheel apogeeu)

install(TARGETS apogeeu LIBRARY DESTINATION lib${LIB_POSTFIX})
install(FILES ${SRC}/60-apogee.rules DESTINATION /etc/udev/rules.d/)
install(FILES ${SRC}/libapogee.h DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

configure_file(${SRC}/apogeec.pc.in ${SRC}/apogeec.pc @ONLY)
install(FILES ${SRC}/apogeec.pc DESTINATION /usr/share/pkgconfig/ COMPONENT Devel)
