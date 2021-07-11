#include "headers/Main.hpp"

static void GLClearError() {

	while (glGetError());
}

static bool GLLogCall(const char* function, const char* file, int line) {

	while (GLenum error = glGetError()) {

		std::cout << "[OPENGL ERROR " << error << "] : " << file
			<< " LINE " << line << " : " << function << std::endl;
		return false;
	}
	return true;
}

static std::string ParseShader(const std::string& filepath) {

	std::ifstream stream(filepath);

	std::string line;
	std::stringstream ss;

	while (getline(stream, line)) {

		ss << line << '\n';
	}
	return ss.str();
}

static unsigned int CompileShader(unsigned int type, const std::string& source) {

	unsigned int id = glCreateShader(type);
	const char* src = source.c_str();
	GLCALL(glShaderSource(id, 1, &src, nullptr));
	GLCALL(glCompileShader(id));

	//Error checker, returns false if contains syntax errors
	int result;
	GLCALL(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
	if (result == GL_FALSE) {

		int length;
		GLCALL(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
		char* message = (char*)_malloca(length * sizeof(char));
		GLCALL(glGetShaderInfoLog(id, length, &length, message));
		std::cout << "Failed to compile shader of type " << type << std::endl;
		std::cout << message << std::endl;

		GLCALL(glDeleteShader(id));
		return 0;
	}
	else {
		
		std::cout << "Shader of type " << type << " compiled successfully." << std::endl;
	}

	return id;
}

static unsigned int CreateShaderProgram(const std::string& vertexShader, const std::string& fragmentShader) {

	unsigned int program = glCreateProgram();
	unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
	unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

	GLCALL(glAttachShader(program, vs));
	GLCALL(glAttachShader(program, fs));
	GLCALL(glLinkProgram(program));
	GLCALL(glValidateProgram(program));

	GLCALL(glDeleteShader(vs));
	GLCALL(glDeleteShader(fs));

	return program;
}

int main(){
	GLFWwindow* window;

	if (!glfwInit())
		return -1;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	window = glfwCreateWindow(1280, 720, "Bengine", nullptr, nullptr);

	if (!window) {
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK)
		return -1;

	std::cout << glGetString(GL_VERSION) << std::endl;

#pragma region Create Shader Program
	const std::string shaderProgramSource[2] = {
		ParseShader("res/shaders/BasicVertex.shader"),
		ParseShader("res/shaders/BasicFragment.shader")
	};

	GLuint shaderProgram = CreateShaderProgram(shaderProgramSource[0], shaderProgramSource[1]);
	GLCALL(glUseProgram(shaderProgram));
#pragma endregion

#pragma region Declare meshes and textures

	const GLuint meshCount = 4;
	std::vector<Mesh> meshes;
	const char* meshFilePaths[]{
		"models/smooth_suzanne.obj",
		"models/cylinder.obj",
		"models/icosphere.obj",
		"models/plane.obj"
	};

#pragma region Set Default Object States

	//smooth suzanne
	Mesh suzanne;
	suzanne.name = "suzanne";
	suzanne.modelPosition_WorldSpace = glm::vec3(3, 2, 2);
	suzanne.textureID = 0;
	meshes.push_back(suzanne);

	//cylinder
	Mesh cylinder;
	cylinder.name = "cylinder";
	cylinder.modelPosition_WorldSpace = glm::vec3(-3, 2, 2);
	cylinder.textureID = 0;
	meshes.push_back(cylinder);

	//icosphere (light)
	Mesh icosphere;
	icosphere.name = "icosphere";
	icosphere.modelPosition_WorldSpace = glm::vec3(0, 2, 2);
	icosphere.textureID = 1;
	meshes.push_back(icosphere);

	//plane
	Mesh plane;
	plane.name = "plane";
	plane.modelPosition_WorldSpace = glm::vec3(0, 0, 0);
	plane.textureID = 0;
	meshes.push_back(plane);

#pragma endregion

	const GLuint textureCount = 2;
	GLuint textures[textureCount];
	const char* textureFilePaths[textureCount]{
		"textures/checker.png",
		"textures/white.png"
	};

#pragma endregion

#pragma region Load model files and create buffer objects

	const GLuint vertex_size = 8;

	std::vector<GLfloat> raw_vertex_data;
	std::vector<GLfloat> VBOs[meshCount];
	std::vector<unsigned short> EBOs[meshCount];

	for (int i = 0; i < meshCount; i++) {
		//load models into raw vertex data vector
		loadOBJ(meshFilePaths[i], raw_vertex_data);
		indexVBO(raw_vertex_data, EBOs[i], VBOs[i], vertex_size);
		raw_vertex_data.clear();
		GenerateAABBfromVBO(VBOs[i], meshes[i].maxAABB, meshes[i].minAABB, vertex_size);
	}

	GLuint vao;
	GLCALL(glGenVertexArrays(1, &vao));
	GLCALL(glBindVertexArray(vao));

	GLuint vbo;
	GLCALL(glGenBuffers(1, &vbo));
	GLCALL(glBindBuffer(GL_ARRAY_BUFFER, vbo));

	GLuint ebo;
	GLCALL(glGenBuffers(1, &ebo));
	GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo));

	GLint posAttrib = glGetAttribLocation(shaderProgram, "vertexposition_local");
	GLCALL(glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, vertex_size * sizeof(GLfloat), (void*)0));
	GLCALL(glEnableVertexAttribArray(posAttrib));

	GLint uvAttrib = glGetAttribLocation(shaderProgram, "uv");
	GLCALL(glVertexAttribPointer(uvAttrib, 2, GL_FLOAT, GL_FALSE, vertex_size * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat))));
	GLCALL(glEnableVertexAttribArray(uvAttrib));

	GLint normalAttrib = glGetAttribLocation(shaderProgram, "normal");
	GLCALL(glVertexAttribPointer(normalAttrib, 3, GL_FLOAT, GL_FALSE, vertex_size * sizeof(GLfloat), (void*)(5 * sizeof(GLfloat))));
	GLCALL(glEnableVertexAttribArray(normalAttrib));

