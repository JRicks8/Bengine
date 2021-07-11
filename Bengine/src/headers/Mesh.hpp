#pragma once
#include <glm.hpp>
#include <vector>

typedef struct {
	const char* name;
	glm::vec3 modelPosition_WorldSpace;
	unsigned int textureID;
	glm::vec3 maxAABB;
	glm::vec3 minAABB;
} Mesh;