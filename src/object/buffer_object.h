//
// Created by zhengxiangyue on 9/16/18.
//

#ifndef XQ_VERTEXBUFFER_H
#define XQ_VERTEXBUFFER_H
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class gl_buffer {
private:
    unsigned int buffer_id;
    unsigned int buffer_type;
public:
    gl_buffer();
    gl_buffer(unsigned int buffer_type, const void *data, unsigned int size);
    ~gl_buffer();
    void enable();
    void disable();
    void deleteBuffer();
};


#endif //ANIMATION_VERTEXBUFFER_H
