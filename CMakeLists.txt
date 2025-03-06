cmake_minimum_required(VERSION 3.10)

set(NAME Glimpse)

message(STATUS "Using ${CMAKE_GENERATOR}")
project(${NAME} VERSION 0.0.1)
set(CMAKE_CXX_STANDARD 20)


# Core library
file(GLOB_RECURSE CORE_SOURCES ${PROJECT_SOURCE_DIR}/src/core/*.cpp)
file(GLOB_RECURSE CORE_HEADERS ${PROJECT_SOURCE_DIR}/src/core/*.h)
add_library(${NAME} ${CORE_SOURCES} ${CORE_HEADERS})
target_include_directories(${NAME} PUBLIC ${PROJECT_SOURCE_DIR}/ext)
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

    target_compile_features(${GUI_NAME} PUBLIC cxx_std_17)

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
message(STATUS "Building tests????")
if(BUILD_TESTS)    
    message(STATUS "Building tests")

    enable_testing()    

    add_library(boost_ut INTERFACE)
    target_include_directories(boost_ut INTERFACE ${PROJECT_SOURCE_DIR}/ext)

    add_executable(Glimpse_tests 
        tests/testing.cpp
        tests/vec3_test.cpp
        tests/ray_test.cpp
        tests/color_test.cpp
        tests/aabb_test.cpp
        tests/texture_test.cpp
        tests/texture_test.cpp
        tests/image_test.cpp
        tests/film_test.cpp
        tests/interval_test.cpp
        tests/onb_test.cpp
        tests/pdf_test.cpp
        tests/perlin_test.cpp
        tests/camera_test.cpp
        tests/render_test.cpp
        tests/sphere_test.cpp
        tests/bvh_node_test.cpp
    )
    target_link_libraries(Glimpse_tests 
        PRIVATE
        Glimpse
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
    target_compile_options(${NAME} PRIVATE 
        /W4 
        /permissive- 
        /wd4100  # Disable unreferenced formal parameter warning
    )
    target_compile_options(${NAME}_cli PRIVATE 
        /W4 
        /permissive- 
        /wd4100  # Disable unreferenced formal parameter warning
    )
    target_compile_options(${NAME}_gui PRIVATE 
        /W4 
        /permissive- 
        /wd4100  # Disable unreferenced formal parameter warning
    )
else()
    target_compile_options(${NAME} PRIVATE -Wall -Wextra -pedantic)
    target_compile_options(${NAME}_cli PRIVATE -Wall -Wextra -pedantic)
    target_compile_options(${NAME}_gui PRIVATE -Wall -Wextra -pedantic)
endif()
