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
    auto left_leg = new hierarchical_node<vec3>(lego_ptr, "model/lego_left_leg.obj", {0.0, -0.62, 0.0}, 20.0, "texture/Bandit Texture_Low Poly (3).bmp", vec3(0.0, -0.7, 0.0), vec3(0.0, -0.7, 0.0));
    auto right_leg = new hierarchical_node<vec3>(lego_ptr, "model/lego_right_leg.obj", {0.0, -0.62, 0.0}, 20.0, "texture/Bandit Texture_Low Poly (3).bmp", vec3(0.0, -0.7, 0.0), vec3(0.0, -0.7, 0.0));
    auto left_arm = new hierarchical_node<vec3>(lego_ptr, "model/lego_left_arm.obj", {0.0, -0.62, 0.0}, 20.0, "texture/Bandit Texture_Low Poly (3).bmp", vec3(0.0, -0.5, 0.0), vec3(0.0, -0.5, 0.0));
    auto right_arm = new hierarchical_node<vec3>(lego_ptr, "model/lego_right_arm.obj", {0.0, -0.62, 0.0}, 20.0, "texture/Bandit Texture_Low Poly (3).bmp", vec3(0.0, -0.5, 0.0), vec3(0.0, -0.5, 0.0));
    auto dog = new hierarchical_node<vec3>(nullptr, "model/Hero_Dog.obj", {5.0, -1.21, 5.0}, 0.008, "texture/dog2.bmp", vec3(0.0, -0.5, 0.0), vec3(0.0, -0.5, 0.0));
    auto objects = std::vector<hierarchical_node<vec3> *>(
        {
            lego_ptr,
            left_arm,
            right_arm,
            left_leg,
            right_leg,
            new hierarchical_node<vec3>(nullptr, "model/plane.obj", {0.0, -1.0, 0.0}, 1.0, "texture/uvtemplate.bmp"),
            dog,
        });

    // For speed computation
    double lastTime = glfwGetTime();
    double lastFrameTime = lastTime,
        lastChangeMainObjectTime = lastTime,
        lastChangeMode = lastTime,
        last_press_space = lastTime,
        last_press_up = lastTime - 0.8,
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

        // press up to move legs and arms, each motion take 0.8 seconds
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS && current_time - last_press_up > 0.8) {
            lego_ptr->actions.push_back(action<glm::vec3>(current_time, {
                vec4(0.0, 0.0, 0.0, 1.0),
                vec4(0.0, 0.0, 0.0, 1.0),
                vec4(0.0, 0.08, 0.0, 1.0),
                vec4(0.0, 0.0, 0.0, 1.0),
                vec4(0.0, 0.08, 0.0, 1.0),
                vec4(0.0, 0.0, 0.0, 1.0),
                vec4(0.0, 0.0, 0.0, 1.0),
            }, {}));
            // First key and last key will be ignored, 0.8 is each duration(0.2) * numbers(4)
            left_leg->actions.push_back(action<glm::vec3>(current_time, {}, {
                vec3(0.0, radians(45.0), 0.0),
                vec3(0.0, 0.0, 0.0),
                vec3(0.0, radians(-45.0), 0.0),
                vec3(0.0, 0.0, 0.0),
                vec3(0.0, radians(45.0), 0.0),
                vec3(0.0, 0.0, 0.0),
                vec3(0.0, radians(-45.0), 0.0),
            }));
            left_arm->actions.push_back(action<glm::vec3>(current_time, {}, {
                vec3(0.0, radians(-45.0), 0.0),
                vec3(0.0, 0.0, 0.0),
                vec3(0.0, radians(45.0), 0.0),
                vec3(0.0, 0.0, 0.0),
                vec3(0.0, radians(-45.0), 0.0),
                vec3(0.0, 0.0, 0.0),
                vec3(0.0, radians(45.0), 0.0),
            }));
            right_leg->actions.push_back(action<glm::vec3>(current_time, {}, {
                vec3(0.0, radians(-45.0), 0.0),
                vec3(0.0, 0.0, 0.0),
                vec3(0.0, radians(45.0), 0.0),
                vec3(0.0, 0.0, 0.0),
                vec3(0.0, radians(-45.0), 0.0),
                vec3(0.0, 0.0, 0.0),
                vec3(0.0, radians(45.0), 0.0),
            }));
            right_arm->actions.push_back(action<glm::vec3>(current_time, {}, {
                vec3(0.0, radians(45.0), 0.0),
                vec3(0.0, 0.0, 0.0),
                vec3(0.0, radians(-45.0), 0.0),
                vec3(0.0, 0.0, 0.0),
                vec3(0.0, radians(45.0), 0.0),
                vec3(0.0, 0.0, 0.0),
                vec3(0.0, radians(-45.0), 0.0),
            }));
            last_press_up = current_time;
        }

        // press space to jump
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && current_time - last_press_space > 0.8) {
            if (lego_ptr->actions.size() > 0)
                lego_ptr->actions.erase(lego_ptr->actions.begin());
            lego_ptr->actions.push_back(action<glm::vec3>(current_time, {
                vec4(0.0, 0.0, 0.0, 1.0),
                vec4(0.0, 0.0, 0.0, 1.0),
                vec4(0.0, 0.3, 0.0, 1.0),
                vec4(0.0, 0.5, 0.0, 1.0),
                vec4(0.0, 0.3, 0.0, 1.0),
                vec4(0.0, 0.0, 0.0, 1.0),
                vec4(0.0, 0.0, 0.0, 1.0),
            }, {}));
            last_press_space = current_time;
        }

        if (current_time - last_press_up < 0.8) {
            // get current direction
            glm::vec4 current_direction = lego_ptr->current_rotation * normalize(vec4(0.0, 0.0, 1.0, 1.0));
            lego_ptr->current_translation[3][0] += 0.06 * current_direction[0];
            lego_ptr->current_translation[3][1] += 0.06 * current_direction[1];
            lego_ptr->current_translation[3][2] += 0.06 * current_direction[2];
        }

        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            // get current direction
            lego_ptr->current_rotation = expressToRotation(getQuatFromIntuition(3.0, vec3(0.0, 1.0, 0.0))) * lego_ptr->current_rotation;
        }

        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            // get current direction
            lego_ptr->current_rotation = expressToRotation(getQuatFromIntuition(-3.0, vec3(0.0, 1.0, 0.0))) * lego_ptr->current_rotation;
        }

//         computeMatricesFromInputs();
        computeMatricesFromCharacter(lego_ptr->current_translation, lego_ptr->current_rotation);
        glm::mat4 projection_matrix = getProjectionMatrix();
        glm::mat4 view_matrix = getViewMatrix();

        // Compute the MVP matrix from keyboard and mouse input

        glm::vec3 lightPos = glm::vec3(14, 14, 14);
        glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
        glUniformMatrix4fv(ViewMatrixId, 1, GL_FALSE, &view_matrix[0][0]);

        for (int i = 0; i < objects.size(); ++i) {

            // check the actions
            auto removing = std::remove_if(objects[i]->actions.begin(), objects[i]->actions.end(), [current_time](action<glm::vec3> _action) {
                    return current_time > _action.end_time;
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

