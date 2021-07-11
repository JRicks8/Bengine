#pragma once
#include <glm.hpp>
#include <vector>

typedef struct {
	glm::vec3 position;
	float fov;
	float cam_angle_vertical;
	float cam_angle_horizontal;
	float cam_near_clipping_plane;
	float cam_far_clipping_plane;
	float speed;
	float mouseSpeed;

	float height;
	float Xwidth;
	float Zdepth;

	glm::vec3 maxAABB;
	glm::vec3 minAABB;
} Player;