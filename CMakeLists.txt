cmake_minimum_required(VERSION 3.23)
project(ubuntu_frame_test)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig)
pkg_check_modules(GTEST QUIET gtest)
pkg_check_modules(GTEST_MAIN QUIET gtest_main)
pkg_check_modules(GMOCK QUIET gmock)
find_path(GMOCK_INCLUDE_DIR gmock/gmock.h PATHS /usr/src/googletest/googlemock/include)

find_program(XVFB_RUN_EXECUTABLE xvfb-run)

include_directories(${GMOCK_INCLUDE_DIR} ${GTEST_INCLUDE_DIR})

message(STATUS "${GTEST_INCLUDE_DIR}" )
message(STATUS "${GMOCK_INCLUDE_DIR}" )

add_executable(ubuntu_frame_test
    ubuntu-frame-test.cpp
)

target_link_libraries(ubuntu_frame_test
    PRIVATE
        X11
        ${GTEST_LIBRARIES}
        ${GTEST_MAIN_LIBRARIES}
        ${GMOCK_LIBRARY}
)
