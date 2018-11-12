//
// Created by zhengxiangyue on 11/4/18.
//

#ifndef XQ_XQSEGMENT_H
#define XQ_XQSEGMENT_H

#include <vector>
#include "rigid_object.h"

template<typename _rotation_type>
class action {
public:
    float start_time;
    float end_time;

    std::vector<glm::vec4> control_points;
    std::vector<_rotation_type> key_rotations;

    action(float current_time, std::vector<glm::vec4> _control_points, std::vector<_rotation_type> _key_rotations, bool repeat=false): control_points(_control_points), key_rotations(_key_rotations){
        start_time = current_time;
        if (!repeat) {
            end_time = current_time + 0.2 * (glm::max(control_points.size(), key_rotations.size()) - 3);
        } else {
            end_time = current_time - 0.1;
        }
    }

};

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
class hierarchical_node : public rigid_object {
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
    std::vector<action<_rotation_type>> actions;

    /**
     * Decide the rotation of the key frames
     */
    std::vector<_rotation_type> key_rotations;

    /**
     *
     */
    glm::mat4 current_translation;

    /**
     *
     */
    glm::mat4 current_rotation;

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
        glm::vec3 child_joint = glm::vec3(0.0, 0.0, 0.0),
        glm::vec4 quat = getQuatFromIntuition(0.0, glm::vec3(1.0, 0.0, 0.0))) : polygon_object(path, position, scale, texture_path, quat) {

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
        float each_rotation_duration = 0.2;

        // If this is a root node, the movement is based on the key input, we should always return the global rotation
        // If not, we should return the locaol rotation

        glm::mat4 _rotation = glm::mat4(1.0);
        for (auto each:actions) {
            auto _keys = each.key_rotations;
            if (_keys.size() < 4) continue;
            current_time -= each.start_time;
            _keys.erase(_keys.begin()), _keys.erase(_keys.end() - 1);
            float current_rotation_time = fmod(current_time, each_rotation_duration * (_keys.size() - 1)) / each_rotation_duration;
            _rotation = expressToRotation(getInterpolateExpress<_rotation_type>(current_rotation_time, _keys)) * _rotation;
        }
        return _rotation;
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
            return getTranslationMatrix(current_time) * getRotationMatrix(current_time) * current_translation * current_rotation;
        auto parent_transformation = parent_ptr->getTransformationMatrix(current_time);
        auto local_rotate = getRotationMatrix(current_time);
        return parent_transformation * T1 * local_rotate * T2;
    }

    /**
     *
     * @param current_time
     * @return
     */
    glm::mat4 getTranslationMatrix(float current_time) {
        float each_path_duration = 0.2;
        glm::mat4 _translation(1.0);
        for (auto each: actions) {
            if (each.control_points.size() < 4)
                continue;
            float current_path_time = fmod(current_time - each.start_time, each_path_duration * (each.control_points.size() - 3)) / each_path_duration;
            auto _each_translate = getInterpolateTranslationMatrix(
                current_path_time,
                blending_matrices[0],
                each.control_points
            );
            _translation = _each_translate * _translation;
        }

        return _translation;
    }
};


#endif //XQ_XQSEGMENT_H