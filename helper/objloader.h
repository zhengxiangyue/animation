#ifndef OBJLOADER_H
#define OBJLOADER_H
#include <vector>
#include <stdio.h>
#include <string>
#include <cstring>

#include <glm/glm.hpp>
#include <sstream>
#include <fstream>
#include <iostream>

bool loadOBJ(
	const char * path, 
	std::vector<glm::vec3> & out_vertices, 
	std::vector<glm::vec2> & out_uvs, 
	std::vector<glm::vec3> & out_normals,
	float scale,
	glm::vec3 position
);

#endif