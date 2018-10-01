#include "objloader.h"

/**
 * load obj file with vertices, normal, texture coordinate
 * @param path
 * @param out_vertices
 * @param out_uvs
 * @param out_normals
 * @param scale
 * @return
 */
bool loadOBJ(
    const char *path,
    std::vector<glm::vec3> &out_vertices,
    std::vector<glm::vec2> &out_uvs,
    std::vector<glm::vec3> &out_normals,
    float scale = 1.0,
    glm::vec3 position = {0.0, 0.0, 0.0}
) {

    std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
    std::vector<glm::vec3> temp_vertices;
    std::vector<glm::vec2> temp_uvs;
    std::vector<glm::vec3> temp_normals;

    FILE *file = fopen(path, "r");
    if (file == nullptr) {
        std::cout << "Can't open the file " << path << std::endl;
        return false;
    }

    std::ifstream file_stream(path);
    std::string each_line;

    while (getline(file_stream, each_line)) {
        std::stringstream ss(each_line);
        std::string word;
        while (ss >> word) {
            if (word == "v") {
                glm::vec3 vertex;
                ss >> vertex.x >> vertex.y >> vertex.z;
                vertex = vertex * scale;
                vertex.x += position.x, vertex.y += position.y, vertex.z += position.z;
                temp_vertices.push_back(vertex);
            } else if (word == "vt") {
                glm::vec2 uv;
                ss >> uv.x >> uv.y;
                uv.y = -uv.y;
                temp_uvs.push_back(uv);
            } else if (word == "vn") {
                glm::vec3 normal;
                ss >> normal.x >> normal.y >> normal.z;
                temp_normals.push_back(normal);
            } else if (word == "f") {
                std::vector<std::string> polygons;
                std::string each_polygon;
                while (ss >> each_polygon) {
                    polygons.push_back(each_polygon);
                    // connect the first to the second and the third, then delete the second, until there is only three
                    while (polygons.size() >= 3) {
                        for (int i = 0; i < 3; ++i) {
                            std::stringstream polygon_stream(polygons[i]);
                            std::string index;
                            getline(polygon_stream, index, '/');
                            vertexIndices.push_back(stoi(index));
                            getline(polygon_stream, index, '/');
                            uvIndices.push_back(std::stoi(index));
                            getline(polygon_stream, index);
                            normalIndices.push_back(stoi(index));
                        }
                        polygons.erase(polygons.begin() + 1);
                    }
                }
            }
        }
    }

    // For each vertex of each triangle
    for (unsigned int i = 0; i < vertexIndices.size(); i++) {

        // Get the indices of its attributes
        unsigned int vertexIndex = vertexIndices[i];
        unsigned int uvIndex = uvIndices[i];
        unsigned int normalIndex = normalIndices[i];

        // Get the attributes thanks to the index
        glm::vec3 vertex = temp_vertices[vertexIndex - 1];
        glm::vec2 uv = temp_uvs[uvIndex - 1];
        glm::vec3 normal = temp_normals[normalIndex - 1];

        // Put the attributes in buffers
        out_vertices.push_back(vertex);
        out_uvs.push_back(uv);
        out_normals.push_back(normal);

    }
    fclose(file);
    return true;
}