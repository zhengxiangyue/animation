//
// Create by zhengxiangyue
//
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <unordered_set>

#include "common/gl_util.h"
#include "common/math.h"

#include "elements/segment.h"

#include "helper/shader.h"
#include "helper/textureLoader.h"
#include "helper/controls.h"
#include "helper/objLoader.h"
#include "helper/vboIndexer.h"
#include "helper/vertexBuffer.h"
#include "helper/IndexBuffer.h"

// todo: change at runtime
#define USING_QUAT true
#define control_POINT_TYPE 0

using namespace glm;

GLFWwindow *window;

// mode 0: edit mode, 1: view mode
int mode = 1;

// current modifying objef
// it's wired since all_object declared in main...
int selected_object_index = 0;

// record all control points so as to now show them in the specific situation
std::unordered_set<int> control_points_index;

void initialGL(unsigned int &MatrixID,
               unsigned int &LightID,
               unsigned int &ViewMatrixId,
               unsigned int &ModelMatrixID,
               unsigned int &TextureID,
               unsigned int &VertexArrayID,
               unsigned int &programID) {
    // Initialise GLFW
    if (!glfwInit()) {
        std::cout << "Error on GLFW" << std::endl;
        exit(1);
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(1024, 768, "Animation Course - Xiangyue Zheng G42416206", NULL, NULL);
    if (window == NULL)
        exit(1);

    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK)
        exit(1);

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

    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // Create and compile our GLSL program from the shaders
    programID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");

    // Get a handle for our "myTextureSampler" uniform
    TextureID = glGetUniformLocation(programID, "myTextureSampler");

    // Get a handle for our "MVP" uniform
    MatrixID = glGetUniformLocation(programID, "MVP");
    ViewMatrixId = glGetUniformLocation(programID, "V");
    ModelMatrixID = glGetUniformLocation(programID, "M");

    // Get a handle for our "LightPosition" uniform
    glUseProgram(programID);
    LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
}

