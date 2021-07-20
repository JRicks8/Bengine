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
#include <gtx/transform.hpp>
#include <gtx/quaternion.hpp>
#include <gtc/type_ptr.hpp>

#include "Mesh.hpp"
#include "Player.hpp"

#include "IndexVBO.hpp"
#include "OBJLoader.hpp"

#define ASSERT(x) if (!(x)) __debugbreak();
#define GLCALL(x) GLClearError();\
    x;\
    ASSERT(GLLogCall(#x, __FILE__, __LINE__))