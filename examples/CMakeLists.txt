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

# add dependencies
add_subdirectory(glad)

cppwindow_add_example(example_basic basic.cpp)

cppwindow_add_example(example_borderless borderless.cpp)

cppwindow_add_example(example_window_controls window_controls.cpp)
target_link_libraries(example_window_controls
    PRIVATE
        glad
)

cppwindow_add_example(example_fullscreen_toggle fullscreen_toggle.cpp)
target_link_libraries(example_fullscreen_toggle
    PRIVATE
        glad
)
cppwindow_add_example(example_mouse_capture mouse_capture.cpp)
target_link_libraries(example_mouse_capture
    PRIVATE
        glad
)

cppwindow_add_example(example_text_input text_input.cpp)

cppwindow_add_example(example_gamepad gamepad.cpp)

cppwindow_add_example(example_opengl opengl.cpp)
target_link_libraries(example_opengl
    PRIVATE
        glad
)

cppwindow_add_example(example_heightmap heightmap.cpp)
target_link_libraries(example_heightmap
    PRIVATE
        glad
)

cppwindow_add_example(example_particles particles.cpp)
target_link_libraries(example_particles
    PRIVATE
        glad
)
