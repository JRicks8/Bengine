#pragma once

#include <glm.hpp>

class Collider {
public:

	Collider() {}

	Collider(glm::vec3 p) :
		position(p) {}

	void SetPosition(glm::vec3 p) { position = p; }
	glm::vec3 GetPosition() const { return position; }

	void SetType(int t) { type = t; }
	int GetType() const { return type; }

	enum {
		TYPE_UNDECLARED,
		TYPE_BOX,
		TYPE_SPHERE
	};

private:
	glm::vec3 position{ glm::vec3(0) };
	int type{ TYPE_UNDECLARED };
};