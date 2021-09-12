#pragma once

#include "LinearMath/btTransform.h"
#include <glm.hpp>

typedef struct Player {
	glm::vec3 position;
	glm::vec3 headOffset{ glm::vec3(0.0f, 0.5f, 0.0f) };
	float fov;
	float cam_angle_vertical;
	float cam_angle_horizontal;
	float cam_near_clipping_plane;
	float cam_far_clipping_plane;
	glm::vec3 cam_offset;
	float speed;
	float mouseSpeed;

	bool grounded;

	btTransform transform;

	const glm::vec3 GetHeadAnchor() {
		return position + headOffset;
	}

	const glm::vec3 GetRArmAnchor(glm::vec3 front, glm::vec3 right) {
		return (position + cam_offset) + front * 2.0f + right * 1.5f - glm::vec3(0.0f, 0.5f, 0.0f);
	}

	const glm::vec3 GetLArmAnchor(glm::vec3 front, glm::vec3 right) {
		return (position + cam_offset) + front * 2.0f - right * 1.5f - glm::vec3(0.0f, 0.5f, 0.0f);
	}
};