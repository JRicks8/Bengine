#pragma once

#include "LinearMath/btTransform.h"
#include "glm.hpp"

class Mesh {
public:
	const char* name = "err";
	bool empty = false;
	unsigned int meshIndex = 0;
	unsigned int bufferIndex = 0;
	glm::vec4 color;
	btTransform transform;
};