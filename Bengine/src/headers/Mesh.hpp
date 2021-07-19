#pragma once

#include "Transform.hpp"
#include "Rigidbody.hpp"

struct Mesh {
	const char* name;
	Transform transform;
	Rigidbody rigidbody;
	unsigned int textureID;
};