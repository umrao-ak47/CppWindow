include(FetchContent)
include(GNUInstallDirs)

if(NOT TARGET cppwindow::cppwindow)
    message(FATAL_ERROR "cppwindow: extras/imgui requires cppwindow::cppwindow")
endif()

set(CPPWINDOW_DEAR_IMGUI_TARGET
    ""
    CACHE STRING "Existing Dear ImGui target to use instead of CppWindow's fetched target")
set(CPPWINDOW_REQUIRED_IMGUI_VERSION v1.92.7 CACHE STRING "Dear ImGui git tag")

set(CPPWINDOW_FETCHED_IMGUI_SOURCE_DIR "${PROJECT_SOURCE_DIR}/.deps/imgui")

if(CPPWINDOW_DEAR_IMGUI_TARGET)
    if(NOT TARGET "${CPPWINDOW_DEAR_IMGUI_TARGET}")
        message(FATAL_ERROR
            "cppwindow: CPPWINDOW_DEAR_IMGUI_TARGET must name an existing target. "
            "Define the Dear ImGui target before add_subdirectory(CppWindow).")
    endif()
    if(CPPWINDOW_INSTALL)
        message(FATAL_ERROR
            "cppwindow: installing the ImGui integration with CPPWINDOW_DEAR_IMGUI_TARGET "
            "is not supported. Use CppWindow's fetched Dear ImGui target for package builds.")
    endif()

    message(STATUS "cppwindow: Using Dear ImGui target ${CPPWINDOW_DEAR_IMGUI_TARGET}")
    add_library(cppwindow_dear_imgui INTERFACE)
    add_library(cppwindow::dear_imgui ALIAS cppwindow_dear_imgui)
    target_link_libraries(cppwindow_dear_imgui
        INTERFACE
            "${CPPWINDOW_DEAR_IMGUI_TARGET}"
    )
else()
    if(EXISTS "${CPPWINDOW_FETCHED_IMGUI_SOURCE_DIR}/imgui.h")
        message(STATUS
            "cppwindow: Configuring Dear ImGui from ${CPPWINDOW_FETCHED_IMGUI_SOURCE_DIR}")
    else()
        message(STATUS
            "cppwindow: Fetching Dear ImGui ${CPPWINDOW_REQUIRED_IMGUI_VERSION} "
            "to ${CPPWINDOW_FETCHED_IMGUI_SOURCE_DIR}")
    endif()
    FetchContent_Declare(
        cppwindow_dear_imgui_source
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG        ${CPPWINDOW_REQUIRED_IMGUI_VERSION}
        SOURCE_DIR     "${CPPWINDOW_FETCHED_IMGUI_SOURCE_DIR}"
    )
    FetchContent_MakeAvailable(cppwindow_dear_imgui_source)

    add_library(cppwindow_dear_imgui STATIC)
    add_library(cppwindow::dear_imgui ALIAS cppwindow_dear_imgui)
    set_target_properties(cppwindow_dear_imgui PROPERTIES EXPORT_NAME dear_imgui)

    target_sources(cppwindow_dear_imgui
        PRIVATE
            "${CPPWINDOW_FETCHED_IMGUI_SOURCE_DIR}/imgui.cpp"
            "${CPPWINDOW_FETCHED_IMGUI_SOURCE_DIR}/imgui_draw.cpp"
            "${CPPWINDOW_FETCHED_IMGUI_SOURCE_DIR}/imgui_tables.cpp"
            "${CPPWINDOW_FETCHED_IMGUI_SOURCE_DIR}/imgui_widgets.cpp"
            "${CPPWINDOW_FETCHED_IMGUI_SOURCE_DIR}/imgui_demo.cpp"
    )
    target_include_directories(cppwindow_dear_imgui
        PUBLIC
            $<BUILD_INTERFACE:${CPPWINDOW_FETCHED_IMGUI_SOURCE_DIR}>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/imgui>
    )
    target_compile_features(cppwindow_dear_imgui PUBLIC cxx_std_20)
endif()

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
            "${CPPWINDOW_FETCHED_IMGUI_SOURCE_DIR}/imconfig.h"
            "${CPPWINDOW_FETCHED_IMGUI_SOURCE_DIR}/imgui.h"
            "${CPPWINDOW_FETCHED_IMGUI_SOURCE_DIR}/imgui_internal.h"
            "${CPPWINDOW_FETCHED_IMGUI_SOURCE_DIR}/imstb_rectpack.h"
            "${CPPWINDOW_FETCHED_IMGUI_SOURCE_DIR}/imstb_textedit.h"
            "${CPPWINDOW_FETCHED_IMGUI_SOURCE_DIR}/imstb_truetype.h"
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
