cmake_minimum_required(VERSION 3.20)

project(cppwindow_subproject_consumer LANGUAGES CXX)

include(FetchContent)

option(
    CPPWINDOW_SUBPROJECT_CONSUMER_PROVIDE_GLFW
    "Provide the glfw target before adding CppWindow"
    OFF)

if(CPPWINDOW_SUBPROJECT_CONSUMER_PROVIDE_GLFW)
    FetchContent_Declare(
        cppwindow_test_glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG        3.4
    )
    set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "")
    set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "")
    set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "")
    set(GLFW_INSTALL OFF CACHE INTERNAL "")
    FetchContent_MakeAvailable(cppwindow_test_glfw)

    if(NOT TARGET glfw)
        message(FATAL_ERROR "cppwindow_subproject_consumer: expected provided glfw target")
    endif()
endif()

set(CPPWINDOW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CPPWINDOW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(CPPWINDOW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(CPPWINDOW_BUILD_IMGUI OFF CACHE BOOL "" FORCE)
set(CPPWINDOW_INSTALL OFF CACHE BOOL "" FORCE)

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../.." cppwindow-build)

if(NOT TARGET cppwindow::cppwindow)
    message(FATAL_ERROR "cppwindow_subproject_consumer: missing cppwindow::cppwindow target")
endif()

add_executable(cppwindow_subproject_consumer main.cpp)
target_link_libraries(cppwindow_subproject_consumer
    PRIVATE
        cppwindow::cppwindow
)
target_compile_features(cppwindow_subproject_consumer
    PRIVATE
        cxx_std_20
)

enable_testing()
add_test(NAME cppwindow_subproject_consumer COMMAND cppwindow_subproject_consumer)
