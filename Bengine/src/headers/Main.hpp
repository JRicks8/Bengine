#pragma once

#include <iostream>
#include <sstream>
#include <fstream>
#include <glm.hpp>

#include <glew.h>
#include <glfw3.h>
#include <SOIL.h>
#include <chrono>
#include <vector>

#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include "Mesh.hpp"
#include "Player.hpp"
#include "BoxShape.hpp"

#define ASSERT(x) if (!(x)) __debugbreak();
#define GLCALL(x) GLClearError();\
    x;\
    ASSERT(GLLogCall(#x, __FILE__, __LINE__))

bool loadOBJ(const char* path, std::vector<float>&out_vertices);

void indexVBO(
	std::vector<float>&in_vbo,
	std::vector<unsigned short>&out_ebo,
	std::vector<float>&out_vbo,
	int vertex_size
);

void StepRigidbodySimulation(std::vector<Mesh> &meshes, float dt);

void InitializeRigidBodies(std::vector<Mesh> &meshes);