#include "headers/Mesh.hpp"
#include "headers/Player.hpp"
#include <iostream>

void GenerateAABBfromVBO(std::vector<float> &inVBO, glm::vec3& outMaxAABB, glm::vec3 &outMinAABB, unsigned int vertex_size) {
	glm::vec3 Max(inVBO[0], inVBO[1], inVBO[2]);
	glm::vec3 Min = Max;

	for (unsigned int i = vertex_size; i < inVBO.size(); i += vertex_size) {

		Max.x = glm::max(inVBO[i], Max.x);
		Max.y = glm::max(inVBO[i + 1], Max.y);
		Max.z = glm::max(inVBO[i + 2], Max.z);

		Min.x = glm::min(inVBO[i], Min.x);
		Min.y = glm::min(inVBO[i + 1], Min.y);
		Min.z = glm::min(inVBO[i + 2], Min.z);
	}

	outMaxAABB = Max;
	outMinAABB = Min;
}

bool intersectPlayerAABB(Mesh mesh, Player player) {

	glm::vec3 aMax = mesh.maxAABB + mesh.modelPosition_WorldSpace;
	glm::vec3 aMin = mesh.minAABB + mesh.modelPosition_WorldSpace;
	glm::vec3 bMax = player.maxAABB + player.position;
	glm::vec3 bMin = player.minAABB + player.position;

	bool intersecting = (aMin.x <= bMax.x && aMax.x >= bMin.x) &&
						(aMin.y <= bMax.y && aMax.y >= bMin.y) &&
						(aMin.z <= bMax.z && aMax.z >= bMin.z);
	//if (intersecting)
	//	std::cout << "Found intersection with " << mesh.name << " AABB values of:"
	//	<< aMax.x << " " << aMax.y << " " << aMax.z << " " << aMin.x << " " << aMin.y << " " << aMin.z << " and player aabb of: "
	//	<< bMax.x << " " << bMax.y << " " << bMax.z << " " << bMin.x << " " << bMin.y << " " << bMin.z << std::endl;

	return intersecting;
}

void CheckForPlayerCollisions(std::vector<Mesh> &meshes, Player &player) {

	for (int i = 0; i < meshes.size(); i++){
		if (intersectPlayerAABB(meshes[i], player)) {
			glm::vec3 directionToMesh = glm::normalize(player.position - meshes[i].modelPosition_WorldSpace);
			std::cout << directionToMesh.x << " " << directionToMesh.y << " " << directionToMesh.z << std::endl;
		}
	}
}