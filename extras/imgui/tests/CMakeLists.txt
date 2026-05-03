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
