cmake_minimum_required(VERSION 3.10)

set(NAME Glimpse)

message(STATUS "Using ${CMAKE_GENERATOR}")
project(${NAME} VERSION 0.0.1)
set(CMAKE_CXX_STANDARD 20)


# Core library
file(GLOB_RECURSE CORE_SOURCES ${PROJECT_SOURCE_DIR}/src/core/*.cpp)
file(GLOB_RECURSE CORE_HEADERS ${PROJECT_SOURCE_DIR}/src/core/*.h)
add_library(${NAME} ${CORE_SOURCES} ${CORE_HEADERS})
target_include_directories(${NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/ext
    ${PROJECT_SOURCE_DIR}/src
 )
target_compile_features(${NAME} PUBLIC cxx_std_17)


# cli 
add_executable(${NAME}_cli src/cli/main.cpp)
target_include_directories(${NAME}_cli PUBLIC 
    ${PROJECT_SOURCE_DIR}/src
    # ${PROJECT_SOURCE_DIR}/ext
)
target_link_libraries(${NAME}_cli ${NAME})
target_compile_features(${NAME}_cli PUBLIC cxx_std_17)


# Platform detection
if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
    message(STATUS "Linux platform detected")
endif()

# gui
option(BUILD_GUI "Build the GUI Viewer" ON)
if(BUILD_GUI AND NOT LINUX)
    message(STATUS "Building GUI")

    set(GUI_NAME ${NAME}_gui)
    set(GUI_PATH ${PROJECT_SOURCE_DIR}/src/gui)

    # GLFW
    set(GLFW_PATH ${PROJECT_SOURCE_DIR}/ext/glfw-3.4.bin.WIN64)
    set(GLFW_INCLUDE ${GLFW_PATH}/include)
    set(GLFW_LIB_DIR ${GLFW_PATH}/lib-vc2022)

    # ImGui
    set(IMGUI_PATH ${PROJECT_SOURCE_DIR}/ext/imgui)
    file(GLOB_RECURSE IMGUI_SOURCES ${IMGUI_PATH}/*.cpp)

    # GLAD
    set(GLAD_PATH ${PROJECT_SOURCE_DIR}/ext/glad)
    set(GLAD_INCLUDE ${GLAD_PATH}/include)
    file(GLOB_RECURSE GLAD_SOURCES ${GLAD_PATH}/src/*.c)

    file(GLOB_RECURSE APP_HEADERS ${GUI_PATH}/*.h)
    file(GLOB_RECURSE APP_SOURCES ${GUI_PATH}/*.cpp)

    add_executable(${GUI_NAME} ${APP_SOURCES} ${APP_HEADERS})
    target_sources(${GUI_NAME} PRIVATE ${GLAD_SOURCES})
    target_sources(${GUI_NAME} PRIVATE ${IMGUI_SOURCES})

    target_compile_features(${GUI_NAME} PUBLIC cxx_std_20)

    target_link_directories(${GUI_NAME} PUBLIC ${GLFW_LIB_DIR})
    target_link_libraries(${GUI_NAME}
        ${NAME} 
        glfw3
        opengl32.lib
    )

    target_include_directories(${GUI_NAME} PUBLIC
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/ext
        ${GLAD_INCLUDE}
        ${GLFW_INCLUDE}
    )
endif()

# Tests
# Does not work!!
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

    add_test(
        NAME GlimpseTests 
        COMMAND Glimpse_tests
        WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    )

    # Add test properties to show output
    set_tests_properties(GlimpseTests PROPERTIES
       FAIL_REGULAR_EXPRESSION "FAILED"
       PASS_REGULAR_EXPRESSION "Suite.*\\| 0 failed"
    )
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
