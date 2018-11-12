#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

GLFWwindow *window;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

#include "src/helper/shader.h"
#include "src/helper/texture.h"
#include "src/helper/objLoader.h"
#include "src/helper/vbo_indexer.h"

#include "src/object/gl_camera.h"
#include "src/object/rigid_object.h"

#define BALL_RADIUS 0.8

void update_velocity_after_contact(glm::vec3 normalized_contact_normal, rigid_object* object) {
    glm::vec3 normal_velocity = (object->current_velocity * normalized_contact_normal) * normalized_contact_normal;
    glm::vec3 perpen_velocity = object->current_velocity - normal_velocity;

    normal_velocity = -0.6f * normal_velocity;
    object->current_velocity = normal_velocity + perpen_velocity;

    glm::vec3 axis = glm::cross(perpen_velocity, normal_velocity);
    object->current_angular_velocity = {-axis.x, -axis.y, -axis.z, glm::length(glm::vec3(object->current_velocity.x, 0.0, object->current_velocity.z))};
};

int main(void) {
    // Initialise GLFW
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        getchar();
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow(1024, 768, "Key framing animation", NULL, NULL);
    if (window == NULL) {
        fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return -1;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    // Hide the mouse and enable unlimited mouvement
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, 512 / 2, 512 / 2);

    // Dark blue background
    glClearColor(float(135.0 / 255.0), float(206.0 / 255.0), float(250.0 / 255.0), 0.0);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    // Cull triangles which normal is not towards the camera
    glEnable(GL_CULL_FACE);

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // Create and compile our GLSL program from the shaders
    GLuint programID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");

    GLuint MatrixID = glGetUniformLocation(programID, "MVP");
    GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
    GLuint ModelMatrixID = glGetUniformLocation(programID, "M");
    GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");
    GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
    glUseProgram(programID);

    glm::vec3 lightPos = glm::vec3(4, 4, 4);
    glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

    // load a ball into a buffer package
    auto ball_package = loadSimpleObjFileToIndexedBuffer("model/beachball.obj", glm::mat4(0.02 * BALL_RADIUS));
    auto lego_package = loadSimpleObjFileToIndexedBuffer("model/Hero_Dog.obj", glm::mat4(0.01));
    auto plane_package = loadSimpleObjFileToIndexedBuffer("model/plane.obj", glm::mat4(1.0));

    auto plane = new rigid_object(plane_package);
    plane->setTexture("uvtemplate.bmp");

    auto wall1 = new rigid_object(plane_package);
    wall1->setTexture("uvtemplate.bmp");
    wall1->current_rotation_in_quaternion = getQuatFromIntuition(90.0, vec3(1.0, 0.0, 0.0));
    wall1->current_translation[3][2] = -100;

    auto wall2 = new rigid_object(plane_package);
    wall2->setTexture("uvtemplate.bmp");
    wall2->current_rotation_in_quaternion = getQuatFromIntuition(90.0, vec3(-1.0, 0.0, 0.0));
    wall2->current_translation[3][2] = 100;

    auto wall3 = new rigid_object(plane_package);
    wall3->setTexture("uvtemplate.bmp");
    wall3->current_rotation_in_quaternion = getQuatFromIntuition(90.0, vec3(0.0, 0.0, 1.0));
    wall3->current_translation[3][0] = 100;

    auto wall4 = new rigid_object(plane_package);
    wall4->setTexture("uvtemplate.bmp");
    wall4->current_rotation_in_quaternion = getQuatFromIntuition(90.0, vec3(0.0, 0.0, -1.0));
    wall4->current_translation[3][0] = -100;

    std::vector<rigid_object*> fixed_objects = {
        plane,
        wall1,
        wall2,
        wall3,
        wall4
    };

    std::vector<rigid_object*> all_objects = {

    };


    // For speed computation
    double lastTime = glfwGetTime();
    double lastFrameTime = lastTime;
    int nbFrames = 0;

    double last_click_left_mouse = lastTime;

    gl_camera *camera = new gl_camera(window, programID);
    camera->setSpeed(10.0);

    do {

        // Measure speed
        double currentTime = glfwGetTime();
        double deltaTime = currentTime - lastFrameTime;
        lastFrameTime = currentTime;
        nbFrames++;
        if (currentTime - lastTime >= 1.0) {
            nbFrames = 0;
            lastTime += 1.0;
        }

        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Compute the MVP matrix from keyboard and mouse input
        camera->computeMatricesFromInputs();

        glm::mat4 ProjectionMatrix = camera->getProjectionMatrix();
        glm::mat4 ViewMatrix = camera->getViewMatrix();
        glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

        // Simple collision dection and resolution
        for (int i = 0 ; i < all_objects.size() ; ++i) {

            auto object = all_objects[i];

            if (object == plane) continue;

            // calculate the current velocity added by gravity
            if (object->current_translation[3][1] > BALL_RADIUS)
                object->current_velocity.y -= 10.8 * deltaTime;

            // if the object contact with other object
            for (int j = i + 1 ; j < all_objects.size() ; ++j) {

                if (all_objects[j] == plane) continue;

                glm::vec3 center1(all_objects[i]->current_translation[3]);
                glm::vec3 center2(all_objects[j]->current_translation[3]);

                glm::vec3 v2_to_v1 = center1 - center2;

                if (glm::length(v2_to_v1) >= 2.0 * BALL_RADIUS) continue;

                // reverse velocity in the direction of normal
                glm::vec3 contact_normal = glm::normalize(center1 - center2);

                auto v2_to_v1_normalized = glm::normalize(center1 - center2);

                float v1_at_direction = glm::dot(all_objects[i]->current_velocity,  v2_to_v1_normalized);
                float v2_at_direction = glm::dot(all_objects[j]->current_velocity,  v2_to_v1_normalized);

                glm::vec3 v1_other = all_objects[i]->current_velocity - v1_at_direction * v2_to_v1_normalized;
                glm::vec3 v2_other = all_objects[j]->current_velocity - v2_at_direction * v2_to_v1_normalized;

                // Conservation of momentum and Conservation of energy
                float a = v1_at_direction + v2_at_direction;
                float b = v1_at_direction * v1_at_direction + v2_at_direction * v2_at_direction;

                float v1 = 0.5 * (a + glm::sqrt(2 * b - a * a)), v2 = 0.5 * (a - glm::sqrt(2 * b - a * a));

                all_objects[i]->current_velocity = v1 * v2_to_v1_normalized + v1_other;
                all_objects[j]->current_velocity = v2 * v2_to_v1_normalized + v2_other;

                // change the direction of two objects' angular velocity
                // calculate the new velocity perpendicular to the normal
                auto axis1 = glm::cross(v2_to_v1_normalized, v1_other);
                glm::vec3 vertical_velocity1 = glm::vec3(0.0, all_objects[i]->current_velocity.y, 0.0);
                glm::vec3 horizontal_velocity1 = all_objects[i]->current_velocity - vertical_velocity1;

                if (glm::length(horizontal_velocity1) == 0.0) {
                    all_objects[i]->current_angular_velocity = {1.0, 0.0, 0.0, 0.0};
                } else {
                    all_objects[i]->current_angular_velocity = glm::vec4(-axis1.x, -axis1.y, -axis1.z, glm::length(horizontal_velocity1));
                }

                auto axis2 = glm::cross(v2_to_v1_normalized, v2_other);
                glm::vec3 vertical_velocity2 = glm::vec3(0.0, all_objects[j]->current_velocity.y, 0.0);
                glm::vec3 horizontal_velocity2 = all_objects[j]->current_velocity - vertical_velocity2;

                if (glm::length(horizontal_velocity2) == 0.0) {
                    all_objects[j]->current_angular_velocity = {1.0, 0.0, 0.0, 0.0};
                } else {
                    all_objects[j]->current_angular_velocity = glm::vec4(-axis2.x, -axis2.y, -axis2.z, glm::length(horizontal_velocity2));
                }
            }

            if (object->current_translation[3][0] <= -100 + BALL_RADIUS && object->current_velocity.x < 0) {
                glm::vec3 contact_normal(1.0, 0.0, 0.0);
                update_velocity_after_contact(contact_normal, object);
            }

            if (object->current_translation[3][0] >= 100 - BALL_RADIUS && object->current_velocity.x > 0) {
                glm::vec3 contact_normal(-1.0, 0.0, 0.0);
                update_velocity_after_contact(contact_normal, object);
            }

            if (object->current_translation[3][2] <= -100 + BALL_RADIUS && object->current_velocity.z < 0) {
                glm::vec3 contact_normal(0.0, 0.0, 1.0);
                update_velocity_after_contact(contact_normal, object);
            }

            if (object->current_translation[3][2] >= 100 - BALL_RADIUS && object->current_velocity.z > 0) {
                glm::vec3 contact_normal(0.0, 0.0, -1.0);
                update_velocity_after_contact(contact_normal, object);
            }

            // if the object contact the plane
            if (object->current_translation[3][1] <= BALL_RADIUS) {

                if (abs(object->current_velocity.y) < 0.1) {
                    // if the velocity is smaller than threshold
                    object->current_velocity.y = 0.0;
                    object->current_translation[3][1] = BALL_RADIUS;
                } else {
                    // recover the frame
                    object->current_translation[3][1] = BALL_RADIUS;
                    // reverse the velocity
                    object->current_velocity.y = -0.6 * object->current_velocity.y;
                }

                glm::vec3 vertical_velocity = glm::vec3(0.0, object->current_velocity.y, 0.0);
                glm::vec3 horizontal_velocity = object->current_velocity - vertical_velocity;

                // calculate the angular velocity based on the current contact
                if (glm::length(horizontal_velocity) == 0.0) {
                    object->current_angular_velocity = {1.0, 0.0, 0.0, 0.0};
                } else {
                    auto angular_axis = glm::cross(glm::vec3(0.0, 1.0, 0.0), horizontal_velocity);
                    object->current_angular_velocity = glm::vec4(angular_axis.x, angular_axis.y, angular_axis.z, glm::length(horizontal_velocity));
                }

                // reduce the horizontal velocity
                if (glm::length(horizontal_velocity) < 0.1f)
                    horizontal_velocity = glm::vec3(0.0);
                else
                    horizontal_velocity *= 0.5f;

                object->current_velocity = vertical_velocity + horizontal_velocity;
            }
        }


        for (auto each:fixed_objects) {
            each->current_rotation_in_quaternion = quatMultiply(angularVelocityToQuaternion(each->current_angular_velocity, deltaTime), each->current_rotation_in_quaternion);
            glm::mat4 ModelMatrix = each->current_translation * quaternionToRotation(each->current_rotation_in_quaternion);
            glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

            glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
            glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

            each->draw();
        }

        for (auto each:all_objects) {

            // move the object based on the velocity and then draw

            each->current_translation[3][0] += deltaTime * each->current_velocity.x;
            each->current_translation[3][1] += deltaTime * each->current_velocity.y;
            each->current_translation[3][2] += deltaTime * each->current_velocity.z;

            each->current_rotation_in_quaternion = quatMultiply(angularVelocityToQuaternion(each->current_angular_velocity, deltaTime), each->current_rotation_in_quaternion);
            glm::mat4 ModelMatrix = each->current_translation * quaternionToRotation(each->current_rotation_in_quaternion);
            glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

            glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
            glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

            each->draw();
        }

        // push a new ball into the scene
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && currentTime - last_click_left_mouse > 0.2f) {

            glm::vec3 position = camera->getPosition();
            glm::vec3 direction = camera->getDirection();
            position = position + 1.5f * direction;

            // get current direction
            auto new_ball = new rigid_object(ball_package);
            new_ball->current_translation[3] = {position.x, position.y , position.z, 1.0};
            new_ball->setTexture("texture/dog2.bmp");
            new_ball->current_velocity = 30.0f * camera->getDirection();

            all_objects.push_back(new_ball);

            last_click_left_mouse = currentTime;
        }

        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
            all_objects.clear();
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

