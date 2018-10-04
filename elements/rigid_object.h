//
// Created by zhengxiangyue on 9/28/18.
//

#include <vector>
#include <unordered_set>
#include "../common/gl_util.h"
#include "../common/math.h"
#include "../helper/shader.h"
#include "../helper/textureLoader.h"
#include "../helper/controls.h"
#include "../helper/objLoader.h"
#include "../helper/vboIndexer.h"
#include "../helper/vertexBuffer.h"
#include "../helper/IndexBuffer.h"

/**
 * scene object
 */
class polygon_object {
public:
    std::vector<glm::vec3> vertices, normals, indexed_vertices, indexed_normals;
    std::vector<glm::vec2> uvs, indexed_uvs;
    std::vector<unsigned short> indices;

    VertexBuffer *vb;
    VertexBuffer *uvb;
    VertexBuffer *normalvb;

    IndexBuffer *eleib;
    unsigned int texture;

    polygon_object(
            const char *path,
            glm::vec3 position,
            float scale = 1.0,
            const char *texture_path = ""
    ) {
        loadOBJ(path, vertices, uvs, normals, scale, position);
        indexVBO(vertices, uvs, normals, indices, indexed_vertices, indexed_uvs, indexed_normals);

        vb = new VertexBuffer(&indexed_vertices[0], indexed_vertices.size() * sizeof(glm::vec3));
        uvb = new VertexBuffer(&indexed_uvs[0], indexed_uvs.size() * sizeof(glm::vec2));
        normalvb = new VertexBuffer(&indexed_normals[0], indexed_normals.size() * sizeof(glm::vec3));
        eleib = new IndexBuffer(&indices[0], indices.size() * sizeof(unsigned short));

        texture = loadBMP_custom(texture_path);
    }
};