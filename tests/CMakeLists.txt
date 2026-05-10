function(cppwindow_add_api_test TARGET)
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

function(cppwindow_add_integration_test TARGET)
    cppwindow_add_api_test(${TARGET} ${ARGN})

    set_tests_properties(${TARGET}
        PROPERTIES
            LABELS runtime
    )
endfunction()

function(cppwindow_add_internal_test TARGET)
    cppwindow_add_api_test(${TARGET} ${ARGN})

    target_include_directories(${TARGET}
        PRIVATE
            ${PROJECT_SOURCE_DIR}/src
    )
endfunction()

# Public API and compile-surface tests.
cppwindow_add_api_test(test_api_compile api/api_compile.cpp)
cppwindow_add_api_test(test_window_controls_api api/window_controls_api.cpp)
cppwindow_add_api_test(test_app_utilities api/app_utilities.cpp)

# Runtime integration tests.
cppwindow_add_integration_test(test_runtime_smoke integration/runtime_smoke.cpp)
set_tests_properties(test_runtime_smoke
    PROPERTIES
        SKIP_RETURN_CODE 77
        TIMEOUT 15
)

# Internal implementation tests.
cppwindow_add_internal_test(test_static_lookup internal/static_lookup.cpp)
cppwindow_add_internal_test(test_window_registry internal/window_registry.cpp)
cppwindow_add_internal_test(test_input_helpers internal/input_helpers.cpp)
cppwindow_add_internal_test(test_glfw_input_state internal/glfw_input_state.cpp)
