//
// Create by zhengxiangyue
//
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <unordered_set>
#include <fstream>

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
#define FOREVER -20

using namespace glm;

GLFWwindow *window;

// mode 0: edit mode, 1: view mode
int mode = 1;

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

    window = glfwCreateWindow(1024, 768, "Motion control toy", NULL, NULL);
    if (window == NULL)
        exit(1);

    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK)
        exit(1);

    // Ensure we can capture the escape key being pressed below
//    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
//    // Hide the mouse and enable unlimited mouvement
//    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
//
//    // Set the mouse at the center of the screen
//    glfwPollEvents();
//    glfwSetCursorPos(window, 1024 / 2, 768 / 2);

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
    auto left_leg = new hierarchical_node<vec3>(lego_ptr, "model/lego_left_leg.obj", {0.0, -0.62, 0.0}, 20.0, "texture/Bandit Texture_Low Poly (3).bmp", vec3(0.0, -0.7, 0.0), vec3(0.0, -0.7, 0.0));
    auto right_leg = new hierarchical_node<vec3>(lego_ptr, "model/lego_right_leg.obj", {0.0, -0.62, 0.0}, 20.0, "texture/Bandit Texture_Low Poly (3).bmp", vec3(0.0, -0.7, 0.0), vec3(0.0, -0.7, 0.0));
    auto left_arm = new hierarchical_node<vec3>(lego_ptr, "model/lego_left_arm.obj", {0.0, -0.62, 0.0}, 20.0, "texture/Bandit Texture_Low Poly (3).bmp", vec3(0.0, -0.5, 0.0), vec3(0.0, -0.5, 0.0));
    auto right_arm = new hierarchical_node<vec3>(lego_ptr, "model/lego_right_arm.obj", {0.0, -0.62, 0.0}, 20.0, "texture/Bandit Texture_Low Poly (3).bmp", vec3(0.0, -0.5, 0.0), vec3(0.0, -0.5, 0.0));
    auto plane = new hierarchical_node<vec3>(nullptr, "model/plane.obj", {0.0, -1.0, 100.0}, 1.0, "texture/uvtemplate.bmp");

    auto square = new hierarchical_node<vec3>(plane, "model/square.obj", {0.0, -1.0, 8.0}, 0.2, "texture/uvtemplate.bmp");
    auto square1 = new hierarchical_node<vec3>(plane, "model/square.obj", {1.0, -1.0, 13.0}, 0.2, "texture/uvtemplate.bmp");
    auto square2 = new hierarchical_node<vec3>(plane, "model/square.obj", {-1.0, -1.0, 16.0}, 0.2, "texture/uvtemplate.bmp");
    auto square3 = new hierarchical_node<vec3>(plane, "model/square.obj", {1.0, -1.0, 16.0}, 0.2, "texture/uvtemplate.bmp");
    auto square4 = new hierarchical_node<vec3>(plane, "model/square.obj", {0.0, -1.0, 16.0}, 0.2, "texture/uvtemplate.bmp");
    auto square5 = new hierarchical_node<vec3>(plane, "model/square.obj", {1.0, -1.0, 23.0}, 0.2, "texture/uvtemplate.bmp");
    auto square6 = new hierarchical_node<vec3>(plane, "model/square.obj", {-1.0, -1.0, 32.0}, 0.2, "texture/uvtemplate.bmp");
    auto square7 = new hierarchical_node<vec3>(plane, "model/square.obj", {0.0, -1.0, 40.0}, 0.2, "texture/uvtemplate.bmp");
    auto square8 = new hierarchical_node<vec3>(plane, "model/square.obj", {1.0, -1.0, 45.0}, 0.2, "texture/uvtemplate.bmp");
    auto square9 = new hierarchical_node<vec3>(plane, "model/square.obj", {-1.0, -1.0, 60.0}, 0.2, "texture/uvtemplate.bmp");
    auto square10 = new hierarchical_node<vec3>(plane, "model/square.obj", {1.0, -1.0, 79.0}, 0.2, "texture/uvtemplate.bmp");
    auto square11 = new hierarchical_node<vec3>(plane, "model/square.obj", {-1.0, -1.0, 99.0}, 0.2, "texture/uvtemplate.bmp");
    auto square12 = new hierarchical_node<vec3>(plane, "model/square.obj", {0.0, -1.0, 102.0}, 0.2, "texture/uvtemplate.bmp");
    auto square13 = new hierarchical_node<vec3>(plane, "model/square.obj", {1.0, -1.0, 113.0}, 0.2, "texture/uvtemplate.bmp");
    auto square14 = new hierarchical_node<vec3>(plane, "model/square.obj", {1.0, -1.0, 123.0}, 0.2, "texture/uvtemplate.bmp");
    auto square15 = new hierarchical_node<vec3>(plane, "model/square.obj", {1.0, -1.0, 123.0}, 0.2, "texture/uvtemplate.bmp");
    auto square16 = new hierarchical_node<vec3>(plane, "model/square.obj", {-1.0, -1.0, 132.0}, 0.2, "texture/uvtemplate.bmp");
    auto square17 = new hierarchical_node<vec3>(plane, "model/square.obj", {0.0, -1.0, 140.0}, 0.2, "texture/uvtemplate.bmp");
    auto square18 = new hierarchical_node<vec3>(plane, "model/square.obj", {1.0, -1.0, 145.0}, 0.2, "texture/uvtemplate.bmp");
    auto square19 = new hierarchical_node<vec3>(plane, "model/square.obj", {-1.0, -1.0, 160.0}, 0.2, "texture/uvtemplate.bmp");
    auto square20 = new hierarchical_node<vec3>(plane, "model/square.obj", {1.0, -1.0, 179.0}, 0.2, "texture/uvtemplate.bmp");
    auto square21 = new hierarchical_node<vec3>(plane, "model/square.obj", {-1.0, -1.0, 199.0}, 0.2, "texture/uvtemplate.bmp");


    auto objects = std::vector<hierarchical_node<vec3> *>(
        {
            lego_ptr,
            left_arm,
            right_arm,
            left_leg,
            right_leg,
            plane,
            square,
            square1,
            square2,
            square3,
            square4,
            square5,
            square6,
            square7,
            square8,
            square9,
            square10,
            square11,
            square12,
            square13,
            square14,
            square15,
            square16,
            square17,
            square18,
            square19,
            square20,
            square21,
        });

    lego_ptr->actions.push_back(action<glm::vec3>(0, {
        vec4(0.0, 0.0, 0.0, 1.0),
        vec4(0.0, 0.0, 0.0, 1.0),
        vec4(0.0, 0.08, 0.0, 1.0),
        vec4(0.0, 0.0, 0.0, 1.0),
        vec4(0.0, 0.08, 0.0, 1.0),
        vec4(0.0, 0.0, 0.0, 1.0),
        vec4(0.0, 0.0, 0.0, 1.0),
    }, {}, true));
    // First key and last key will be ignored, 0.8 is each duration(0.2) * numbers(4)
    left_leg->actions.push_back(action<glm::vec3>(0, {}, {
        vec3(0.0, radians(45.0), 0.0),
        vec3(0.0, 0.0, 0.0),
        vec3(0.0, radians(-45.0), 0.0),
        vec3(0.0, 0.0, 0.0),
        vec3(0.0, radians(45.0), 0.0),
        vec3(0.0, 0.0, 0.0),
        vec3(0.0, radians(-45.0), 0.0),
    }, true));
    left_arm->actions.push_back(action<glm::vec3>(0, {}, {
        vec3(0.0, radians(-45.0), 0.0),
        vec3(0.0, 0.0, 0.0),
        vec3(0.0, radians(45.0), 0.0),
        vec3(0.0, 0.0, 0.0),
        vec3(0.0, radians(-45.0), 0.0),
        vec3(0.0, 0.0, 0.0),
        vec3(0.0, radians(45.0), 0.0),
    }, true));
    right_leg->actions.push_back(action<glm::vec3>(0, {}, {
        vec3(0.0, radians(-45.0), 0.0),
        vec3(0.0, 0.0, 0.0),
        vec3(0.0, radians(45.0), 0.0),
        vec3(0.0, 0.0, 0.0),
        vec3(0.0, radians(-45.0), 0.0),
        vec3(0.0, 0.0, 0.0),
        vec3(0.0, radians(45.0), 0.0),
    }, true));
    right_arm->actions.push_back(action<glm::vec3>(0, {}, {
        vec3(0.0, radians(45.0), 0.0),
        vec3(0.0, 0.0, 0.0),
        vec3(0.0, radians(-45.0), 0.0),
        vec3(0.0, 0.0, 0.0),
        vec3(0.0, radians(45.0), 0.0),
        vec3(0.0, 0.0, 0.0),
        vec3(0.0, radians(-45.0), 0.0),
    }, true));

    // For speed computation
    double lastTime = glfwGetTime();
    double lastFrameTime = lastTime,
        lastChangeMainObjectTime = lastTime,
        lastChangeMode = lastTime,
        last_press_space = lastTime,
        last_press_up = lastTime - 0.8,
        last_press_left = lastTime - 0.8,
        last_press_right = lastTime - 0.8,
        lastAddcontrolPoint = lastTime;
    int nbFrames = 0;

    std::string last_motion;

    do {
        double current_time = glfwGetTime();
        float deltaTime = (float) (current_time - lastFrameTime);
        lastFrameTime = current_time;
        nbFrames++;
        if (current_time - lastTime >= 1.0) {
            nbFrames = 0;
            lastTime += 1.0;
        }

        std::ifstream motionFile;
        motionFile.open("lip_position.txt");
        std::string current_motion;
        // read motion from file
        if (motionFile >> current_motion && last_motion != current_motion) {
            motionFile >> current_motion;
            if (current_motion == "left" && current_time - last_press_left > 0.5) {
                // use 0.5 s to
                last_press_left = current_time;
            } else if (current_motion == "right" && current_time - last_press_right > 0.5) {
                // use 0.5 s to
                last_press_right = current_time;
            } else if (current_motion == "top" && current_time - last_press_space > 0.8) {
                lego_ptr->actions.push_back(action<glm::vec3>(current_time, {
                    vec4(0.0, 0.0, 0.0, 1.0),
                    vec4(0.0, 0.0, 0.0, 1.0),
                    vec4(0.0, 0.4, 0.0, 1.0),
                    vec4(0.0, 0.6, 0.0, 1.0),
                    vec4(0.0, 0.4, 0.0, 1.0),
                    vec4(0.0, 0.0, 0.0, 1.0),
                    vec4(0.0, 0.0, 0.0, 1.0),
                }, {}));
                last_press_space = current_time;
            }

        }
        last_motion = current_motion;
        motionFile.close();

        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        plane->current_translation[3][2] -= 0.05;
        if (plane->current_translation[3][2] <= -200.0) {
            plane->current_translation[3][2] = 0.0;
        }

        if (current_time - last_press_left < 0.5) {
            lego_ptr->current_translation[3][0] += 0.04;
        }
        if (current_time - last_press_right < 0.5) {
            lego_ptr->current_translation[3][0] -= 0.04;
        }

//        computeMatricesFromInputs();
        computeMatricesFromCharacter(lego_ptr->current_translation, lego_ptr->current_rotation);

        glm::mat4 projection_matrix = getProjectionMatrix();
        glm::mat4 view_matrix = getViewMatrix();

        glm::vec3 lightPos = glm::vec3(14, 14, 14);
        glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
        glUniformMatrix4fv(ViewMatrixId, 1, GL_FALSE, &view_matrix[0][0]);

        for (int i = 0; i < objects.size(); ++i) {

            // check the actions
            auto removing = std::remove_if(objects[i]->actions.begin(), objects[i]->actions.end(), [current_time](action<glm::vec3> _action) {
                    return _action.end_time > _action.start_time && current_time > _action.end_time;
            });
            if (objects[i]->actions.size() > 0 && removing != objects[i]->actions.end())
                objects[i]->actions.erase(removing);

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

