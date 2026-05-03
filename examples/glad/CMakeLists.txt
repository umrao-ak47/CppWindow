cmake_minimum_required(VERSION 3.20)

project(CppWindowExampleGlad LANGUAGES C)

add_library(_cppwindow_example_glad STATIC
    src/glad.c
)

target_include_directories(_cppwindow_example_glad
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

set_target_properties(_cppwindow_example_glad
    PROPERTIES
        FOLDER "examples"
)
