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

cppwindow_add_example(example_opengl opengl.cpp)
target_link_libraries(example_opengl
    PRIVATE
        glad
)