int main(void) {

    unsigned int MatrixID, LightID, ViewMatrixId, ModelMatrixID, TextureID, VertexArrayID, programID;
    initialGL(MatrixID, LightID, ViewMatrixId, ModelMatrixID, TextureID, VertexArrayID, programID);

    auto lego_ptr = new hierarchical_node<vec3>(nullptr, "model/lego.obj", {0.0, -0.62, 0.0}, 20.0, "texture/Bandit Texture_Low Poly (3).bmp");
    auto left_arm = new hierarchical_node<vec3>(lego_ptr, "model/lego_left_arm.obj", {0.0, -0.62, 0.0}, 20.0, "texture/Bandit Texture_Low Poly (3).bmp", vec3(0.0, -0.5, 0.0), vec3(0.0, -0.5, 0.0));

    auto objects = std::vector<hierarchical_node<vec3> *>(
        {
            lego_ptr,
            new hierarchical_node<vec3>(lego_ptr, "model/lego_left_leg.obj", {0.0, -0.62, 0.0}, 20.0, "texture/Bandit Texture_Low Poly (3).bmp", vec3(0.0, -0.7, 0.0), vec3(0.0, -0.7, 0.0)),
            new hierarchical_node<vec3>(lego_ptr, "model/lego_right_leg.obj", {0.0, -0.62, 0.0}, 20.0, "texture/Bandit Texture_Low Poly (3).bmp", vec3(0.0, -0.7, 0.0), vec3(0.0, -0.7, 0.0)),
            left_arm,
            new hierarchical_node<vec3>(lego_ptr, "model/lego_right_arm.obj", {0.0, -0.62, 0.0}, 20.0, "texture/Bandit Texture_Low Poly (3).bmp", vec3(0.0, -0.5, 0.0), vec3(0.0, -0.5, 0.0)),
            new hierarchical_node<vec3>(left_arm, "model/ak47.obj", {0.0, 2.0, 0.0}, 0.006, "texture/KFC.bmp", vec3(-0.2, -0.3, 0.0), vec3(0.0, 2.5, 0.0)),
            new hierarchical_node<vec3>(nullptr, "model/plane.obj", {0.0, -1.0, 0.0}, 1.0, "texture/uvtemplate.bmp"),
        });

    float step = 0.5;

    for (int i = 0; i < 361; ++i) {
        objects[0]->control_points.push_back(vec4(20 * sin(radians(i * 1.0)), i % 2 ? 0.0 : 0.05, 20 * cos(radians(i * 1.0)), 1.0));
        objects[0]->key_rotations.push_back(vec3(radians(90.0 + i * 1.0), 0.0, 0.0));
    }

    objects[1]->key_rotations = {
        vec3(0.0, radians(45.0), 0.0),
        vec3(0.0, 0.0, 0.0),
        vec3(0.0, radians(-45.0), 0.0),
        vec3(0.0, 0.0, 0.0),
        vec3(0.0, radians(45.0), 0.0),
        vec3(0.0, 0.0, 0.0),
        vec3(0.0, radians(-45.0), 0.0),
    };

    objects[2]->key_rotations = {
        vec3(0.0, radians(-45.0), 0.0),
        vec3(0.0, 0.0, 0.0),
        vec3(0.0, radians(45.0), 0.0),
        vec3(0.0, 0.0, 0.0),
        vec3(0.0, radians(-45.0), 0.0),
        vec3(0.0, 0.0, 0.0),
        vec3(0.0, radians(45.0), 0.0),
    };

    objects[3]->key_rotations = {
        vec3(0.0, radians(-60.0), 0.0),
        vec3(0.0, 0.0, 0.0),
        vec3(0.0, radians(60.0), 0.0),
        vec3(0.0, 0.0, 0.0),
        vec3(0.0, radians(-60.0), 0.0),
        vec3(0.0, 0.0, 0.0),
        vec3(0.0, radians(60.0), 0.0),
    };

    objects[4]->key_rotations = {
        vec3(0.0, radians(60.0), 0.0),
        vec3(0.0, 0.0, 0.0),
        vec3(0.0, radians(-60.0), 0.0),
        vec3(0.0, 0.0, 0.0),
        vec3(0.0, radians(60.0), 0.0),
        vec3(0.0, 0.0, 0.0),
        vec3(0.0, radians(-60.0), 0.0),
    };

    objects[5]->key_rotations = {
        vec3(radians(90.0), 0.0, 0.0),
        vec3(radians(90.0), 0.0, 0.0),
        vec3(radians(90.0), 0.0, 0.0),
        vec3(radians(90.0), 0.0, 0.0),
    };


    // For speed computation
    double lastTime = glfwGetTime();
    double lastFrameTime = lastTime,
        lastChangeMainObjectTime = lastTime,
        lastChangeMode = lastTime,
        lastAddcontrolPoint = lastTime;
    int nbFrames = 0;

    do {
        double current_time = glfwGetTime();
        float deltaTime = (float) (current_time - lastFrameTime);
        lastFrameTime = current_time;
        nbFrames++;
        if (current_time - lastTime >= 1.0) {
            nbFrames = 0;
            lastTime += 1.0;
        }

        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Compute the MVP matrix from keyboard and mouse input
        computeMatricesFromInputs();
        glm::mat4 projection_matrix = getProjectionMatrix();
        glm::mat4 view_matrix = getViewMatrix();

        glm::vec3 lightPos = glm::vec3(14, 14, 14);
        glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
        glUniformMatrix4fv(ViewMatrixId, 1, GL_FALSE, &view_matrix[0][0]);

        for (int i = 0; i < objects.size(); ++i) {

            glm::mat4 model_matrix;

            model_matrix = objects[i]->getTransformationMatrix(current_time);

            glm::mat4 MVP = projection_matrix * view_matrix * model_matrix;

            // Send our transformation to the currently bound shader,
            // in the "MVP" uniform
            glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
            glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &model_matrix[0][0]);

            // Bind our texture in Texture Unit 0, in the first assignment, not use texture in the shader
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, objects[i]->texture);
            // Set our "myTextureSampler" sampler to use Texture Unit 0
            glUniform1i(TextureID, 0);

            glEnableVertexAttribArray(0);
            objects[i]->vb->bind();
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
            objects[i]->uvb->bind();
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
            objects[i]->normalvb->bind();
            glVertexAttribPointer(
                2,
                3,
                GL_FLOAT,
                GL_FALSE,
                0,
                (void *) 0
            );

            objects[i]->eleib->bind();
            glDrawElements(
                GL_TRIANGLES,
                objects[i]->indices.size(),
                GL_UNSIGNED_SHORT,
                (void *) 0
            );

            glDisableVertexAttribArray(0);
            glDisableVertexAttribArray(1);
            glDisableVertexAttribArray(2);

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

