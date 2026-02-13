cmake_minimum_required(VERSION 3.20)

project(CppWindow LANGUAGES CXX)

include(FetchContent)

# ------------------------------------------------------------------------------
# Options & Versions
# ------------------------------------------------------------------------------
option(CPPWINDOW_INSTALL "Create install target" OFF)
# Enable examples automatically if this is the root project
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    option(CPPWINDOW_BUILD_EXAMPLES "Build cppwindow examples" ON)
else()
    option(CPPWINDOW_BUILD_EXAMPLES "Build cppwindow examples" OFF)
endif()

set(CPPWINDOW_REQUIRED_GLFW_VERSION 3.4)

# ------------------------------------------------------------------------------
# Fetch GLFW
# ------------------------------------------------------------------------------
if(NOT TARGET glfw)
    message(STATUS "cppwindow: Fetching GLFW v${CPPWINDOW_REQUIRED_GLFW_VERSION}...")
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG        ${CPPWINDOW_REQUIRED_GLFW_VERSION}
    )
    # Quiet the GLFW internal build
    set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "")
    set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "")
    set(GLFW_INSTALL OFF CACHE INTERNAL "")
    
    FetchContent_MakeAvailable(glfw)
else()
    get_target_property(FOUND_VERSION glfw VERSION)
    if(FOUND_VERSION VERSION_LESS CPPWINDOW_REQUIRED_GLFW_VERSION)
        message(FATAL_ERROR "cppwindow: Existing glfw target is too old (${FOUND_VERSION}).")
    endif()
endif()

# ------------------------------------------------------------------------------
# cppwindow library
# ------------------------------------------------------------------------------
add_library(cppwindow STATIC)
add_library(cppwindow::cppwindow ALIAS cppwindow)

target_sources(cppwindow
    PRIVATE
        src/backend/glfw/glfw_impl.cpp
        src/cppwindow.cpp
)

target_include_directories(cppwindow
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# ------------------------------------------------------------------------------
# Linking & Definitions
# ------------------------------------------------------------------------------
target_link_libraries(cppwindow
    PRIVATE
        glfw
)

target_compile_features(cppwindow
    PUBLIC
        cxx_std_20
)

# ------------------------------------------------------------------------------
# Install rules
# ------------------------------------------------------------------------------
if(CPPWINDOW_INSTALL)
    include(GNUInstallDirs)
    install(
        TARGETS cppwindow
        EXPORT cppwindowTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
    install(
        DIRECTORY include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
    install(
        EXPORT cppwindowTargets
        NAMESPACE cppwindow::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cppwindow
    )
endif()

# -----------------------------
# Examples
# -----------------------------
if(CPPWINDOW_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
