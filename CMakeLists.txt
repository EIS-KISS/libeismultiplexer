cmake_minimum_required(VERSION 3.0)

project(eismultiplexer)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUSB REQUIRED libusb-1.0)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "..." FORCE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

set(SRC_FILES eismultiplexer.c usbshm.c)
add_library(${PROJECT_NAME} SHARED ${SRC_FILES})
target_link_libraries(${PROJECT_NAME} ${LIBUSB_LIBRARIES} -pthread)
target_include_directories(${PROJECT_NAME} PUBLIC ${LIBUSB_INCLUDE_DIRS})
add_definitions("-std=c17 -Wall -O2 -fno-strict-aliasing")
install(TARGETS ${PROJECT_NAME} DESTINATION lib)
install(FILES ./eismultiplexer.h DESTINATION include)

link_directories(${CMAKE_CURRENT_BINARY_DIR})
add_executable(${PROJECT_NAME}_test main.c)
add_dependencies(${PROJECT_NAME}_test ${PROJECT_NAME})
target_link_libraries( ${PROJECT_NAME}_test -l${PROJECT_NAME})
add_definitions("-std=c17 -Wall -O2 -fno-strict-aliasing")
install(TARGETS ${PROJECT_NAME}_test DESTINATION bin)

configure_file(pkgconfig/eismuliplexer.pc.in pkgconfig/eismuliplexer.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/pkgconfig/eismuliplexer.pc DESTINATION lib/pkgconfig)

if(CMAKE_SYSTEM_NAME MATCHES "^Linux")
	set(UDEV_RULES_INSTALL_DIR /lib/udev/rules.d CACHE PATH "install directory for linux udev config")
	install(FILES 90-eismultiplexer.rules DESTINATION ${UDEV_RULES_INSTALL_DIR})
endif()
