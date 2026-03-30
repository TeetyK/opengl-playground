cmake_minimum_required(VERSION 3.18)

project(OpenGL_PLAYVGROUND VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_path(STB_INCLUDE_DIRS "stb_image.h")

add_executable(program_main 
    src/main.cpp
    src/Shader.cpp
    src/Windows.cpp
    src/Mesh.cpp
    src/Image.cpp
    src/Logic.cpp
    src/Character.cpp
)

target_link_libraries(program_main PRIVATE
    glfw
    glad::glad
    imgui::imgui
    glm::glm
)
target_include_directories(program_main PRIVATE ${STB_INCLUDE_DIRS})

enable_testing()

add_executable(test_character test/test_character.cpp src/Character.cpp src/Mesh.cpp src/Shader.cpp)
target_link_libraries(test_character PRIVATE
    glfw
    glad::glad
    glm::glm
)
add_test(NAME CharacterTest COMMAND test_character)