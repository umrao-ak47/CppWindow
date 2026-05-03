cmake_minimum_required(VERSION 3.20)

project(cppwindow_imgui_target_provider LANGUAGES CXX)

include(FetchContent)

set(CPPWINDOW_TEST_IMGUI_SOURCE_DIR
    "${CMAKE_CURRENT_LIST_DIR}/../../.deps/imgui")
if(NOT EXISTS "${CPPWINDOW_TEST_IMGUI_SOURCE_DIR}/imgui.h")
    FetchContent_Declare(
        cppwindow_test_imgui_source
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG        v1.92.7
        SOURCE_DIR     "${CPPWINDOW_TEST_IMGUI_SOURCE_DIR}"
    )
    FetchContent_MakeAvailable(cppwindow_test_imgui_source)
endif()

add_library(test_imgui STATIC
    "${CPPWINDOW_TEST_IMGUI_SOURCE_DIR}/imgui.cpp"
    "${CPPWINDOW_TEST_IMGUI_SOURCE_DIR}/imgui_draw.cpp"
    "${CPPWINDOW_TEST_IMGUI_SOURCE_DIR}/imgui_tables.cpp"
    "${CPPWINDOW_TEST_IMGUI_SOURCE_DIR}/imgui_widgets.cpp"
    "${CPPWINDOW_TEST_IMGUI_SOURCE_DIR}/imgui_demo.cpp"
)
target_include_directories(test_imgui
    PUBLIC
        "${CPPWINDOW_TEST_IMGUI_SOURCE_DIR}"
)
target_compile_features(test_imgui PUBLIC cxx_std_20)

set(CPPWINDOW_BUILD_IMGUI ON CACHE BOOL "" FORCE)
set(CPPWINDOW_DEAR_IMGUI_TARGET test_imgui CACHE STRING "" FORCE)
set(CPPWINDOW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CPPWINDOW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(CPPWINDOW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(CPPWINDOW_INSTALL OFF CACHE BOOL "" FORCE)

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../.." cppwindow-build)

add_executable(cppwindow_imgui_target_provider main.cpp)
target_link_libraries(cppwindow_imgui_target_provider
    PRIVATE
        cppwindow::imgui
)
target_compile_features(cppwindow_imgui_target_provider
    PRIVATE
        cxx_std_20
)

enable_testing()
add_test(NAME cppwindow_imgui_target_provider COMMAND cppwindow_imgui_target_provider)
