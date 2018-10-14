cmake_minimum_required(VERSION 3.8)
project(animation)

find_package(glfw3 REQUIRED)
find_package(glew REQUIRED)
find_package(glm REQUIRED)
find_package(OpenCV REQUIRED)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

include_directories(
        external/glfw-3.1.2/include/
        external/glm-0.9.7.1/
        external/glew-1.13.0/include/
)

add_executable(
        playground
        playground.cpp
        helper/shader.cpp
        helper/textureLoader.cpp
        helper/vboIndexer.cpp
        helper/vertexBuffer.cpp
        helper/IndexBuffer.cpp
        )
target_link_libraries(playground glfw)
target_link_libraries(playground glew)
target_link_libraries(playground "-framework OpenGL")

set(USE_AVX_INSTRUCTIONS ON CACHE BOOL "Use AVX instructions")
add_subdirectory(/Users/zhengxiangyue/Projects/dlib dlib_build)
add_executable(face_recognation face_recognation.cpp)
target_link_libraries(face_recognation dlib::dlib ${OpenCV_LIBS})