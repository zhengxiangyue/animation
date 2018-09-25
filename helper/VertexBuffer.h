//
// Created by zhengxiangyue on 9/16/18.
//

#ifndef ANIMATION_VERTEXBUFFER_H
#define ANIMATION_VERTEXBUFFER_H
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class VertexBuffer {
private:
    unsigned int m_renderer_id;
public:
    VertexBuffer();
    VertexBuffer(const void *data, unsigned int size);

    ~VertexBuffer();

    void bind();

    void unbind();

};


#endif //ANIMATION_VERTEXBUFFER_H
