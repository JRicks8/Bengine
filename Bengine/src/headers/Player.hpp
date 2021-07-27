#pragma once

#include "LinearMath/btTransform.h"

#include <glm.hpp>

typedef struct {
	glm::vec3 position;
	float fov;
	float cam_angle_vertical;
	float cam_angle_horizontal;
	float cam_near_clipping_plane;
	float cam_far_clipping_plane;
	float speed;
	float mouseSpeed;
	btTransform transform;
} Player;