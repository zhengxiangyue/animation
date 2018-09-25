//
// Created by zhengxiangyue on 9/16/18.
//

#include "VertexBuffer.h"
VertexBuffer::VertexBuffer(){}
VertexBuffer::VertexBuffer(const void *data, unsigned int size) {
    glGenBuffers(1, &m_renderer_id);
    glBindBuffer(GL_ARRAY_BUFFER, m_renderer_id);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
}

VertexBuffer::~VertexBuffer() {
    glDeleteBuffers(1, &m_renderer_id);
}

void VertexBuffer::bind() {
    glBindBuffer(GL_ARRAY_BUFFER, m_renderer_id);
}

void VertexBuffer::unbind() {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
