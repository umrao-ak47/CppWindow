add_executable(test_imgui_api_compile imgui_api_compile.cpp)
target_link_libraries(test_imgui_api_compile
    PRIVATE
        cppwindow::imgui
)
target_compile_features(test_imgui_api_compile
    PRIVATE
        cxx_std_20
)
add_test(NAME test_imgui_api_compile COMMAND test_imgui_api_compile)

if(TARGET _cppwindow_imgui_example_opengl)
    add_executable(test_imgui_runtime_smoke imgui_runtime_smoke.cpp)
    target_link_libraries(test_imgui_runtime_smoke
        PRIVATE
            _cppwindow_imgui_example_opengl
    )
    target_compile_features(test_imgui_runtime_smoke
        PRIVATE
            cxx_std_20
    )
    add_test(NAME test_imgui_runtime_smoke COMMAND test_imgui_runtime_smoke)
    set_tests_properties(test_imgui_runtime_smoke
        PROPERTIES
            SKIP_RETURN_CODE 77
            TIMEOUT 15
            LABELS runtime
    )
endif()
