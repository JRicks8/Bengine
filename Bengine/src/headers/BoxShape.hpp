#pragma once
#include <glm.hpp>

typedef struct {
	float width, height, depth;
	float mass;
	glm::mat3 iBody, iBodyInv; //body inertia and inverse body inertia
} BoxShape;