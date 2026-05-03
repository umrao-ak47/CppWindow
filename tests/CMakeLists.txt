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
cppwindow_add_test(test_api_compile api_compile.cpp)
cppwindow_add_test(test_window_controls_api window_controls_api.cpp)
cppwindow_add_test(test_app_utilities app_utilities.cpp)

cppwindow_add_test(test_runtime_smoke runtime_smoke.cpp)
set_tests_properties(test_runtime_smoke
    PROPERTIES
        SKIP_RETURN_CODE 77
        TIMEOUT 15
        LABELS runtime
)

cppwindow_add_test(test_input_helpers input_helpers.cpp)
target_link_libraries(test_input_helpers
    PRIVATE
        glfw
)
target_include_directories(test_input_helpers
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src
)

cppwindow_add_test(test_glfw_input_state glfw_input_state.cpp)
target_link_libraries(test_glfw_input_state
    PRIVATE
        glfw
)
target_include_directories(test_glfw_input_state
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src
)
