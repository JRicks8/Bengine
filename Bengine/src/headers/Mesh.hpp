#pragma once

#include "LinearMath/btTransform.h"

struct Mesh {
	const char* name{ "err" };
	unsigned int meshIndex{ 0 };
	unsigned int bufferIndex{ 0 };
	unsigned int textureID{ 0 };
	btTransform transform;
};