add_library(_cppwindow_imgui_example_opengl STATIC
    opengl_imgui_renderer.cpp
)
target_include_directories(_cppwindow_imgui_example_opengl
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}"
)
target_link_libraries(_cppwindow_imgui_example_opengl
    PUBLIC
        cppwindow::imgui
    PRIVATE
        _cppwindow_example_glad
)
target_compile_features(_cppwindow_imgui_example_opengl
    PUBLIC
        cxx_std_20
)
set_target_properties(_cppwindow_imgui_example_opengl
    PROPERTIES
        FOLDER "examples"
)

function(cppwindow_add_imgui_example TARGET)
    add_executable(${TARGET}
        ${ARGN}
    )
    target_link_libraries(${TARGET}
        PRIVATE
            _cppwindow_imgui_example_opengl
    )
    target_compile_features(${TARGET}
        PRIVATE
            cxx_std_20
    )
    set_target_properties(${TARGET}
        PROPERTIES
            FOLDER "examples"
    )
endfunction()

cppwindow_add_imgui_example(cppwindow_example_imgui_minimal imgui_minimal.cpp)
cppwindow_add_imgui_example(cppwindow_example_imgui_overlay imgui_overlay.cpp)
cppwindow_add_imgui_example(cppwindow_example_imgui_input_capture imgui_input_capture.cpp)
cppwindow_add_imgui_example(cppwindow_example_imgui_style_demo imgui_style_demo.cpp)
