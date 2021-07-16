#pragma once

#include <glm.hpp>

#include "Rigidbody.hpp"

typedef struct Contact {
	Rigidbody a; // body containing colliding vertex (used only in vertex-face collision)
	Rigidbody b; // body containing colliding face (used only in vertex-face collision)

	glm::vec3 vertexPosition; // world space (used only in vertex-face collision)
	glm::vec3 normal; // normal of colliding face in vertex-face collision, cross of edgeA and edgeB in edge-edge collisions
	glm::vec3 edgeA; // edge direction of body a (used only in edge-edge collision)
	glm::vec3 edgeB; // edge direction of body b (used only in edge-edge collision)

	bool vfContact; // TRUE if vertex/face contact
};