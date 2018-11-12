//
// Created by zhengxiangyue on 9/16/18.
//

#include "buffer_object.h"

gl_buffer::gl_buffer() {

}

gl_buffer::gl_buffer(unsigned int buffer_type, const void *data, unsigned int size) {
    this->buffer_type = buffer_type;
    glGenBuffers(1, &buffer_id);
    glBindBuffer(buffer_type, buffer_id);
    glBufferData(buffer_type, size, data, GL_STATIC_DRAW);
}

gl_buffer::~gl_buffer() {

}

void gl_buffer::enable() {
    glBindBuffer(buffer_type, buffer_id);
}

void gl_buffer::disable() {
    glBindBuffer(buffer_type, buffer_id);
}

void gl_buffer::deleteBuffer() {
    glDeleteBuffers(1, &buffer_id);
}
