#pragma once

#include "LinearMath/btTransform.h"

class Mesh {
public:
	const char* name = "err";
	bool empty = false;
	unsigned int meshIndex = 0;
	unsigned int bufferIndex = 0;
	unsigned int textureID = 0;
	btTransform transform;
};