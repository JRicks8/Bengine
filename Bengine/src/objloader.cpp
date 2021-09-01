#include <vector>
#include <string>
#include <glm.hpp>
#include <iostream>

bool loadOBJ(const char* path, std::vector<float>& out_vertices) {

	std::vector<glm::vec3> out_positions;
	std::vector<glm::vec2> out_uvs;
	std::vector<glm::vec3> out_normals;
	std::vector<unsigned int> positionIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> temp_positions;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;

	FILE* file = NULL;
	fopen_s(&file, path, "r");
	if (file == NULL) {
		printf("Couldn't open file");
		return false;
	}

	while (1) {
		char lineHeader[256];

		int res = fscanf_s(file, "%s", lineHeader, sizeof(lineHeader));
		if (res == EOF)
			break;

		if (strcmp(lineHeader, "v") == 0) {
			glm::vec3 vertex;
			fscanf_s(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			temp_positions.push_back(vertex);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			glm::vec2 uv;
			fscanf_s(file, "%f %f\n", &uv.x, &uv.y);
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			glm::vec3 normal;
			fscanf_s(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			std::string pos1, pos2, pos3;
			unsigned int positionIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf_s(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n",
				&positionIndex[0], &uvIndex[0], &normalIndex[0],
				&positionIndex[1], &uvIndex[1], &normalIndex[1],
				&positionIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9) {
				printf("File cannot be read by my crappy parser.");
				return false;
			}
			positionIndices.push_back(positionIndex[0]);
			positionIndices.push_back(positionIndex[1]);
			positionIndices.push_back(positionIndex[2]);
			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}
	}

	for (unsigned int i = 0; i < positionIndices.size(); i++) {
		unsigned int vertexIndex = positionIndices[i];
		glm::vec3 vertex = temp_positions[vertexIndex - 1];
		out_positions.push_back(vertex);
	}
	for (unsigned int i = 0; i < uvIndices.size(); i++) {
		unsigned int uvIndex = uvIndices[i];
		glm::vec2 uv = temp_uvs[uvIndex - 1];
		out_uvs.push_back(uv);
	}
	for (unsigned int i = 0; i < normalIndices.size(); i++) {
		unsigned int normalIndex = normalIndices[i];
		glm::vec3 normal = temp_normals[normalIndex - 1];
		out_normals.push_back(normal);
	}

	for (unsigned int i = 0; i < out_positions.size(); i++) {
		out_vertices.push_back(out_positions[i].x);
		out_vertices.push_back(out_positions[i].y);
		out_vertices.push_back(out_positions[i].z);
		out_vertices.push_back(out_uvs[i].x);
		out_vertices.push_back(out_uvs[i].y);
		out_vertices.push_back(out_normals[i].x);
		out_vertices.push_back(out_normals[i].y);
		out_vertices.push_back(out_normals[i].z);
	}
	return true;
}