//
// Created by zhengxiangyue on 11/4/18.
//

#include <glm/glm.hpp>

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