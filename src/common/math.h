//
// Created by zhengxiangyue on 9/28/18.
//
#ifndef ANIMATION_MATH_H
#define ANIMATION_MATH_H

#include <vector>
#include "gl_util.h"

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
glm::vec4 getQuatFromIntuition(float degree, glm::vec3 axis) {
    float hsar = sin(glm::radians(degree / 2));
    axis = normalize(axis);
    return glm::vec4(axis.x * hsar, axis.y * hsar, axis.z * hsar, cos(glm::radians(degree / 2)));
}

glm::vec4 getQuatFromTwoVec3(glm::vec3 start, glm::vec3 dest) {
    return getQuatFromIntuition(glm::degrees(glm::angle(start, dest)), glm::cross(start, dest));
}

/**
 * quat multiplication
 * @param q1
 * @param q2
 * @return
 */
glm::vec4 quatMultiply(glm::vec4 q1, glm::vec4 q2) {
    float w1 = q1.w, w2 = q2.w;
    glm::vec3 v1(q1.x, q1.y, q1.z), v2(q2.x, q2.y, q2.z);
    float w = q1.w * q2.w - (q1.x * q2.x + q1.y * q2.y + q1.z * q2.z);
    glm::vec3 v1xv2(q1.y * q2.z - q1.z * q2.y, q1.z * q2.x - q1.x * q2.z, q1.x * q2.y - q1.y * q2.x);
    glm::vec3 v = w1 * v2 + w2 * v1 + v1xv2;
    return glm::vec4(v.x, v.y, v.z, w);
}

/**
 * Quaternion to rotation matrix
 * @param vec
 * @return
 */
glm::mat4 expressToRotation(glm::vec4 vec) {
    vec = normalize(vec);
    float x = vec.x, y = vec.y, z = vec.z, w = vec.w;

    return transpose(glm::mat4({
                                   {1 - 2 * y * y - 2 * z * z, 2 * x * y - 2 * w * z,     2 * x * z + 2 * w * y},
                                   {2 * x * y + 2 * w * z,     1 - 2 * x * x - 2 * z * z, 2 * y * z - 2 * w * x},
                                   {2 * x * z - 2 * w * y,     2 * y * z + 2 * w * x,     1 - 2 * x * x - 2 * y * y},
                               }));
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
glm::mat4 expressToRotation(glm::vec3 euler) {
    auto rz = transpose(glm::mat4({
                                      {cos(euler.z), -sin(euler.z), 0, 0},
                                      {sin(euler.z), cos(euler.z),  0, 0},
                                      {0,            0,             1, 0},
                                      {0,            0,             0, 1}
                                  }));

    auto rx = transpose(glm::mat4({
                                      {cos(euler.x),  0, sin(euler.x), 0},
                                      {0,             1, 0,            0},
                                      {-sin(euler.x), 0, cos(euler.x), 0},
                                      {0,             0, 0,            1}
                                  }));

    auto ry = transpose(glm::mat4({
                                      {1, 0,            0,             0},
                                      {0, cos(euler.y), -sin(euler.y), 0},
                                      {0, sin(euler.y), cos(euler.y),  0},
                                      {0, 0,            0,             1}
                                  }));

    return rx * ry * rz;
}


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
 * 4 control_points determines one curve
 *
 * if there are 4 control points, time belongs to 0 and 1
 * if there are 5 control points, time belongs to 0 and 2
 *
 * @param time
 * @param M
 * @param control_points
 * @return
 */
glm::mat4 getInterpolateTranslationMatrix(float time, glm::mat4 M, std::vector<glm::vec4> control_points) {

    int start_index = floor(time);
    if (start_index >= control_points.size() - 3)
        return glm::mat4(1.0);

    time = fmod(time, 1.0);
    glm::vec4 position = glm::vec4(time * time * time, time * time, time, 1)
                         * M
                         * transpose(glm::mat4(control_points[start_index], control_points[start_index + 1], control_points[start_index + 2], control_points[start_index + 3]));

    glm::vec4 d_speed = glm::vec4(3 * time * time, 2 * time, 1.0, 0.0)
                         * M
                         * transpose(glm::mat4(control_points[start_index], control_points[start_index + 1], control_points[start_index + 2], control_points[start_index + 3]));


    return transpose(glm::mat4({
                                   {1.0, 0.0, 0.0, position.x},
                                   {0.0, 1.0, 0.0, position.y},
                                   {0.0, 0.0, 1.0, position.z},
                                   {0.0, 0.0, 0.0, 1.0},
                               }));
}

/**
 * given A and B, calculate the rotation matrix which convert A to B
 * @param start
 * @param result
 * @return
 */
glm::mat4 getRotationFromTwoVec(glm::vec3 start, glm::vec3 result) {
    glm::vec3 normal = glm::cross(glm::normalize(start), glm::normalize(result));
    float sin_angle = glm::length(normal);
    return expressToRotation(getQuatFromIntuition(asin(sin_angle), normal));
}

/**
 * convert quaternian to a rotation matrix
 * @param vec4
 * @return
 */
glm::mat4 quaternionToRotation(vec4 vec) {
    vec = normalize(vec);
    float x = vec.x, y = vec.y, z = vec.z, w = vec.w;

    return transpose(mat4({
                              {1 - 2 * y * y - 2 * z * z, 2 * x * y - 2 * w * z,     2 * x * z + 2 * w * y},
                              {2 * x * y + 2 * w * z,     1 - 2 * x * x - 2 * z * z, 2 * y * z - 2 * w * x},
                              {2 * x * z - 2 * w * y,     2 * y * z + 2 * w * x,     1 - 2 * x * x - 2 * y * y},
                          }));
}

glm::vec4 angularVelocityToQuaternion(glm::vec4 angular_velocity, float delta_time) {
    return getQuatFromIntuition(delta_time * glm::degrees(angular_velocity.w), glm::vec3(angular_velocity));
}

#endif