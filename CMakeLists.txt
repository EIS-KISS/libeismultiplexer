cmake_minimum_required(VERSION 3.0)

project(uvosled)

set(SRC_FILES uvosled.c usbshm.c)
set(LIBS -pthread -lusb-1.0 )

add_library(${PROJECT_NAME} SHARED ${SRC_FILES})
target_link_libraries( ${PROJECT_NAME} ${LIBS})
add_definitions("-std=c17 -Wall -O2 -fno-strict-aliasing")

set(CMAKE_INSTALL_PREFIX "/usr")
install(TARGETS ${PROJECT_NAME} DESTINATION lib)
install(FILES ./uvosled.h DESTINATION include)

link_directories(${CMAKE_CURRENT_BINARY_DIR})
set(SRC_FILES_TEST_APP main.c)
set(LIBS_TEST -l${PROJECT_NAME})
add_executable(${PROJECT_NAME}_test ${SRC_FILES_TEST_APP})
add_dependencies(${PROJECT_NAME}_test ${PROJECT_NAME})
target_link_libraries( ${PROJECT_NAME}_test ${LIBS_TEST})
add_definitions("-std=c17 -Wall -O2 -fno-strict-aliasing")

install(TARGETS ${PROJECT_NAME}_test DESTINATION bin)

if(CMAKE_SYSTEM_NAME MATCHES "^Linux")
	set(UDEV_RULES_INSTALL_DIR /lib/udev/rules.d CACHE PATH "install directory for linux udev config")
	install(FILES 90-usbled.rules DESTINATION ${UDEV_RULES_INSTALL_DIR})
endif()
