cmake_minimum_required(VERSION 3.12)
project(animation)

find_package(glfw3 REQUIRED)
find_package(glew REQUIRED)
find_package(glm REQUIRED)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

add_executable(
        playground
        playground.cpp
        helper/shader.cpp
        helper/controls.cpp
        helper/textureLoader.cpp
        helper/objloader.cpp
        helper/vboindexer.cpp
        helper/VertexBuffer.cpp
        helper/IndexBuffer.cpp
        )

target_link_libraries(playground glfw)
target_link_libraries(playground glew)
target_link_libraries(playground glm)
target_link_libraries(playground "-framework OpenGL")