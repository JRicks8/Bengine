#pragma once

#include <glm.hpp>

struct BoxCollider {
	BoxCollider() :
		width(1.0f),
		height(1.0f),
		depth(1.0f) {}

	BoxCollider(float w, float h, float d) :
		width(w),
		height(h),
		depth(d) {}

	glm::mat3 CalculateInertia(float mass) {
		return glm::mat3(
			height * height + depth * depth, 0, 0,
			0, width * width + depth * depth, 0,
			0, 0, width * width + height * height) * (mass / 12);
	}

	float width{ 1.0f };
	float height{ 1.0f };
	float depth{ 1.0f };
};