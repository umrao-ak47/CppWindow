cmake_minimum_required(VERSION 3.20)

project(cppwindow_package_consumer LANGUAGES CXX)

option(CPPWINDOW_PACKAGE_CONSUMER_USE_IMGUI "Require installed ImGui component targets" OFF)

file(READ "${CMAKE_CURRENT_LIST_DIR}/../../VERSION" CPPWINDOW_VERSION)
string(STRIP "${CPPWINDOW_VERSION}" CPPWINDOW_VERSION)

if(CPPWINDOW_PACKAGE_CONSUMER_USE_IMGUI)
    find_package(cppwindow ${CPPWINDOW_VERSION} CONFIG REQUIRED COMPONENTS imgui)
else()
    find_package(cppwindow ${CPPWINDOW_VERSION} CONFIG REQUIRED)
endif()

add_executable(cppwindow_package_consumer main.cpp)

if(CPPWINDOW_PACKAGE_CONSUMER_USE_IMGUI)
    target_link_libraries(cppwindow_package_consumer
        PRIVATE
            cppwindow::imgui
    )
    target_compile_definitions(cppwindow_package_consumer
        PRIVATE
            CPPWINDOW_PACKAGE_CONSUMER_USE_IMGUI
    )
else()
    target_link_libraries(cppwindow_package_consumer
        PRIVATE
            cppwindow::cppwindow
    )
endif()

target_compile_features(cppwindow_package_consumer
    PRIVATE
        cxx_std_20
)

enable_testing()
add_test(NAME cppwindow_package_consumer COMMAND cppwindow_package_consumer)
