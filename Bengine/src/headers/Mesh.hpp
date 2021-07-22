#pragma once

#include "Transform.hpp"
#include "LinearMath/btTransform.h"

struct Mesh {
	const char* name{ "err" };
	btTransform transform;
	unsigned int textureID{ 0 };
};