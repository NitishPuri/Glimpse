cmake_minimum_required(VERSION 3.10)

set(NAME Glimpse)

message(STATUS "Using ${CMAKE_GENERATOR}")

project(${NAME} VERSION 0.0.1)

# core library
# add_library(${NAME}_core STATIC ${CORE_SOURCES} ${CORE_HEADERS})
# target_include_directories(${NAME}_core PUBLIC ${PROJECT_SOURCE_DIR}/src)
# target_compile_features(${NAME}_core PUBLIC cxx_std_17)

# cli 
add_executable(${NAME}_cli src/cli/main.cpp)
target_include_directories(${NAME}_cli PUBLIC 
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/ext
)
target_compile_features(${PROJECT_NAME}_cli PUBLIC cxx_std_17)

# gui
option(BUILD_GUI "Build the GUI Viewer" ON)
if(BUILD_GUI)
    set(GUI_NAME ${NAME}_gui)

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



    add_executable(${GUI_NAME} src/gui/app.cpp)
    target_sources(${GUI_NAME} PRIVATE ${GLAD_SOURCES})
    target_sources(${GUI_NAME} PRIVATE ${IMGUI_SOURCES})

    target_compile_features(${GUI_NAME} PUBLIC cxx_std_17)

    target_link_directories(${GUI_NAME} PUBLIC ${GLFW_LIB_DIR})
    target_link_libraries(${GUI_NAME} 
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
