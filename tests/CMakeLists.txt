function(cppwindow_add_test TARGET)
    add_executable(${TARGET} ${ARGN})

    target_link_libraries(${TARGET}
        PRIVATE
            cppwindow::cppwindow
    )

    target_compile_features(${TARGET}
        PRIVATE
            cxx_std_20
    )

    add_test(NAME ${TARGET} COMMAND ${TARGET})
endfunction()

cppwindow_add_test(test_static_lookup static_lookup.cpp)
cppwindow_add_test(test_window_controls_api window_controls_api.cpp)

cppwindow_add_test(test_glfw_input_state glfw_input_state.cpp)
target_link_libraries(test_glfw_input_state
    PRIVATE
        glfw
)
target_include_directories(test_glfw_input_state
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src
)
