#pragma once

#include <glm.hpp>

#include "Collider.hpp"

class SphereCollider : Collider {
public:
	SphereCollider() {
		SetType(TYPE_SPHERE);
	}

	SphereCollider(float r) :
		radius(r) {
		SetType(TYPE_SPHERE);
	}

	float GetRadius() const { return radius; }
	void SetRadius(float r) { radius = r; }

	glm::mat3 CalculateInertia(float mass) {
		return glm::mat3(
			0.4f * mass * (radius * radius), 0, 0,
			0, 0.4f * mass * (radius * radius), 0,
			0, 0, 0.4f * mass * (radius * radius)) * (mass / 12);
	}

private:
	float radius{ 1.0f };
};