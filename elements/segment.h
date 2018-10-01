//
// Created by zhengxiangyue on 9/29/18.
//

#ifndef ANIMATION_SEGMENT_H
#define ANIMATION_SEGMENT_H

#include <vector>
#include "rigid_object.h"

std::vector<glm::mat4> blending_matrices(
    {
        // Catmull-Rom
        transpose(glm::mat4(
            {
                {-0.5, 1.5,  -1.5, 0.5},
                {1.0,  -2.5, 2.0,  -0.5},
                {-0.5, 0.0,  0.5,  0.0},
                {0.0,  1.0,  0.0,  0.0}
            })),
        // B spline
        transpose(glm::mat4(
            {
                {-1.0 / 6.0, 3.0 / 6.0,  -3.0 / 6.0, 1.0 / 6.0},
                {3.0 / 6.0,  -6.0 / 6.0, 3.0 / 6.0,  0.0},
                {-3.0 / 6.0, 0.0 / 6.0,  3.0 / 6.0,  0.0},
                {1.0 / 6.0,  4.0 / 6.0,  1.0 / 6.0,  0.0}
            }))
    });

template<typename _rotation_type>
class hierarchical_node : public polygon_object {
private:
    /**
     * T1 * R * T2
     */
    glm::mat4 T1;
    glm::mat4 T2;
public:
    /**
     * If this object has a parent object, the final transformation
     * is multiply the influence of the parent
     */
    hierarchical_node<_rotation_type> *parent_ptr;

    /**
     * The parent joint point is a local position of the joint
     * the world coordinates of this point is the same as the
     * parent segment's child_joint_point world coordinates
     */
    glm::vec3 parent_joint_point;

    /**
     * this child joint position help to decide the rotation original.
     * we first translate all object points so that the joint point comes
     * to the original point, then do the location rotation
     */
    glm::vec3 child_joint_point;

    /**
     * Decide the translation of the key frames
     * If this is a child node, it shouldn't have
     * any control_points
     */
    std::vector<glm::vec4> control_points;

    /**
     * Decide the rotation of the key frames
     */
    std::vector<_rotation_type> key_rotations;

    /**
     *
     * @param parent
     * @param path
     * @param position
     * @param scale
     * @param texture_path
     * @param parent_joint
     * @param child_joint
     */
    hierarchical_node(
        hierarchical_node<_rotation_type> *parent,
        const char *path,
        glm::vec3 position,
        float scale = 1.0,
        const char *texture_path = "",
        glm::vec3 parent_joint = glm::vec3(0.0, 0.0, 0.0),
        glm::vec3 child_joint = glm::vec3(0.0, 0.0, 0.0)) : polygon_object(path, position, scale, texture_path) {

        parent_ptr = parent;
        parent_joint_point = parent_joint;
        child_joint_point = child_joint;
        T2 = parent_ptr ? glm::transpose(glm::mat4(
            {
                1.0, 0.0, 0.0, -child_joint_point.x,
                0.0, 1.0, 0.0, -child_joint_point.y,
                0.0, 0.0, 1.0, -child_joint_point.z,
                0.0, 0.0, 0.0, 1.0,
            })) : glm::mat4(1.0);
        T1 = parent_ptr ? glm::transpose(glm::mat4(
            {
                1.0, 0.0, 0.0, parent_joint_point.x,
                0.0, 1.0, 0.0, parent_joint_point.y,
                0.0, 0.0, 1.0, parent_joint_point.z,
                0.0, 0.0, 0.0, 1.0,
            })) : glm::mat4(1.0);
    }

    /**
     * Return the
     * @param current_time
     * @return
     */
    glm::mat4 getRotationMatrix(float current_time) {
        if (key_rotations.size() < 4)
            return glm::mat4(1.0);
        float each_rotation_duration = 0.2;
        auto _keys = key_rotations;
        _keys.erase(_keys.begin()), _keys.erase(_keys.end() - 1);
        float current_rotation_time = fmod(current_time, each_rotation_duration * (_keys.size() - 1)) / each_rotation_duration;
        return expressToRotation(getInterpolateExpress<_rotation_type>(current_rotation_time, _keys));
    }

    /**
     * If this a parent object, return T * R directly
     * Or get parent transformation and multiply local TRT
     * todo: calculate from top and cache the matrix
     * @param current_time
     * @return
     */
    glm::mat4 getTransformationMatrix(float current_time) {
        if (parent_ptr == nullptr)
            return getTranslationMatrix(current_time) * getRotationMatrix(current_time);
        auto parent_transformation = parent_ptr->getTransformationMatrix(current_time);
        auto local_rotate = getRotationMatrix(current_time);
        return parent_transformation * T1 * local_rotate * T2;
    }

    glm::vec3 getDirection(float current_time) {
        if (control_points.size() < 4)
            return glm::vec3(1.0);
        float each_path_duration = 0.2;
        float current_path_time = fmod(current_time, each_path_duration * (control_points.size() - 3)) / each_path_duration;

        return getInBetweenSpeed(current_path_time, blending_matrices[0], control_points);
    }

    /**
     *
     * @param current_time
     * @return
     */
    glm::mat4 getTranslationMatrix(float current_time) {

        if (control_points.size() < 4)
            return glm::mat4(1.0);
        float each_path_duration = 0.2;
        float current_path_time = fmod(current_time, each_path_duration * (control_points.size() - 3)) / each_path_duration;

        return getInterpolateTranslationMatrix(
            current_path_time,
            blending_matrices[0],
            control_points
        );
    }

    /**
     * T1 * R * T2
     * @param eurler
     * @return
     */
//    glm::mat4 getTransformMatrix(glm::vec3 eurler) {
//        if (parent == nullptr)
//            return RotationMatrix * TranslationMatrix;
//
//        auto parent_transform_matrix = parent->getTransformMatrix(glm::vec3(0.0, 0.0, 0.0));
//
//        glm::vec3 T1_vector = parent_joint_point - parent->child_joint_point;
//
//        glm::mat4 T1 = glm::transpose(glm::mat4({
//                                                        {1.0, 0.0, 0.0, T1_vector.x},
//                                                        {0.0, 1.0, 0.0, T1_vector.y},
//                                                        {0.0, 0.0, 1.0, T1_vector.z},
//                                                        {0.0, 0.0, 0.0, 1.0}
//                                                }));
//
//        glm::mat4 T2 = glm::transpose(glm::mat4({
//                                                        {1.0, 0.0, 0.0, -parent_joint_point.x},
//                                                        {0.0, 1.0, 0.0, -parent_joint_point.y},
//                                                        {0.0, 0.0, 1.0, -parent_joint_point.z},
//                                                        {0.0, 0.0, 0.0, 1.0}
//                                                }));
//
//
//        return parent_model_matrix * T1 * expressToRotation(eurler) * T2;
//    }

};

#endif //ANIMATION_SEGMENT_H
