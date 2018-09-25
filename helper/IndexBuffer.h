//
// Created by zhengxiangyue on 9/16/18.
//

#ifndef ANIMATION_INDEXBUFFER_H
#define ANIMATION_INDEXBUFFER_H
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class IndexBuffer {
private:
    unsigned int m_renderer_id;
    unsigned int m_count;
public:
    IndexBuffer();
    IndexBuffer(const void *data, unsigned int count);

    ~IndexBuffer();

    void bind() const ;

    void unbind() const ;

};


#endif //ANIMATION_INDEXBUFFER_H