#pragma endregion

#pragma region Load Textures

	unsigned char* images[textureCount];
	int widths[textureCount], heights[textureCount];
	
	GLCALL(glGenTextures(textureCount, textures));
	
	for (int i = 0; i < textureCount; i++) {

		GLCALL(glActiveTexture(GL_TEXTURE0 + i));
		GLCALL(glBindTexture(GL_TEXTURE_2D, textures[i])); // BIND TEXTURE

		images[i] = SOIL_load_image(textureFilePaths[i], &widths[i], &heights[i], 0, SOIL_LOAD_RGB); // LOAD IMAGE
		GLCALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, widths[i], heights[i], 0, GL_RGB,
			GL_UNSIGNED_BYTE, images[i]));
		SOIL_free_image_data(images[i]);

		GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT)); // IMAGE PARAMETERS
		GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
		GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
		GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

		GLCALL(glGenerateMipmap(GL_TEXTURE_2D));
	}
	

#pragma endregion

#pragma region Declare Uniforms

	//GLuint uniTime = glGetUniformLocation(shaderProgram, "time");

	GLuint uniModel = glGetUniformLocation(shaderProgram, "model");
	GLuint uniView = glGetUniformLocation(shaderProgram, "view");
	GLuint uniProj = glGetUniformLocation(shaderProgram, "proj");

	GLuint uniLightPos = glGetUniformLocation(shaderProgram, "lightposition_worldspace");
	GLuint uniLightPower = glGetUniformLocation(shaderProgram, "lightpower");
	GLuint uniLightColor = glGetUniformLocation(shaderProgram, "lightcolor");

	GLuint uniModelPosition = glGetUniformLocation(shaderProgram, "modelposition_worldspace");

	GLuint uniTexture = glGetUniformLocation(shaderProgram, "tex");

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

#pragma endregion

#pragma region Setting up player and controls

	Player player;

	player.position = glm::vec3(0.0f, 0.0f, 0.0f);
	player.cam_angle_horizontal = 3.14f;
	player.cam_angle_vertical = 0.0f;
	player.cam_near_clipping_plane = 0.1f;
	player.cam_far_clipping_plane = 50.0f;
	player.fov = 70.0f;

	player.speed = 20.0f;
	player.mouseSpeed = 10.0f;

	player.Xwidth = 1.0f;
	player.Zdepth = 1.0f;
	player.height = 1.0f;

	float x = player.Xwidth / 2;
	float y = player.height / 2;
	float z = player.Zdepth / 2;

	player.maxAABB = glm::vec3(x, y, z);
	player.minAABB = -player.maxAABB;

#pragma endregion

#pragma region Objects

	glm::vec3 LightPosition_WorldSpace(0, 0, 1);
	glm::vec3 LightColor(1, 1, 1);
	GLfloat LightPower = 2.0f;

#pragma endregion

