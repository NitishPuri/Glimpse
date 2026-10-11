cmake_minimum_required(VERSION 3.10)

set(NAME Glimpse)

message(STATUS "Using ${CMAKE_GENERATOR}")
project(${NAME} VERSION 0.0.1)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  # for clangd / VS Code IntelliSense
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# Core library
file(GLOB_RECURSE CORE_SOURCES ${PROJECT_SOURCE_DIR}/src/glimpse/*.cpp)
file(GLOB_RECURSE CORE_HEADERS ${PROJECT_SOURCE_DIR}/src/glimpse/*.h)
add_library(${NAME} ${CORE_SOURCES} ${CORE_HEADERS})
target_include_directories(${NAME} PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_include_directories(${NAME} SYSTEM PUBLIC ${PROJECT_SOURCE_DIR}/ext)  # vendored: no warnings from it
target_compile_features(${NAME} PUBLIC cxx_std_17)


# cli 
add_executable(${NAME}_cli src/cli/main.cpp)
target_include_directories(${NAME}_cli PUBLIC 
    ${PROJECT_SOURCE_DIR}/src
    # ${PROJECT_SOURCE_DIR}/ext
)
target_link_libraries(${NAME}_cli ${NAME})
target_compile_features(${NAME}_cli PUBLIC cxx_std_17)


# gui
option(BUILD_GUI "Build the GUI Viewer" ON)
if(BUILD_GUI)
    message(STATUS "Building GUI")

    # GLFW 3.4 from source, pinned by hash (same as Glint). X11 only: Wayland needs extra dev packages.
    include(FetchContent)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(glfw
      URL https://github.com/glfw/glfw/archive/refs/tags/3.4.tar.gz
      URL_HASH SHA256=c038d34200234d071fae9345bc455e4a8f2f544ab60150765d7704e08f3dac01)
    FetchContent_MakeAvailable(glfw)
    set(OpenGL_GL_PREFERENCE GLVND)  # silences CMP0072; GLVND is the modern libOpenGL/libGLX split
    find_package(OpenGL REQUIRED)

    set(GUI_NAME ${NAME}_gui)
    set(GUI_PATH ${PROJECT_SOURCE_DIR}/src/gui)
    set(IMGUI_PATH ${PROJECT_SOURCE_DIR}/ext/imgui)  # vendored: ImGui + ImPlot + glfw/opengl3 backends
    set(GLAD_PATH ${PROJECT_SOURCE_DIR}/ext/glad)    # vendored GL loader

    file(GLOB IMGUI_SOURCES ${IMGUI_PATH}/*.cpp)
    file(GLOB_RECURSE APP_HEADERS ${GUI_PATH}/*.h)
    file(GLOB_RECURSE APP_SOURCES ${GUI_PATH}/*.cpp)

    add_executable(${GUI_NAME} ${APP_SOURCES} ${APP_HEADERS} ${IMGUI_SOURCES} ${GLAD_PATH}/src/glad.c)
    target_compile_features(${GUI_NAME} PUBLIC cxx_std_20)
    target_link_libraries(${GUI_NAME} PRIVATE ${NAME} glfw OpenGL::GL)
    target_include_directories(${GUI_NAME} PRIVATE
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/ext
        ${GLAD_PATH}/include
    )
endif()

# Tests
option(BUILD_TESTS "Build the tests" ON)
if(BUILD_TESTS)    
    message(STATUS "Building tests")

    enable_testing()    

    add_library(boost_ut INTERFACE)
    target_include_directories(boost_ut INTERFACE ${PROJECT_SOURCE_DIR}/ext)

    add_executable(Glimpse_tests 
        tests/test_cfg.h
        tests/testing.cpp

        tests/unit_tests/random_test.cpp
        tests/unit_tests/vec3_test.cpp
        tests/unit_tests/ray_test.cpp
        tests/unit_tests/aabb_test.cpp
        tests/unit_tests/texture_test.cpp
        tests/unit_tests/image_test.cpp
        tests/unit_tests/film_test.cpp
        tests/unit_tests/interval_test.cpp
        tests/unit_tests/onb_test.cpp
        tests/unit_tests/pdf_test.cpp
        tests/unit_tests/perlin_test.cpp
        tests/unit_tests/camera_test.cpp
        tests/unit_tests/render_test.cpp
        tests/unit_tests/sphere_test.cpp
        tests/unit_tests/bvh_node_test.cpp
        tests/unit_tests/cli_test.cpp
        tests/unit_tests/gui_layout_test.cpp
        tests/unit_tests/logger_test.cpp

        tests/e2e/e2e_test.cpp
        tests/e2e/test_scenes.h
    )
    target_link_libraries(Glimpse_tests 
        PRIVATE
        ${NAME}
        boost_ut
    )
    target_include_directories(Glimpse_tests 
        PRIVATE 
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/ext
    )

    # boost.ut's runner calls std::exit(-1) when any assertion failed, so ctest can rely on the exit code.
    # Tests write their scratch files into the build tree, and run from the repo root like the apps (./res).
    target_compile_definitions(Glimpse_tests PRIVATE GLIMPSE_TEST_OUTPUT_DIR="${CMAKE_CURRENT_BINARY_DIR}/test_output")
    add_test(NAME GlimpseTests COMMAND Glimpse_tests WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
endif()




# Enable warnings
if (MSVC)
    set(COMPILE_OPTIONS 
        /W4 
        /permissive- 
        /wd4100  # Disable unreferenced formal parameter warning
    )

    target_compile_options(${NAME} PRIVATE ${COMPILE_OPTIONS})
    target_compile_options(${NAME}_cli PRIVATE ${COMPILE_OPTIONS})
    # target_compile_options(${NAME}_gui PRIVATE ${COMPILE_OPTIONS})
    target_compile_options(${NAME}_tests PRIVATE ${COMPILE_OPTIONS})

    target_compile_definitions(${NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_compile_definitions(${NAME}_cli PRIVATE _CRT_SECURE_NO_WARNINGS)
    # target_compile_definitions(${NAME}_gui PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_compile_definitions(${NAME}_tests PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
    set(COMPILE_OPTIONS 
        -Wall -Wextra -pedantic 
        -Wno-unused-parameter  # Disable unused parameter warning
    )
    target_compile_options(${NAME} PRIVATE ${COMPILE_OPTIONS})
    target_compile_options(${NAME}_cli PRIVATE ${COMPILE_OPTIONS})
    target_compile_options(${NAME}_tests PRIVATE ${COMPILE_OPTIONS})
endif()
