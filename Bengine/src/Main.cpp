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

#pragma region physics init

	//glm::quat q0 = { 0, 0, 0, 1 };
	//glm::quat q1 = { 0.1, 0.5, 0.3, 0.8 };
	//
	//std::cout << (q0 * q1).w << " " <<
	//	(q0 * q1).x << " " <<
	//	(q0 * q1).y << " " <<
	//	(q0 * q1).z << " " << std::endl;

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
	suzanne.transform = Transform(glm::vec3(3, 2, 2));
	suzanne.rigidbody = Rigidbody(suzanne.transform);
	suzanne.rigidbody.SetSphereCollider(SphereCollider());
	suzanne.rigidbody.InitializeRigidbody();
	suzanne.textureID = 0;
	meshes.push_back(suzanne);

	//cylinder
	Mesh cylinder;
	cylinder.name = "cylinder";
	cylinder.transform = Transform(glm::vec3(-3, 2, 2));
	cylinder.rigidbody = Rigidbody(cylinder.transform);
	cylinder.rigidbody.SetSphereCollider(SphereCollider());
	cylinder.rigidbody.InitializeRigidbody();
	cylinder.textureID = 0;
	meshes.push_back(cylinder);

	//icosphere (light)
	Mesh icosphere;
	icosphere.name = "icosphere";
	icosphere.transform = Transform(glm::vec3(0, 2, 2));
	icosphere.rigidbody = Rigidbody(icosphere.transform);
	cylinder.rigidbody.SetSphereCollider(SphereCollider());
	icosphere.rigidbody.InitializeRigidbody();
	icosphere.textureID = 0;
	meshes.push_back(icosphere);

	//plane
	Mesh plane;
	plane.name = "plane";
	plane.transform = Transform();
	plane.rigidbody = Rigidbody(plane.transform);
	plane.rigidbody.InitializeRigidbody();
	plane.rigidbody.SetMass(0.0f); // interpreted as infinite mass, meaning no movement
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

	GLuint uniTexture = glGetUniformLocation(shaderProgram, "tex");

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

#pragma endregion

#pragma region Setting up player and controls

	Player player;

	player.position = glm::vec3(0.0f, 2.0f, 0.0f);
	player.cam_angle_horizontal = 3.14f;
	player.cam_angle_vertical = 0.0f;
	player.cam_near_clipping_plane = 0.1f;
	player.cam_far_clipping_plane = 50.0f;
	player.fov = 70.0f;

	player.speed = 10.0f;
	player.mouseSpeed = 2.5f;

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

	glm::mat4 matrix = glm::mat4(1);

	while (!glfwWindowShouldClose(window)) {

		GLCALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
		glClearColor(0.29f, 0.32f, 0.57f, 0.0f);

#pragma region fps and dt

		double t_now = glfwGetTime();
		nbFrames++;

		if (t_now - fps_time >= 1.0) {
			printf("%f ms/frame\n", 1000 / double(nbFrames));
			nbFrames = 0;
			fps_time += 1.0;
		}

		float dt = float(t_now - t_last);
		t_last = glfwGetTime();

#pragma endregion

#pragma region camera and window

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

#pragma endregion

#pragma region input

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
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
			for (unsigned int i = 0; i < meshes.size(); i++) {
				meshes[i].rigidbody.ApplyForceAtLocalPosition(glm::vec3(0, 5, 0), glm::vec3(0.1f, 0.0f, 0.1f));
			}
		}
		if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
			std::cout << meshes[0].transform.GetOrientation().w << " " <<
				meshes[0].transform.GetOrientation().x << " " <<
				meshes[0].transform.GetOrientation().y << " " <<
				meshes[0].transform.GetOrientation().z << " " << std::endl;
		}

#pragma endregion

#pragma region light

		LightPosition_WorldSpace = glm::vec3(cos(t_now * 1.5), 1, sin(t_now * 1.5)) * 2.5f;

		GLCALL(glUniform3f(uniLightPos, LightPosition_WorldSpace.x, LightPosition_WorldSpace.y, LightPosition_WorldSpace.z));
		GLCALL(glUniform1f(uniLightPower, LightPower));
		GLCALL(glUniform3f(uniLightColor, LightColor.x, LightColor.y, LightColor.z));

#pragma endregion

#pragma region physics

		

#pragma endregion

#pragma region MVP matrices

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

#pragma endregion

		for (int i = 0; i < meshCount; i++) {

			meshes[i].rigidbody.Simulate(dt);
			meshes[i].transform = meshes[i].rigidbody.GetTransform();

			glm::mat4 specificModel = model;

			glm::mat4 translate = glm::translate(meshes[i].transform.GetPosition());

			glm::mat4 rotate = glm::toMat4(meshes[i].transform.GetOrientation());

			glm::mat4 scale = glm::scale(meshes[i].transform.GetScale());

			specificModel = translate * rotate * scale;

			GLCALL(glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(specificModel)));

			GLCALL(glActiveTexture(GL_TEXTURE0 + meshes[i].textureID));
			GLCALL(glBindTexture(GL_TEXTURE_2D, textures[meshes[i].textureID])); // BIND TEXTURE
			GLCALL(glUniform1i(uniTexture, meshes[i].textureID));

			GLCALL(glBufferData(GL_ARRAY_BUFFER, VBOs[i].size() * sizeof(GLfloat), &VBOs[i][0], GL_STATIC_DRAW));
			GLCALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, EBOs[i].size() * sizeof(unsigned short), &EBOs[i][0], GL_STATIC_DRAW));

			glDrawElements(GL_TRIANGLES, EBOs[i].size(), GL_UNSIGNED_SHORT, (void*)0);
		}

		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	GLCALL(glDeleteProgram(shaderProgram))

	glfwTerminate();
	return 0;
}