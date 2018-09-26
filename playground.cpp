//
// Create by zhengxiangyue
//
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <unordered_set>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "helper/shader.h"
#include "helper/textureLoader.h"
#include "helper/controls.h"
#include "helper/objloader.h"
#include "helper/vboindexer.h"
#include "helper/VertexBuffer.h"
#include "helper/IndexBuffer.h"

// todo: change at runtime
#define USING_QUAT true
#define CONTROLL_POINT_TYPE 0

using namespace glm;

GLFWwindow *window;

// mode 0: edit mode, 1: view mode
int mode = 1;

// current modifying objef
// it's wired since all_object declared in main...
int selected_object_index = 0;

// record all controll points so as to now show them in the specific situation
std::unordered_set<int> controll_points_index;

/**
 * Get a quaternian such that the object roll {degree} around the {axis}
 * The function does the normalize for the axis.
 * So we called this 'from intuition'
 *
 * Notice that in my program, all quaternians are (x, y, z, w) where
 * (x, y, z) represent the axis and w represents the angle
 *
 * @param degree
 * @param axis
 * @return
 */
vec4 getQuatFromIntuition(float degree, vec3 axis) {
    float hsar = sin(radians(degree / 2));
    axis = normalize(axis);
    return vec4(axis.x * hsar, axis.y * hsar, axis.z * hsar, cos(radians(degree / 2)));
}

/**
 * quat multiplication
 * @param q1
 * @param q2
 * @return
 */
vec4 quatMultiply(vec4 q1, vec4 q2) {
    float w1 = q1.w, w2 = q2.w;
    vec3 v1(q1.x, q1.y, q1.z), v2(q2.x, q2.y, q2.z);
    float w = q1.w * q2.w - (q1.x * q2.x + q1.y * q2.y + q1.z * q2.z);
    vec3 v1xv2(q1.y * q2.z - q1.z * q2.y, q1.z * q2.x - q1.x * q2.z, q1.x * q2.y - q1.y * q2.x);
    vec3 v = w1 * v2 + w2 * v1 + v1xv2;
    return vec4(v.x, v.y, v.z, w);
}

/**
 * The euler is represented as the radians around x, y, z
 * The order(of matrix) is left multiply by Z, then Y, Then X.
 * So when in the gimbal system, rotate around Z axis
 * will affect the axis position of the X and Y.
 *
 * Also notice that the order of rotating in gimbal doesn't matter
 * since the order of the matrix is fixed.
 *
 * The order(of matrix) determine how axis are mutually restrained.
 *
 * @param euler radian, float, x, y, z
 * @return
 */
mat4 eulerYXZtoRotation(vec3 euler) {
    auto rz = transpose(mat4({
                                     {cos(euler.z), -sin(euler.z), 0, 0},
                                     {sin(euler.z), cos(euler.z),  0, 0},
                                     {0,            0,             1, 0},
                                     {0,            0,             0, 1}
                             }));

    auto rx = transpose(mat4({
                                     {cos(euler.x),  0, sin(euler.x), 0},
                                     {0,             1, 0,            0},
                                     {-sin(euler.x), 0, cos(euler.x), 0},
                                     {0,             0, 0,            1}
                             }));

    auto ry = transpose(mat4({
                                     {1, 0,            0,             0},
                                     {0, cos(euler.y), -sin(euler.y), 0},
                                     {0, sin(euler.y), cos(euler.y),  0},
                                     {0, 0,            0,             1}
                             }));

    return rx * ry * rz;
}

/**
 * convert quaternian to a rotation matrix
 * @param vec4
 * @return
 */
mat4 quaternianToRotation(vec4 vec) {
    vec = normalize(vec);
    float x = vec.x, y = vec.y, z = vec.z, w = vec.w;

    return transpose(mat4({
                                  {1 - 2 * y * y - 2 * z * z, 2 * x * y - 2 * w * z,     2 * x * z + 2 * w * y},
                                  {2 * x * y + 2 * w * z,     1 - 2 * x * x - 2 * z * z, 2 * y * z - 2 * w * x},
                                  {2 * x * z - 2 * w * y,     2 * y * z + 2 * w * x,     1 - 2 * x * x - 2 * y * y},
                          }));
}


/**
 * scene object
 */
class polygon_object {
public:
    std::vector<glm::vec3> vertices, normals, indexed_vertices, indexed_normals;
    std::vector<glm::vec2> uvs, indexed_uvs;
    std::vector<unsigned short> indices;

    VertexBuffer *vb;
    VertexBuffer *uvb;
    VertexBuffer *normalvb;

