function(cppwindow_add_imgui_example TARGET)
    add_executable(${TARGET}
        ${ARGN}
        "${CPPWINDOW_IMGUI_SOURCE_DIR_ABS}/backends/imgui_impl_opengl3.cpp"
    )
    target_include_directories(${TARGET}
        PRIVATE
            "${CPPWINDOW_IMGUI_SOURCE_DIR_ABS}/backends"
    )
    target_link_libraries(${TARGET}
        PRIVATE
            cppwindow::imgui
            _cppwindow_example_glad
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

cppwindow_add_imgui_example(example_imgui_minimal imgui_minimal.cpp)
cppwindow_add_imgui_example(example_imgui_overlay imgui_overlay.cpp)
cppwindow_add_imgui_example(example_imgui_input_capture imgui_input_capture.cpp)
cppwindow_add_imgui_example(example_imgui_style_demo imgui_style_demo.cpp)
