cmake_minimum_required(VERSION 3.24)

# Standalone experiment. This project is intentionally not referenced by the
# CloudShell or repository build; configure and build it on its own.
project(maaend-visual-hosting-experiment LANGUAGES CXX)

if(NOT WIN32)
    message(FATAL_ERROR "Visual hosting experiment only supports Windows")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Reuse the CloudShell WebView2 SDK locator (WEBVIEW2_SDK_DIR env/cache var).
include("${CMAKE_CURRENT_LIST_DIR}/../../CloudShell/cmake/WebView2Sdk.cmake")

add_executable(visual-hosting-wheel
    src/main.cpp
    src/VisualHost.cpp
    src/Util.cpp
)

target_include_directories(visual-hosting-wheel PRIVATE src)
target_link_libraries(visual-hosting-wheel PRIVATE
    WebView2::LoaderStatic
    dcomp
    dwmapi
    shlwapi
    shcore
    ole32
    user32
    gdi32
    shell32
)
target_compile_definitions(visual-hosting-wheel PRIVATE
    UNICODE
    _UNICODE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
)
target_compile_options(visual-hosting-wheel PRIVATE /utf-8 /W4 /permissive-)

# The test page is loaded from the executable directory at runtime.
add_custom_command(TARGET visual-hosting-wheel POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/wheel-page.html"
        "$<TARGET_FILE_DIR:visual-hosting-wheel>/wheel-page.html"
    VERBATIM
)
