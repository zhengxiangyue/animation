//
// Created by zhengxiangyue on 9/16/18.
//

#include "IndexBuffer.h"

IndexBuffer::IndexBuffer(){}

IndexBuffer::IndexBuffer(const void *data, unsigned int count): m_count(count) {
    glGenBuffers(1, &m_renderer_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_renderer_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, GL_STATIC_DRAW);

}

IndexBuffer::~IndexBuffer() {
    glDeleteBuffers(1, &m_renderer_id);
}

void IndexBuffer::bind() const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_renderer_id);
}

void IndexBuffer::unbind() const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
