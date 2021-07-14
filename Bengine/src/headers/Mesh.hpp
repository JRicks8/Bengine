#pragma once

#include "Rigidbody.hpp"

typedef struct Mesh {
	const char* name;
	glm::vec3 modelPosition_WorldSpace;
	unsigned int textureID;
	glm::vec3 maxAABB;
	glm::vec3 minAABB;

	Rigidbody rigidbody;
};