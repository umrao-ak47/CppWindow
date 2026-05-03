include(FetchContent)
include(GNUInstallDirs)

if(NOT TARGET cppwindow::cppwindow)
    message(FATAL_ERROR "cppwindow: extras/imgui requires cppwindow::cppwindow")
endif()

option(CPPWINDOW_FETCH_IMGUI "Fetch Dear ImGui when optional ImGui integration is enabled" ON)
set(CPPWINDOW_IMGUI_SOURCE_DIR
    "${PROJECT_SOURCE_DIR}/.deps/imgui"
    CACHE PATH "Dear ImGui source directory used by FetchContent")
set(CPPWINDOW_REQUIRED_IMGUI_VERSION v1.92.7 CACHE STRING "Dear ImGui git tag")

get_filename_component(
    CPPWINDOW_IMGUI_SOURCE_DIR_ABS
    "${CPPWINDOW_IMGUI_SOURCE_DIR}"
    ABSOLUTE
    BASE_DIR "${PROJECT_SOURCE_DIR}"
)

if(NOT EXISTS "${CPPWINDOW_IMGUI_SOURCE_DIR_ABS}/imgui.h")
    if(NOT CPPWINDOW_FETCH_IMGUI)
        message(FATAL_ERROR
            "cppwindow: CPPWINDOW_BUILD_IMGUI requires Dear ImGui sources at "
            "${CPPWINDOW_IMGUI_SOURCE_DIR_ABS} when CPPWINDOW_FETCH_IMGUI is OFF")
    endif()

    message(STATUS
        "cppwindow: Fetching Dear ImGui ${CPPWINDOW_REQUIRED_IMGUI_VERSION} "
        "to ${CPPWINDOW_IMGUI_SOURCE_DIR_ABS}")
    FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG        ${CPPWINDOW_REQUIRED_IMGUI_VERSION}
        SOURCE_DIR     "${CPPWINDOW_IMGUI_SOURCE_DIR_ABS}"
    )
    FetchContent_MakeAvailable(imgui)
else()
    message(STATUS "cppwindow: Configuring Dear ImGui from ${CPPWINDOW_IMGUI_SOURCE_DIR_ABS}")
endif()

add_library(cppwindow_dear_imgui STATIC)
add_library(cppwindow::dear_imgui ALIAS cppwindow_dear_imgui)
set_target_properties(cppwindow_dear_imgui PROPERTIES EXPORT_NAME dear_imgui)

target_sources(cppwindow_dear_imgui
    PRIVATE
        "${CPPWINDOW_IMGUI_SOURCE_DIR_ABS}/imgui.cpp"
        "${CPPWINDOW_IMGUI_SOURCE_DIR_ABS}/imgui_draw.cpp"
        "${CPPWINDOW_IMGUI_SOURCE_DIR_ABS}/imgui_tables.cpp"
        "${CPPWINDOW_IMGUI_SOURCE_DIR_ABS}/imgui_widgets.cpp"
        "${CPPWINDOW_IMGUI_SOURCE_DIR_ABS}/imgui_demo.cpp"
)
target_include_directories(cppwindow_dear_imgui
    PUBLIC
        $<BUILD_INTERFACE:${CPPWINDOW_IMGUI_SOURCE_DIR_ABS}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/imgui>
)
target_compile_features(cppwindow_dear_imgui PUBLIC cxx_std_20)

add_library(cppwindow_imgui STATIC)
add_library(cppwindow::imgui ALIAS cppwindow_imgui)
set_target_properties(cppwindow_imgui PROPERTIES EXPORT_NAME imgui)

target_sources(cppwindow_imgui
    PRIVATE
        src/platform.cpp
)
target_include_directories(cppwindow_imgui
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(cppwindow_imgui
    PUBLIC
        cppwindow::cppwindow
        cppwindow::dear_imgui
)
target_compile_features(cppwindow_imgui PUBLIC cxx_std_20)

if(CPPWINDOW_INSTALL)
    install(
        DIRECTORY include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
    install(
        TARGETS
            cppwindow_dear_imgui
            cppwindow_imgui
        EXPORT cppwindowTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
    install(
        FILES
            "${CPPWINDOW_IMGUI_SOURCE_DIR_ABS}/imconfig.h"
            "${CPPWINDOW_IMGUI_SOURCE_DIR_ABS}/imgui.h"
            "${CPPWINDOW_IMGUI_SOURCE_DIR_ABS}/imgui_internal.h"
            "${CPPWINDOW_IMGUI_SOURCE_DIR_ABS}/imstb_rectpack.h"
            "${CPPWINDOW_IMGUI_SOURCE_DIR_ABS}/imstb_textedit.h"
            "${CPPWINDOW_IMGUI_SOURCE_DIR_ABS}/imstb_truetype.h"
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/imgui
    )
endif()

if(CPPWINDOW_BUILD_EXAMPLES)
    if(TARGET _cppwindow_example_glad)
        add_subdirectory(examples)
    else()
        message(STATUS
            "cppwindow: Skipping ImGui example because _cppwindow_example_glad is unavailable")
    endif()
endif()

if(CPPWINDOW_BUILD_TESTS)
    add_subdirectory(tests)
endif()
