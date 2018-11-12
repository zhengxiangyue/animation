cmake_minimum_required(VERSION 3.12)
project(XQ)

set(CMAKE_CXX_STANDARD 14)

find_package(glfw3 REQUIRED)
find_package(glew REQUIRED)
find_package(glm REQUIRED)
find_package(OpenCV REQUIRED)

include_directories(
        external/glfw-3.1.2/include/
        external/glm-0.9.7.1/
        external/glew-1.13.0/include/
)

add_executable(
        XQ
        main.cpp
        src/helper/shader.cpp
        src/object/buffer_object.cpp
        src/helper/texture.cpp
        src/helper/vbo_indexer.cpp
)
target_link_libraries(XQ glfw)
target_link_libraries(XQ glew)
target_link_libraries(XQ "-framework OpenGL")

add_executable(
        KeyFraming
        keyframing.cpp
        src/helper/shader.cpp
        src/object/buffer_object.cpp
        src/helper/texture.cpp
        src/helper/vbo_indexer.cpp
)
target_link_libraries(KeyFraming glfw)
target_link_libraries(KeyFraming glew)
target_link_libraries(KeyFraming "-framework OpenGL")

add_executable(
        Physics
        physics.cpp
        src/helper/shader.cpp
        src/object/buffer_object.cpp
        src/helper/texture.cpp
        src/helper/vbo_indexer.cpp
)
target_link_libraries(Physics glfw)
target_link_libraries(Physics glew)
target_link_libraries(Physics "-framework OpenGL")



