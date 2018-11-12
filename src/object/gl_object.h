//
// Created by zhengxiangyue on 11/3/18.
//

#ifndef XQ_XQOBJECT_H
#define XQ_XQOBJECT_H

#include <vector>
#include <glm/glm.hpp>

#include "buffer_object.h"
#include "../helper/objLoader.h"
#include "../helper/vbo_indexer.h"
#include "../helper/texture.h"

struct simpleBufferPackage{
    unsigned int array_size;
    gl_buffer *position_buffer, *normal_buffer, *uv_buffer, *element_buffer;
    simpleBufferPackage(gl_buffer *position_buffer, gl_buffer *normal_buffer, gl_buffer *uv_buffer, gl_buffer *element_buffer, unsigned int array_size):
        position_buffer(position_buffer), normal_buffer(normal_buffer), uv_buffer(uv_buffer), element_buffer(element_buffer), array_size(array_size){}
};

/**
 *
 * @param file_path
 * @param local_transformation
 */
simpleBufferPackage* loadSimpleObjFileToIndexedBuffer(const char *file_path, const glm::mat4 &local_init_transformation) {

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;
    bool res = loadOBJ(file_path, vertices, uvs, normals, local_init_transformation);

    std::vector<unsigned short> indices;
    std::vector<glm::vec3> indexed_vertices;
    std::vector<glm::vec2> indexed_uvs;
    std::vector<glm::vec3> indexed_normals;
    indexVBO(vertices, uvs, normals, indices, indexed_vertices, indexed_uvs, indexed_normals);

    unsigned int element_buffer_size = indices.size() * sizeof(unsigned short);

    return new simpleBufferPackage(
        new gl_buffer(GL_ARRAY_BUFFER, &indexed_vertices[0], indexed_vertices.size() * sizeof(glm::vec3)),
        new gl_buffer(GL_ARRAY_BUFFER, &indexed_normals[0], indexed_normals.size() * sizeof(glm::vec3)),
        new gl_buffer(GL_ARRAY_BUFFER, &indexed_uvs[0], indexed_uvs.size() * sizeof(glm::vec2)),
        new gl_buffer(GL_ELEMENT_ARRAY_BUFFER, &indices[0], element_buffer_size),
        element_buffer_size
        );
}

class gl_object {
public:

    /**
     * Three basic buffers owned by an object
     */
    gl_buffer *position_buffer, *normal_buffer, *uv_buffer, *element_buffer;

    /**
     *
     */
    int element_buffer_size;

    unsigned int current_texture_id;
    void indicateTexture(unsigned int texture_id) {
        current_texture_id = texture_id;
    }

    /**
     *
     * @param file_path
     * @param local_init_transformation
     */
    gl_object(simpleBufferPackage* package) {
        position_buffer = package->position_buffer;
        normal_buffer = package->normal_buffer;
        uv_buffer = package->uv_buffer;
        element_buffer = package->element_buffer;
        element_buffer_size = package->array_size;
    }

    unsigned int texture_object_id;
    void setTexture(const char* path) {
        texture_object_id = loadBMP_custom(path);
    }

    /**
     *
     */
    void draw() {

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture_object_id);
        glUniform1i(current_texture_id, 0);

        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(0);
        position_buffer->enable();
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        // 2nd attribute buffer : UVs
        glEnableVertexAttribArray(1);
        uv_buffer->enable();
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

        // 3rd attribute buffer : normals
        glEnableVertexAttribArray(2);
        normal_buffer->enable();
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        // Index buffer
        element_buffer->enable();

        // Draw the triangles !
        glDrawElements(GL_TRIANGLES, element_buffer_size, GL_UNSIGNED_SHORT, (void*)0);

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);
    }

    /**
     *
     */
    gl_object() {}
};




#endif //XQ_XQOBJECT_H
