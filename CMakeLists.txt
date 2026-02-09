cmake_minimum_required(VERSION 3.20)

project(glfw-vk LANGUAGES CXX)

include(FetchContent)

# ------------------------------------------------------------------------------
# Options & Versions
# ------------------------------------------------------------------------------
option(GLFWVK_INSTALL "Create install target" OFF)

set(GLFWVK_REQUIRED_GLFW_VERSION 3.4)

# ------------------------------------------------------------------------------
# Fetch GLFW
# ------------------------------------------------------------------------------
if(NOT TARGET glfw)
    message(STATUS "glfw-vk: Fetching GLFW v${GLFWVK_REQUIRED_GLFW_VERSION}...")
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG        ${GLFWVK_REQUIRED_GLFW_VERSION}
    )
    # Quiet the GLFW internal build
    set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "")
    set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "")
    set(GLFW_INSTALL OFF CACHE INTERNAL "")
    
    FetchContent_MakeAvailable(glfw)
else()
    get_target_property(FOUND_VERSION glfw VERSION)
    if(FOUND_VERSION VERSION_LESS GLFWVK_REQUIRED_GLFW_VERSION)
        message(FATAL_ERROR "glfw-vk: Existing glfw target is too old (${FOUND_VERSION}).")
    endif()
endif()

# ------------------------------------------------------------------------------
# glfw-vk library
# ------------------------------------------------------------------------------
add_library(glfw-vk STATIC)
add_library(glfw-vk::glfw-vk ALIAS glfw-vk)

target_sources(glfw-vk
    PRIVATE
        src/InputMap.cpp
        src/InputState.cpp
        src/Window.cpp
        src/Context.cpp
)

target_include_directories(glfw-vk
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# ------------------------------------------------------------------------------
# Linking & Definitions
# ------------------------------------------------------------------------------
target_link_libraries(glfw-vk
    PRIVATE
        glfw
)

target_compile_features(glfw-vk
    PUBLIC
        cxx_std_20
)

# ------------------------------------------------------------------------------
# Install rules
# ------------------------------------------------------------------------------
if(GLFWVK_INSTALL)
    include(GNUInstallDirs)
    install(
        TARGETS glfw-vk
        EXPORT glfw-vkTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
    install(
        DIRECTORY include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
    install(
        EXPORT glfw-vkTargets
        NAMESPACE glfw-vk::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/glfw-vk
    )
endif()