#pragma region Misc. variables

	double t_last = 0.0;
	double t_now = 0.0;
	double fps_time = glfwGetTime();
	GLuint nbFrames = 0;

#pragma endregion

	while (!glfwWindowShouldClose(window)) {

		GLCALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

		// The meaty operations :o
		double t_now = glfwGetTime();
		nbFrames++;

		if (t_now - fps_time >= 1.0) {
			printf("%f ms/frame\n", 1000 / double(nbFrames));
			nbFrames = 0;
			fps_time += 1.0;
		}

		float dt = float(t_now - t_last);

		double xpos, ypos;
		int windowX, windowY;
		glfwGetWindowSize(window, &windowX, &windowY);
		glfwGetCursorPos(window, &xpos, &ypos);
		glfwSetCursorPos(window, windowX / 2, windowY / 2);

		player.cam_angle_horizontal += player.mouseSpeed * float(windowX / 2 - xpos) * dt;
		player.cam_angle_vertical += player.mouseSpeed * float(windowY / 2 - ypos) * dt;
		if (player.cam_angle_vertical > glm::radians(90.0f)) player.cam_angle_vertical = glm::radians(90.0f);
		if (player.cam_angle_vertical < glm::radians(-90.0f)) player.cam_angle_vertical = glm::radians(-90.0f);

		// View Directions
		glm::vec3 front(
			cos(player.cam_angle_vertical) * sin(player.cam_angle_horizontal),
			sin(player.cam_angle_vertical),
			cos(player.cam_angle_vertical) * cos(player.cam_angle_horizontal)
		);

		glm::vec3 right(
			sin(player.cam_angle_horizontal - 3.14f / 2.0f),
			0.0f,
			cos(player.cam_angle_horizontal - 3.14f / 2.0f)
		);

		glm::vec3 up = glm::cross(right, front);

		// Do movement input

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			player.position += front * player.speed * dt;
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			player.position += right * -player.speed * dt;
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			player.position += front * -player.speed * dt;
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			player.position += right * player.speed * dt;
		}
		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
			player.position += up * player.speed * dt;
		}
		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
			player.position += up * -player.speed * dt;
		}

		// Model, view, and projection matrices
		glm::mat4 model = glm::mat4(1.0f);

		glm::mat4 view = glm::lookAt(
			player.position,
			player.position + front,
			up
		);

		glm::mat4 proj = glm::perspective(glm::radians(player.fov), (float)windowX / (float)windowY, player.cam_near_clipping_plane, player.cam_far_clipping_plane);

		GLCALL(glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj)));
		GLCALL(glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view)));
		GLCALL(glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model)));

		LightPosition_WorldSpace = glm::vec3(cos(t_now * 1.5), 1, sin(t_now * 1.5)) * 2.5f;
		meshes[2].modelPosition_WorldSpace = LightPosition_WorldSpace;

		GLCALL(glUniform3f(uniLightPos, LightPosition_WorldSpace.x, LightPosition_WorldSpace.y, LightPosition_WorldSpace.z));
		GLCALL(glUniform1f(uniLightPower, LightPower));
		GLCALL(glUniform3f(uniLightColor, LightColor.x, LightColor.y, LightColor.z));

		for (int i = 0; i < meshCount; i++) {
			CheckForPlayerCollisions(meshes, player);

			GLCALL(glUniform3f(uniModelPosition, meshes[i].modelPosition_WorldSpace.x,
				meshes[i].modelPosition_WorldSpace.y,
				meshes[i].modelPosition_WorldSpace.z));

			GLCALL(glActiveTexture(GL_TEXTURE0 + meshes[i].textureID));
			GLCALL(glBindTexture(GL_TEXTURE_2D, textures[meshes[i].textureID])); // BIND TEXTURE
			GLCALL(glUniform1i(uniTexture, meshes[i].textureID));

			GLCALL(glBufferData(GL_ARRAY_BUFFER, VBOs[i].size() * sizeof(GLfloat), &VBOs[i][0], GL_STATIC_DRAW));
			GLCALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, EBOs[i].size() * sizeof(unsigned short), &EBOs[i][0], GL_STATIC_DRAW));

			glDrawElements(GL_TRIANGLES, EBOs[i].size(), GL_UNSIGNED_SHORT, (void*)0);
		}

		glfwSwapBuffers(window);

		glfwPollEvents();

		t_last = glfwGetTime();
	}

	GLCALL(glDeleteProgram(shaderProgram))

	glfwTerminate();
	return 0;
}