    IndexBuffer *eleib;

    glm::mat4 RotationMatrix;
    glm::mat4 TranslationMatrix;
    glm::mat4 ModelMatrix;

    unsigned int texture;

    std::vector<vec4> controll_points;
    std::unordered_set<int> controll_points_index;

    std::vector<vec3> key_eurlers;
    std::vector<vec4> key_quaternians;

    // for modify mode displaying pose
    float eurler_x, eurler_y, eurler_z;
    vec4 current_quat;

    // controll point type
    int controll_point_type = 0;

    polygon_object(
            const char *path,
            vec3 position,
            float scale = 1.0,
            const char *texture_path = ""
    ) {
        loadOBJ(path, vertices, uvs, normals, scale);
        indexVBO(vertices, uvs, normals, indices, indexed_vertices, indexed_uvs, indexed_normals);

        vb = new VertexBuffer(&indexed_vertices[0], indexed_vertices.size() * sizeof(glm::vec3));
        uvb = new VertexBuffer(&indexed_uvs[0], indexed_uvs.size() * sizeof(glm::vec2));
        normalvb = new VertexBuffer(&indexed_normals[0], indexed_normals.size() * sizeof(glm::vec3));
        eleib = new IndexBuffer(&indices[0], indices.size() * sizeof(unsigned short));

        RotationMatrix = eulerYXZtoRotation(vec3(0.0, 0.0, 0.0));

        // translation matrix is derived from position
        TranslationMatrix = transpose(mat4(
                {{1.0, 0.0, 0.0, position.x},
                 {0.0, 1.0, 0.0, position.y},
                 {0.0, 0.0, 1.0, position.z},
                 {0.0, 0.0, 0.0, 1.0}
                }));

        texture = loadBMP_custom(texture_path);

        eurler_x = eurler_y = eurler_z = 0.0;

        current_quat = getQuatFromIntuition(0.0, vec3(0.0, 1.0, 0.0));
    }
};

/**
 * Rotation expression interpolation.
 * {express} contains several expression such as eulers, quternians
 * Time period is normalized into [0, 1).
 *
 * e.g. when there are 2 expression in the {express}, time should
 * belong to [0, 1). When there are 3 expression in the {express},
 * time should belong to [0, 2).
 *
 * @tparam _type
 * @param time
 * @param express
 * @return
 */
template<typename _type>
_type getInterpolateExpress(float time, std::vector<_type> express) {
    int start_index = floor(time);
    time = fmod(time, 1.0);
    return express[start_index] + time * (express[start_index + 1] - express[start_index]);
}

/**
 * Cubic interpolation.
 * 4 controll_points determines one curve
 *
 * if there are 4 controll points, time belongs to 0 and 1
 * if there are 5 controll points, time belongs to 0 and 2
 *
 * @param time
 * @param M
 * @param controll_points
 * @return
 */
mat4 getInterpolateTranslationMatrix(float time, mat4 M, std::vector<vec4> controll_points) {

    int start_index = floor(time);
    if (start_index >= controll_points.size() - 3)
        return mat4(1.0);

    time = fmod(time, 1.0);
    vec4 position = vec4(time * time * time, time * time, time, 1)
                    * M
                    * transpose(mat4(controll_points[start_index], controll_points[start_index + 1], controll_points[start_index + 2], controll_points[start_index + 3]));
    return transpose(mat4({
                                  {1.0, 0.0, 0.0, position.x},
                                  {0.0, 1.0, 0.0, position.y},
                                  {0.0, 0.0, 1.0, position.z},
                                  {0.0, 0.0, 0.0, 1.0},
                          }));
}

