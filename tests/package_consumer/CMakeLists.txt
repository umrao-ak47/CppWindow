cmake_minimum_required(VERSION 3.20)

project(cppwindow_package_consumer LANGUAGES CXX)

file(READ "${CMAKE_CURRENT_LIST_DIR}/../../VERSION" CPPWINDOW_VERSION)
string(STRIP "${CPPWINDOW_VERSION}" CPPWINDOW_VERSION)

find_package(cppwindow ${CPPWINDOW_VERSION} CONFIG REQUIRED)

add_executable(cppwindow_package_consumer main.cpp)

target_link_libraries(cppwindow_package_consumer
    PRIVATE
        cppwindow::cppwindow
)

target_compile_features(cppwindow_package_consumer
    PRIVATE
        cxx_std_20
)

enable_testing()
add_test(NAME cppwindow_package_consumer COMMAND cppwindow_package_consumer)
