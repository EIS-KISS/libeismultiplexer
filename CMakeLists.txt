cmake_minimum_required(VERSION 3.20)

project(eismultiplexer C)

find_package(PkgConfig REQUIRED)
find_package(Doxygen)
pkg_check_modules(LIBUSB REQUIRED libusb-1.0)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
include(GitVersion)
get_version_from_git()
message("Building version ${PROJECT_VERSION}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

add_compile_definitions(VERSION_MAJOR=${CMAKE_PROJECT_VERSION_MAJOR})
add_compile_definitions(VERSION_MINOR=${CMAKE_PROJECT_VERSION_MINOR})
add_compile_definitions(VERSION_PATCH=${CMAKE_PROJECT_VERSION_PATCH})

option(UDEV_RULES "install udev files" ON)

set(SRC_FILES eismultiplexer.c usbshm.c)

if(CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
	message(FATAL_ERROR "Windows builds have to be cross compiled on UNIX")
endif()

message("Platform " ${CMAKE_SYSTEM_NAME})
if(WIN32)
	message("Building for Windows")
	file(GLOB_RECURSE ARGP_SRC ${PROJECT_SOURCE_DIR}/argp/*.*)
	set(SRC_FILES ${SRC_FILES} ${ARGP_SRC})
	include_directories(./argp)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/scripts/release-win.sh ${CMAKE_CURRENT_BINARY_DIR}/release.sh @ONLY)
	add_custom_target(package
		COMMAND ${CMAKE_CURRENT_BINARY_DIR}/release.sh
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "Createing release archive"
		VERBATIM)
else()
	message("Building for UNIX")
	if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
		set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "..." FORCE)
	endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
endif(WIN32)

add_library(${PROJECT_NAME} SHARED ${SRC_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC ${LIBUSB_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${LIBUSB_LIBRARIES})
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -O2 -fno-strict-aliasing)
install(TARGETS ${PROJECT_NAME} DESTINATION lib)
install(FILES ./eismultiplexer.h DESTINATION include)

add_library(${PROJECT_NAME}_static STATIC ${SRC_FILES})
target_include_directories(${PROJECT_NAME}_static PUBLIC ${LIBUSB_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME}_static ${LIBUSB_LIBRARIES})
target_compile_options(${PROJECT_NAME}_static PRIVATE -Wall -O2 -fno-strict-aliasing)
install(TARGETS ${PROJECT_NAME}_static DESTINATION lib)

link_directories(${CMAKE_CURRENT_BINARY_DIR})
add_executable(${PROJECT_NAME}_cli main.c)
add_dependencies(${PROJECT_NAME}_cli ${PROJECT_NAME}_static)
target_link_libraries( ${PROJECT_NAME}_cli ${PROJECT_NAME}_static)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -O2 -fno-strict-aliasing)
install(TARGETS ${PROJECT_NAME}_cli DESTINATION bin)

configure_file(pkgconfig/eismuliplexer.pc.in pkgconfig/eismuliplexer.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/pkgconfig/eismuliplexer.pc DESTINATION lib/pkgconfig)

if (DOXYGEN_FOUND)
	set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/doc/libeismulitplexer.doxygen.in)
	set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/doc/libeismulitplexer.doxygen)
	configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
	message("Doxygen build started")

	add_custom_target(doc
		COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "Generating API documentation with Doxygen"
		VERBATIM )
else (DOXYGEN_FOUND)
  message("Doxygen needs to be installed to generate the doxygen documentation")
endif (DOXYGEN_FOUND)

if(CMAKE_SYSTEM_NAME MATCHES "^Linux" AND UDEV_RULES)
	set(UDEV_RULES_INSTALL_DIR /lib/udev/rules.d CACHE PATH "install directory for linux udev config")
	install(FILES 90-eismultiplexer.rules DESTINATION ${UDEV_RULES_INSTALL_DIR})
endif()