int main(void) {
    // Initialise GLFW
    if (!glfwInit()) {
        std::cout << "Error on GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(1024, 768, "Animation Course - Xiangyue Zheng G42416206", NULL, NULL);
    if (window == NULL)
        return -1;

    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK)
        return -1;

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    // Hide the mouse and enable unlimited mouvement
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, 1024 / 2, 768 / 2);

    // Dark blue background
    glClearColor(float(135.0 / 255.0), float(206.0 / 255.0), float(250.0 / 255.0), 0.0);
    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    // Cull triangles which normal is not towards the camera
    glDisable(GL_CULL_FACE);

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // Create and compile our GLSL program from the shaders
    GLuint programID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");

    // Get a handle for our "myTextureSampler" uniform
    GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");

    // Get a handle for our "MVP" uniform
    GLuint MatrixID = glGetUniformLocation(programID, "MVP");
    GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
    GLuint ModelMatrixID = glGetUniformLocation(programID, "M");

    std::vector<polygon_object> all_objects = {
            polygon_object("model/Mig 29_obj.obj", {9.0, -3.0, 5.0}, 0.06, "uvtemplate.bmp"),
            polygon_object("model/Sniper.obj", {0.0, -2.4, 2.0}, 0.4, "uvtemplate.bmp"),
            polygon_object("model/ak47.obj", {0.0, -2.5, 7.0}, 0.02, "uvtemplate.bmp"),
            polygon_object("model/plane.obj", {0.0, -3.0, 0.0}, 1.0, "uvtemplate.bmp"),
    };

    // Get a handle for our "LightPosition" uniform
    glUseProgram(programID);
    GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

    // For speed computation
    double lastTime = glfwGetTime();
    double lastFrameTime = lastTime,
            lastChangeMainObjectTime = lastTime,
            lastChangeMode = lastTime,
            lastAddControllPoint = lastTime;
    int nbFrames = 0;

    do {

        // Measure speed
        double currentTime = glfwGetTime();
        float deltaTime = (float) (currentTime - lastFrameTime);
        lastFrameTime = currentTime;
        nbFrames++;
        if (currentTime - lastTime >= 1.0) {
            nbFrames = 0;
            lastTime += 1.0;
        }

        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Compute the MVP matrix from keyboard and mouse input
        computeMatricesFromInputs();
        glm::mat4 ProjectionMatrix = getProjectionMatrix();
        glm::mat4 ViewMatrix = getViewMatrix();

        glm::vec3 lightPos = glm::vec3(14, 14, 14);
        glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
        glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]); // This one doesn't change between objects, so this can be done once for all objects that use "programID"

        // modify mode
        if (mode == 0) {
            // When press M, change the main object
            if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
                if (currentTime - lastChangeMainObjectTime >= 0.2) {
                    while (true) {
                        selected_object_index += 1;
                        if (controll_points_index.find(selected_object_index) == controll_points_index.end())
                            break;
                    }

                    selected_object_index %= all_objects.size();
                }
                lastChangeMainObjectTime = currentTime;
            }

            // When press UP, move upword(y increase) upword
            if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
                auto rotation = USING_QUAT ? quaternianToRotation(all_objects[selected_object_index].current_quat)
                                           : eulerYXZtoRotation(
                                vec3(all_objects[selected_object_index].eurler_x, all_objects[selected_object_index].eurler_y, all_objects[selected_object_index].eurler_z));
                auto direc = rotation * vec4(0.0, 0.0, 1.0, 1.0);
                all_objects[selected_object_index].TranslationMatrix[3][0] += 0.2 * direc.x;
                all_objects[selected_object_index].TranslationMatrix[3][1] += 0.2 * direc.y;
                all_objects[selected_object_index].TranslationMatrix[3][2] += 0.2 * direc.z;
            }
            // When press Down, move downword(y decrease) downword
            if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
                auto rotation = USING_QUAT
                                ? quaternianToRotation(all_objects[selected_object_index].current_quat)
                                : eulerYXZtoRotation(vec3(all_objects[selected_object_index].eurler_x, all_objects[selected_object_index].eurler_y, all_objects[selected_object_index].eurler_z));
                auto direc = rotation * vec4(0.0, 0.0, 1.0, 1.0);
                all_objects[selected_object_index].TranslationMatrix[3][0] -= 0.2 * direc.x;
                all_objects[selected_object_index].TranslationMatrix[3][1] -= 0.2 * direc.y;
                all_objects[selected_object_index].TranslationMatrix[3][2] -= 0.2 * direc.z;
            }

            // When press z,
            if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
                auto angle = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ? -3.0 : 3.0);
                if (USING_QUAT) {
                    all_objects[selected_object_index].current_quat = quatMultiply(
                            all_objects[selected_object_index].current_quat, getQuatFromIntuition(angle, vec3(0.0, 0.0, 1.0)));
                } else {
                    all_objects[selected_object_index].eurler_z += radians(angle);
                }
            }

            // When press y,
            if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) {
                auto angle = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ? -3.0 : 3.0);
                if (USING_QUAT) {
                    all_objects[selected_object_index].current_quat = quatMultiply(
                            all_objects[selected_object_index].current_quat, getQuatFromIntuition(angle, vec3(0.0, 1.0, 0.0)));
                } else {
                    all_objects[selected_object_index].eurler_y += radians(angle);
                }
            }
            // When press x,
            if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
                auto angle = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ? -3.0 : 3.0);
                if (USING_QUAT) {
                    all_objects[selected_object_index].current_quat = quatMultiply(
                            all_objects[selected_object_index].current_quat, getQuatFromIntuition(angle, vec3(1.0, 0.0, 0.0)));
                } else {
                    all_objects[selected_object_index].eurler_x += radians(angle);
                }
            }

            // When press p, set position
            if (glfwGetKey(window, GLFW_KEY_P)) {

                if (currentTime - lastAddControllPoint > 0.5) {
                    all_objects[selected_object_index].controll_points.emplace_back(
                            vec4(all_objects[selected_object_index].TranslationMatrix[3][0], all_objects[selected_object_index].TranslationMatrix[3][1],
                                 all_objects[selected_object_index].TranslationMatrix[3][2], 1.0));
                    all_objects.push_back(polygon_object("model/square.obj", {all_objects[selected_object_index].TranslationMatrix[3][0], all_objects[selected_object_index].TranslationMatrix[3][1],
                                                                              all_objects[selected_object_index].TranslationMatrix[3][2]}, 0.05, "uvtemplate.bmp"));
                    all_objects[selected_object_index].controll_points_index.insert(all_objects.size() - 1);
                    controll_points_index.insert(all_objects.size() - 1);

                    all_objects[selected_object_index].key_eurlers.emplace_back(
                            vec3(all_objects[selected_object_index].eurler_x, all_objects[selected_object_index].eurler_y, all_objects[selected_object_index].eurler_z));
                    all_objects[selected_object_index].key_quaternians.push_back(all_objects[selected_object_index].current_quat);
                }
                lastAddControllPoint = currentTime;
            }


            for (int i = 0; i < all_objects.size(); ++i) {

                if (controll_points_index.find(i) != controll_points_index.end() &&
                    all_objects[selected_object_index].controll_points_index.find(i) == all_objects[selected_object_index].controll_points_index.end()) {
                    continue;
                }

                auto obj = all_objects[i];

                if (i == selected_object_index) {
                    glUniform1i(glGetUniformLocation(programID, "selected"), 1);
                } else {
                    glUniform1i(glGetUniformLocation(programID, "selected"), 0);
                }

                // in the situation of "modify mode", rotation matrix is derived through
                auto modify_mode_rotation = USING_QUAT ? quaternianToRotation(obj.current_quat) : eulerYXZtoRotation(vec3(obj.eurler_x, obj.eurler_y, obj.eurler_z));
                obj.ModelMatrix = obj.TranslationMatrix * modify_mode_rotation;
                glm::mat4 MVP = ProjectionMatrix * ViewMatrix * obj.ModelMatrix;

                // Send our transformation to the currently bound shader,
                // in the "MVP" uniform
                glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
                glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &obj.ModelMatrix[0][0]);

                // Bind our texture in Texture Unit 0
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, obj.texture);
                // Set our "myTextureSampler" sampler to use Texture Unit 0
                glUniform1i(TextureID, 0);

                glEnableVertexAttribArray(0);
                obj.vb->bind();
                glVertexAttribPointer(
                        0,                  // attribute
                        3,                  // size
                        GL_FLOAT,           // type
                        GL_FALSE,           // normalized?
                        0,                  // stride
                        (void *) 0            // array buffer offset
                );

                // 2nd attribute buffer : UVs
                glEnableVertexAttribArray(1);
                obj.uvb->bind();
                glVertexAttribPointer(
                        1,                                // attribute
                        2,                                // size
                        GL_FLOAT,                         // type
                        GL_FALSE,                         // normalized?
                        0,                                // stride
                        (void *) 0                          // array buffer offset
                );

                // 3rd attribute buffer : normals
                glEnableVertexAttribArray(2);
                obj.normalvb->bind();
                glVertexAttribPointer(
                        2,                                // attribute
                        3,                                // size
                        GL_FLOAT,                         // type
                        GL_FALSE,                         // normalized?
                        0,                                // stride
                        (void *) 0                          // array buffer offset
                );

                obj.eleib->bind();
                // Draw the triangles !
                glDrawElements(
                        GL_TRIANGLES,      // mode
                        obj.indices.size(),    // count
                        GL_UNSIGNED_SHORT,   // type
                        (void *) 0           // element array buffer offset
                );

                glDisableVertexAttribArray(0);
                glDisableVertexAttribArray(1);
                glDisableVertexAttribArray(2);

            }
        } else {
            for (int i = 0; i < all_objects.size(); ++i) {
                auto obj = all_objects[i];

                if (USING_QUAT) {
                    if (obj.key_quaternians.size() >= 2) {
                        float each_rotation_duration = 2.5;
                        auto _keys = obj.key_quaternians;
                        _keys.erase(_keys.begin()), _keys.erase(_keys.end() - 1);
                        float current_rotation_time = fmod(currentTime, each_rotation_duration * (_keys.size() - 1)) / each_rotation_duration;
                        obj.RotationMatrix = quaternianToRotation(getInterpolateExpress<vec4>(current_rotation_time, _keys));
                    }
                } else {
                    if (obj.key_eurlers.size() >= 2) {
                        float each_rotation_duration = 2.5;
                        auto _keys = obj.key_eurlers;
                        _keys.erase(_keys.begin()), _keys.erase(_keys.end() - 1);
                        float current_rotation_time = fmod(currentTime, each_rotation_duration * (_keys.size() - 1)) / each_rotation_duration;
                        obj.RotationMatrix = eulerYXZtoRotation(getInterpolateExpress<vec3>(current_rotation_time, _keys));
                    }
                }


                if (obj.controll_points.size() >= 4) {
                    float each_path_duration = 2.5;
                    // use 2.5 s to pass each four controll point
                    float current_path_time = fmod(currentTime, each_path_duration * (obj.controll_points.size() - 3)) / each_path_duration;

                    std::vector<mat4> blending_matrices({
                                                                // Catmull-Rom
                                                                transpose(mat4({
                                                                                       {-0.5, 1.5,  -1.5, 0.5},
                                                                                       {1.0,  -2.5, 2.0,  -0.5},
                                                                                       {-0.5, 0.0,  0.5,  0.0},
                                                                                       {0.0,  1.0,  0.0,  0.0}
                                                                               })),
                                                                // B spline
                                                                transpose(mat4({
                                                                                       {-1.0 / 6.0, 3.0 / 6.0,  -3.0 / 6.0, 1.0 / 6.0},
                                                                                       {3.0 / 6.0,  -6.0 / 6.0, 3.0 / 6.0,  0.0},
                                                                                       {-3.0 / 6.0, 0.0 / 6.0,  3.0 / 6.0,  0.0},
                                                                                       {1.0 / 6.0,  4.0 / 6.0,  1.0 / 6.0,  0.0}
                                                                               }))
                                                        });

                    obj.TranslationMatrix = getInterpolateTranslationMatrix(
                            current_path_time,
                            blending_matrices[CONTROLL_POINT_TYPE],
                            obj.controll_points
                    );
                }

                obj.ModelMatrix = obj.TranslationMatrix * obj.RotationMatrix;
                glm::mat4 MVP = ProjectionMatrix * ViewMatrix * obj.ModelMatrix;

                // Send our transformation to the currently bound shader,
                // in the "MVP" uniform
                glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
                glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &obj.ModelMatrix[0][0]);

                // Bind our texture in Texture Unit 0, in the first assignment, not use texture in the shader
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, obj.texture);
                // Set our "myTextureSampler" sampler to use Texture Unit 0
                glUniform1i(TextureID, 0);

                glEnableVertexAttribArray(0);
                obj.vb->bind();
                glVertexAttribPointer(
                        0,
                        3,
                        GL_FLOAT,
                        GL_FALSE,
                        0,
                        (void *) 0
                );

                // 2nd attribute buffer : UVs
                glEnableVertexAttribArray(1);
                obj.uvb->bind();
                glVertexAttribPointer(
                        1,
                        2,
                        GL_FLOAT,
                        GL_FALSE,
                        0,
                        (void *) 0
                );

                // 3rd attribute buffer : normals
                glEnableVertexAttribArray(2);
                obj.normalvb->bind();
                glVertexAttribPointer(
                        2,
                        3,
                        GL_FLOAT,
                        GL_FALSE,
                        0,
                        (void *) 0
                );

                obj.eleib->bind();
                glDrawElements(
                        GL_TRIANGLES,
                        obj.indices.size(),
                        GL_UNSIGNED_SHORT,
                        (void *) 0
                );

                glDisableVertexAttribArray(0);
                glDisableVertexAttribArray(1);
                glDisableVertexAttribArray(2);

            }
        }

        // When press N, change mode
        if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
            if (currentTime - lastChangeMode >= 0.2) {
                mode = !mode;
            }
            lastChangeMode = currentTime;
        }

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0);

    glDeleteProgram(programID);
    glDeleteVertexArrays(1, &VertexArrayID);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}

