cmake_minimum_required(VERSION 3.0)
set(PROJ apogee_control)
set(VERSION "0.2.0")
set(CMAKE_COLOR_MAKEFILE ON)
#set(CMAKE_VERBOSE_MAKEFILE ON)
if(DEFINED EBUG)
	add_definitions(-DEBUG)
endif()


project(${PROJ} VERSION ${PROJ_VERSION} LANGUAGES C CXX)

#if(DEFINED CMAKE_INSTALL_PREFIX AND CMAKE_INSTALL_PREFIX MATCHES "/usr/local")
#	set(CMAKE_INSTALL_PREFIX "/usr")
#endif()

if(NOT DEFINED LOCALEDIR)
	set(LOCALEDIR ${CMAKE_INSTALL_PREFIX}/share/locale)
endif()
set(SOURCES defhdrs.c takepic.c usage.c camtools.c am.c macros.c)
if(DEFINED USE_BTA AND USE_BTA STREQUAL "yes")
	set(SOURCES ${SOURCES} bta_print.c)
	add_definitions(-DUSE_BTA)
endif()
if(DEFINED TELALT)
	add_definitions(-DTELALT=${TELALT})
endif()
if(DEFINED TELLONG)
	add_definitions(-DTELLONG=${TELLONG})
endif()
if(DEFINED TELLAT)
	add_definitions(-DTELLAT=${TELLAT})
endif()
if(DEFINED TELFOCUS)
	add_definitions(-DTELFOCUS=${TELFOCUS})
endif()
if(NOT DEFINED PROCESSOR_COUNT)
  set(PROCESSOR_COUNT 2) # by default 2 cores
  set(cpuinfo_file "/proc/cpuinfo")
  if(EXISTS "${cpuinfo_file}")
    file(STRINGS "${cpuinfo_file}" procs REGEX "^processor.: [0-9]+$")
    list(LENGTH procs PROCESSOR_COUNT)
  endif()
endif()
add_definitions(-DTHREAD_NUMBER=${PROCESSOR_COUNT})
message("In multithreaded operations will use ${PROCESSOR_COUNT} threads")
set(LCPATH ${CMAKE_CURRENT_SOURCE_DIR}/locale/ru)
set(CFLAGS -O2 -Wextra -Wall -Werror -W -std=gnu99)
set(PO_FILE ${LCPATH}/messages.po)
set(MO_FILE ${LCPATH}/LC_MESSAGES/${PROJ}.mo)
set(RU_FILE ${LCPATH}/ru.po)
find_package(PkgConfig REQUIRED)

# find CFITSIO
SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})
FIND_PACKAGE(CFITSIO REQUIRED)

set(MODULES apogeec>=1.71 libusb>=0.1.10)
if(DEFINED USE_BTA AND USE_BTA STREQUAL "yes")
	set(MODULES ${MODULES} sla)
endif()
if(DEFINED USE_PNG AND USE_PNG STREQUAL "yes")
	set(MODULES ${MODULES} libpng>=1.2)
	add_definitions(-DUSEPNG)
endif()
pkg_check_modules(${PROJ} REQUIRED ${MODULES})
if(DEFINED USE_RAW AND USE_RAW STREQUAL "yes")
	add_definitions(-DUSERAW)
endif()

find_package(OpenMP)
if(OPENMP_FOUND)
	add_definitions(-DOMP)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

add_executable(${PROJ} ${SOURCES} ${PO_FILE} ${MO_FILE})
include_directories(${${PROJ}_INCLUDE_DIRS} ${CFITSIO_INCLUDE_DIR} image_view_module)
link_directories(${${PROJ}_LIBRARY_DIRS})
add_definitions(${CFLAGS} -DLOCALEDIR=\"${LOCALEDIR}\" -DPACKAGE_VERSION=\"${VERSION}\"
	-DGETTEXT_PACKAGE=\"${PROJ}\" -DPROJNAME=\"${PROJ}\")

if(DEFINED USE_IMAGEVIEW AND USE_IMAGEVIEW STREQUAL "yes")
	add_subdirectory(image_view_module)
endif()
if(IMAGEVIEW_FOUND)
	message("Found OpenGL. Will use ${IMLIB} to show data")
	find_package(X11 REQUIRED)
	target_link_libraries(${PROJ} ${IMLIB} ${X11_LIBRARIES} ${${PROJ}_LIBRARIES} ${CFITSIO_LIBRARIES} -lm)
	add_definitions(-DIMAGEVIEW)
else()
	message("Will compile without image view module")
	target_link_libraries(${PROJ} ${${PROJ}_LIBRARIES} ${CFITSIO_LIBRARIES} -lm)
endif()

# Installation of the program
INSTALL(FILES ${MO_FILE} DESTINATION "share/locale/ru/LC_MESSAGES")
INSTALL(TARGETS ${PROJ} DESTINATION "bin")

if(DEFINED EBUG)
	message("Don't forget to do 'msgcat' ru.po from image view with this ru.po")
	find_package(Gettext REQUIRED)
	find_program(GETTEXT_XGETTEXT_EXECUTABLE xgettext)
	if(NOT GETTEXT_XGETTEXT_EXECUTABLE OR NOT GETTEXT_MSGFMT_EXECUTABLE)
		message(FATAL_ERROR "xgettext not found")
	endif()
	file(MAKE_DIRECTORY ${LCPATH})
	file(MAKE_DIRECTORY ${LCPATH}/LC_MESSAGES)
	add_custom_command(
		OUTPUT ${PO_FILE}
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		COMMAND ${GETTEXT_XGETTEXT_EXECUTABLE} --from-code=utf-8 ${SOURCES} -c -k_ -kN_ -o ${PO_FILE}
		COMMAND sed -i 's/charset=.*\\\\n/charset=koi8-r\\\\n/' ${PO_FILE}
		COMMAND enconv ${PO_FILE}
		DEPENDS ${SOURCES}
	)
	# we need this to prewent ru.po from deleting by make clean
	add_custom_target(
		RU_FILE
		COMMAND [ -f ${RU_FILE} ] && ${GETTEXT_MSGMERGE_EXECUTABLE} -Uis ${RU_FILE} ${PO_FILE} || cp ${PO_FILE} ${RU_FILE}
		DEPENDS ${PO_FILE} ${SOURCES}
	)
	add_custom_target(
		MO_FILE
		COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} ${RU_FILE} -o ${MO_FILE}
		DEPENDS ${RU_FILE} ${SOURCES}
	)
	add_custom_command(
		OUTPUT ${MO_FILE}
		COMMAND make RU_FILE && make MO_FILE
		DEPENDS ${RU_FILE} ${SOURCES}
	)
endif()

