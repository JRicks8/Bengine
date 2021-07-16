#pragma once

#include <glm.hpp>
#include <gtx/quaternion.hpp>
#include "BoxShape.hpp"

typedef struct {
	glm::vec3 position;        // x(t)
	glm::vec3 linearMomentum;  // P(t)
	glm::vec3 angularMomentum; // L(t)
	glm::quat orientation;     // q(t)

	glm::mat3 rotationMatrix;  // R(t)
	glm::mat3 ITensor;         // inverse inertia
	glm::vec3 linearVelocity;  // v(t)
	glm::vec3 angularVelocity; // omega(t)

	glm::vec3 force;           // F(t)
	glm::vec3 torque;          // tau(t)
	BoxShape shape;
} Rigidbody;