cmake_minimum_required(VERSION 3.18)

project(OpenGL_PLAYVGROUND VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)

add_executable(program_main 
    src/main.cpp
)

target_link_libraries(program_main PRIVATE
    glfw
    glad::glad
)