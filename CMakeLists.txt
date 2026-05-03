cmake_minimum_required(VERSION 3.20)

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" CPPWINDOW_VERSION)
string(STRIP "${CPPWINDOW_VERSION}" CPPWINDOW_VERSION)

project(CppWindow VERSION ${CPPWINDOW_VERSION} LANGUAGES CXX)

include(FetchContent)
include(GNUInstallDirs)

# ------------------------------------------------------------------------------
# Options & Versions
# ------------------------------------------------------------------------------
option(CPPWINDOW_INSTALL "Create install target" OFF)
option(CPPWINDOW_BUILD_DOCS "Build cppwindow API documentation with Doxygen" OFF)
option(CPPWINDOW_ENABLE_SANITIZERS "Build with address and undefined behavior sanitizers" OFF)
option(CPPWINDOW_BUILD_IMGUI "Build optional Dear ImGui integration targets" OFF)
set(CPPWINDOW_GLFW_SOURCE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/.deps/glfw"
    CACHE PATH "GLFW source directory used by FetchContent")
# Enable examples and tests automatically if this is the root project
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    option(CPPWINDOW_BUILD_EXAMPLES "Build cppwindow examples" ON)
    option(CPPWINDOW_BUILD_TESTS "Build cppwindow tests" ON)
else()
    option(CPPWINDOW_BUILD_EXAMPLES "Build cppwindow examples" OFF)
    option(CPPWINDOW_BUILD_TESTS "Build cppwindow tests" OFF)
endif()

set(CPPWINDOW_REQUIRED_GLFW_VERSION 3.4)

function(cppwindow_add_docs_target)
    find_package(Doxygen REQUIRED)

    configure_file(
        docs/Doxyfile.in
        "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
        @ONLY
    )

    add_custom_target(cppwindow_docs
        COMMAND ${DOXYGEN_EXECUTABLE} "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMENT "Generating CppWindow API documentation"
        VERBATIM
    )
endfunction()

get_filename_component(
    CPPWINDOW_GLFW_SOURCE_DIR_ABS
    "${CPPWINDOW_GLFW_SOURCE_DIR}"
    ABSOLUTE
    BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR
   AND CPPWINDOW_BUILD_DOCS
   AND NOT CPPWINDOW_BUILD_EXAMPLES
   AND NOT CPPWINDOW_BUILD_TESTS
   AND NOT CPPWINDOW_INSTALL)
    cppwindow_add_docs_target()
    return()
endif()

# ------------------------------------------------------------------------------
# Fetch GLFW
# ------------------------------------------------------------------------------
if(NOT TARGET glfw)
    if(EXISTS "${CPPWINDOW_GLFW_SOURCE_DIR_ABS}/CMakeLists.txt")
        message(STATUS "cppwindow: Configuring GLFW from ${CPPWINDOW_GLFW_SOURCE_DIR_ABS}")
    else()
        message(STATUS
            "cppwindow: Fetching GLFW v${CPPWINDOW_REQUIRED_GLFW_VERSION} "
            "to ${CPPWINDOW_GLFW_SOURCE_DIR_ABS}")
    endif()
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG        ${CPPWINDOW_REQUIRED_GLFW_VERSION}
        SOURCE_DIR     "${CPPWINDOW_GLFW_SOURCE_DIR_ABS}"
    )
    # Quiet the GLFW internal build
    set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "")
    set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "")
    set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "")
    set(GLFW_INSTALL ${CPPWINDOW_INSTALL} CACHE INTERNAL "")

    FetchContent_MakeAvailable(glfw)
else()
    get_target_property(FOUND_VERSION glfw VERSION)
    if(FOUND_VERSION VERSION_LESS CPPWINDOW_REQUIRED_GLFW_VERSION)
        message(FATAL_ERROR "cppwindow: Existing glfw target is too old (${FOUND_VERSION}).")
    endif()
endif()

# ------------------------------------------------------------------------------
# Sanitizers
# ------------------------------------------------------------------------------
if(CPPWINDOW_ENABLE_SANITIZERS)
    if(MSVC)
        message(FATAL_ERROR
            "CPPWINDOW_ENABLE_SANITIZERS is only supported with GCC/Clang-like compilers")
    endif()

    set(CPPWINDOW_SANITIZER_FLAGS -fsanitize=address,undefined)
    add_compile_options(${CPPWINDOW_SANITIZER_FLAGS} -fno-omit-frame-pointer)
    add_link_options(${CPPWINDOW_SANITIZER_FLAGS})
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
        $<BUILD_INTERFACE:glfw>
        $<INSTALL_INTERFACE:glfw>
)

target_compile_features(cppwindow
    PUBLIC
        cxx_std_20
)

target_compile_definitions(cppwindow PRIVATE
    $<$<PLATFORM_ID:Windows>:CPPWINDOW_PLATFORM_WINDOWS>
    $<$<PLATFORM_ID:Windows>:NOMINMAX>
    $<$<PLATFORM_ID:Windows>:WIN32_LEAN_AND_MEAN>
    $<$<PLATFORM_ID:Darwin>:CPPWINDOW_PLATFORM_MACOS>
    $<$<PLATFORM_ID:Linux>:CPPWINDOW_PLATFORM_LINUX>
    $<$<AND:$<PLATFORM_ID:Linux>,$<BOOL:${GLFW_BUILD_X11}>>:CPPWINDOW_GLFW_HAS_X11>
    $<$<AND:$<PLATFORM_ID:Linux>,$<BOOL:${GLFW_BUILD_WAYLAND}>>:CPPWINDOW_GLFW_HAS_WAYLAND>
)

# ------------------------------------------------------------------------------
# Tests bootstrap
# ------------------------------------------------------------------------------
if(CPPWINDOW_BUILD_TESTS)
    include(CTest)
endif()

# ------------------------------------------------------------------------------
# Examples
# ------------------------------------------------------------------------------
if(CPPWINDOW_BUILD_EXAMPLES)
    add_subdirectory(examples/glad)
    add_subdirectory(examples)
endif()

# ------------------------------------------------------------------------------
# Optional Dear ImGui integration
# ------------------------------------------------------------------------------
if(CPPWINDOW_BUILD_IMGUI)
    add_subdirectory(extras/imgui)
endif()

# ------------------------------------------------------------------------------
# Tests
# ------------------------------------------------------------------------------
if(CPPWINDOW_BUILD_TESTS)
    add_subdirectory(tests)
endif()

# ------------------------------------------------------------------------------
# Install rules
# ------------------------------------------------------------------------------
if(CPPWINDOW_INSTALL)
    include(CMakePackageConfigHelpers)

    install(
        TARGETS cppwindow
        EXPORT cppwindowTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
    install(
        DIRECTORY include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    configure_package_config_file(
        cmake/cppwindowConfig.cmake.in
        "${CMAKE_CURRENT_BINARY_DIR}/cppwindowConfig.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cppwindow
    )
    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/cppwindowConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )
    install(
        FILES
            "${CMAKE_CURRENT_BINARY_DIR}/cppwindowConfig.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/cppwindowConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cppwindow
    )
    install(
        EXPORT cppwindowTargets
        NAMESPACE cppwindow::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cppwindow
    )
endif()

# ------------------------------------------------------------------------------
# Documentation
# ------------------------------------------------------------------------------
if(CPPWINDOW_BUILD_DOCS)
    cppwindow_add_docs_target()
endif()
