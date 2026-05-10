function(cppwindow_add_example TARGET)
    add_executable(${TARGET} ${ARGN})

    target_link_libraries(${TARGET}
        PRIVATE
            cppwindow::cppwindow
    )

    target_compile_features(${TARGET}
        PRIVATE
            cxx_std_20
    )

    # Keep IDEs clean
    set_target_properties(${TARGET}
        PROPERTIES
            FOLDER "examples"
    )
endfunction()

function(cppwindow_link_example_glad TARGET)
    target_link_libraries(${TARGET}
        PRIVATE
            _cppwindow_example_glad
    )
endfunction()

cppwindow_add_example(cppwindow_example_basic basic.cpp)

cppwindow_add_example(cppwindow_example_borderless borderless.cpp)

cppwindow_add_example(cppwindow_example_window_controls window_controls.cpp)
cppwindow_link_example_glad(cppwindow_example_window_controls)

cppwindow_add_example(cppwindow_example_fullscreen_toggle fullscreen_toggle.cpp)
cppwindow_link_example_glad(cppwindow_example_fullscreen_toggle)
cppwindow_add_example(cppwindow_example_mouse_capture mouse_capture.cpp)
cppwindow_link_example_glad(cppwindow_example_mouse_capture)

cppwindow_add_example(cppwindow_example_text_input text_input.cpp)

cppwindow_add_example(cppwindow_example_gamepad gamepad.cpp)

cppwindow_add_example(cppwindow_example_app_utilities app_utilities.cpp)

cppwindow_add_example(cppwindow_example_input_helpers input_helpers.cpp)
cppwindow_link_example_glad(cppwindow_example_input_helpers)

cppwindow_add_example(cppwindow_example_multi_window multi_window.cpp)

cppwindow_add_example(cppwindow_example_event_viewer event_viewer.cpp)

cppwindow_add_example(cppwindow_example_monitor_info monitor_info.cpp)

cppwindow_add_example(cppwindow_example_native_handles native_handles.cpp)

cppwindow_add_example(cppwindow_example_fixed_step_loop fixed_step_loop.cpp)

cppwindow_add_example(cppwindow_example_opengl opengl.cpp)
cppwindow_link_example_glad(cppwindow_example_opengl)

cppwindow_add_example(cppwindow_example_heightmap heightmap.cpp)
cppwindow_link_example_glad(cppwindow_example_heightmap)

cppwindow_add_example(cppwindow_example_particles particles.cpp)
cppwindow_link_example_glad(cppwindow_example_particles)
