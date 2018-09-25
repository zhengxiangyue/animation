//
// Created by zhengxiangyue on 9/16/18.
//

#ifndef ANIMATION_VERTEXBUFFERLAYOUT_H
#define ANIMATION_VERTEXBUFFERLAYOUT_H

#include <GLFW/glfw3.h>
#include <vector>

struct VertexBufferElement {
    unsigned int count;
    unsigned int type;
    unsigned char normalized;

    static unsigned int getSizeOfType(unsigned int type) {
        switch (type){
            case GL_FLOAT:
                return 4;
            case GL_UNSIGNED_INT:
                return 4;
            case GL_UNSIGNED_BYTE:
                return 1;
            default:
                return 0;
        }

    }
};

using namespace std;

class VertexBufferLayout {
    vector<VertexBufferElement> m_elements;
    unsigned int m_stride;
public:
    VertexBufferLayout() : m_stride(0) {}

    template<typename T>
    void push(int count) {
        static_assert(false);
    }

    template<>
    void push<float>(int count) {
        m_elements.push_back({GL_FLOAT, count, GL_FALSE});
        m_stride += sizeof(GLfloat);
    }

    template<>
    void push<unsigned int>(int count) {
        m_elements.push_back({GL_UNSIGNED_INT, count, GL_FALSE});
        m_stride += sizeof(GLint);
    }

    template<>
    void push<unsigned char>(int count) {
        m_elements.push_back({GL_UNSIGNED_BYTE, count, GL_TRUE});
        m_stride += sizeof(GL_BYTE);
    }

    inline unsigned int getStride() const {
        return m_stride;
    }

    inline const std::vector<VertexBufferElement> getElements() const {
        return m_elements;
    }
};


#endif //ANIMATION_VERTEXBUFFERLAYOUT_H
