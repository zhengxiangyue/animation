//
// Created by zhengxiangyue on 11/4/18.
//

#ifndef XQ_XQRIGIDOBJECT_H
#define XQ_XQRIGIDOBJECT_H

#include "gl_object.h"

class rigid_object : public gl_object {
public:
    rigid_object(simpleBufferPackage *package) : gl_object(package) {
        current_rotation_in_quaternion = getQuatFromIntuition(0.0f, {1.0, 0.0, 0.0});
        current_angular_velocity = glm::vec4(0.0, 1.0, 0.0, 0.0);
    }

    // x, y, z, speed
    glm::vec4 current_angular_velocity;

    glm::mat4 current_translation;

    glm::vec3 getVec3Translation() {
        return glm::vec3(current_translation[3][0], current_translation[3][1], current_translation[3][2]);
    }

    glm::vec4 current_rotation_in_quaternion;

    glm::vec3 current_velocity;

    virtual void updateTranslation(float deltaTime) {
        current_translation[3][0] += deltaTime * current_velocity.x;
        current_translation[3][1] += deltaTime * current_velocity.y;
        current_translation[3][2] += deltaTime * current_velocity.z;
    };

};


#endif //XQ_XQRIGIDOBJECT_H
