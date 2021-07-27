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
/// <summary>
/// Converts glm::vec3 to btVector3.
/// </summary>
/// <param name="v"></param>
/// <returns></returns>
static btVector3 Vec3ToBt(const glm::vec3 v) {
	return btVector3(v.x, v.y, v.z);
}
/// <summary>
/// Converts btVector3 to glm::vec3.
/// </summary>
/// <param name="v"></param>
/// <returns></returns>
static glm::vec3 BtToVec3(const btVector3 v) {
	return glm::vec3(v.getX(), v.getY(), v.getZ());
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

	//default setup for memory and collisions
	btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
	btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);
	btBroadphaseInterface* overlappingPairCache = new btDbvtBroadphase();
	btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver();
	btDiscreteDynamicsWorld* dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);

	dynamicsWorld->setGravity(btVector3(0, -10, 0));

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

#pragma region Set Default Object States and Collision objects

	//collision shapes
	btAlignedObjectArray<btCollisionShape*> collisionShapes;

	//Player
	{
		//btCollisionShape* colShape = new btBoxShape(btVector3(1,1,1));
		btCollisionShape* colShape = new btCapsuleShape(btScalar(1.0), btScalar(2.0));
		collisionShapes.push_back(colShape);
	
		/// Create Dynamic Objects
		btTransform startTransform;
		startTransform.setIdentity();
	
		btScalar mass(1.f);
	
		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		bool isDynamic = (mass != 0.f);
	
		btVector3 localInertia(0, 0, 0);
		if (isDynamic)
			colShape->calculateLocalInertia(mass, localInertia);
	
		startTransform.setOrigin(btVector3(0, 3, 0));
	
		//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
		btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);
		btRigidBody* body = new btRigidBody(rbInfo);
		body->setAngularFactor(btScalar(0));
		body->setFriction(btScalar(3.0f));
	
		dynamicsWorld->addRigidBody(body);
	}

	//smooth suzanne
	Mesh suzanne;
	suzanne.name = "suzanne";
	suzanne.textureID = 0;
	meshes.push_back(suzanne);
	{ // dynamic member

		//btCollisionShape* colShape = new btBoxShape(btVector3(1,1,1));
		btCollisionShape* colShape = new btSphereShape(btScalar(1.));
		collisionShapes.push_back(colShape);

		/// Create Dynamic Objects
		btTransform startTransform;
		startTransform.setIdentity();

		btScalar mass(1.f);

		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		bool isDynamic = (mass != 0.f);

		btVector3 localInertia(0, 0, 0);
		if (isDynamic)
			colShape->calculateLocalInertia(mass, localInertia);

		startTransform.setOrigin(btVector3(-3, 3, 0));

		//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
		btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);
		btRigidBody* body = new btRigidBody(rbInfo);
		body->setFriction(btScalar(.1f));

		dynamicsWorld->addRigidBody(body);
	}

	//cylinder
	Mesh cylinder;
	cylinder.name = "cylinder";
	cylinder.textureID = 0;
	meshes.push_back(cylinder);
	{ // dynamic member
		
		//btCollisionShape* colShape = new btBoxShape(btVector3(1,1,1));
		btCollisionShape* colShape = new btSphereShape(btScalar(1.));
		collisionShapes.push_back(colShape);

		/// Create Dynamic Objects
		btTransform startTransform;
		startTransform.setIdentity();

		btScalar mass(1.f);

		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		bool isDynamic = (mass != 0.f);

		btVector3 localInertia(0, 0, 0);
		if (isDynamic)
			colShape->calculateLocalInertia(mass, localInertia);

		startTransform.setOrigin(btVector3(1, 5, 0));

		//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
		btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);
		btRigidBody* body = new btRigidBody(rbInfo);

		dynamicsWorld->addRigidBody(body);
	}

	//icosphere
	Mesh icosphere;
	icosphere.name = "icosphere";
	icosphere.textureID = 0;
	meshes.push_back(icosphere);
	{ // dynamic member

		//btCollisionShape* colShape = new btBoxShape(btVector3(1,1,1));
		btCollisionShape* colShape = new btSphereShape(btScalar(1.));
		collisionShapes.push_back(colShape);

		/// Create Dynamic Objects
		btTransform startTransform;
		startTransform.setIdentity();

		btScalar mass(1.f);

		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		bool isDynamic = (mass != 0.f);

		btVector3 localInertia(-1, 0, 0);
		if (isDynamic)
			colShape->calculateLocalInertia(mass, localInertia);

		startTransform.setOrigin(btVector3(-2, 7, 0));

		//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
		btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);
		btRigidBody* body = new btRigidBody(rbInfo);

		dynamicsWorld->addRigidBody(body);
	}

	//plane
	Mesh plane;
	plane.name = "plane";
	plane.textureID = 0;
	meshes.push_back(plane);
	{ // static member
		btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(10.), btScalar(0.05), btScalar(10.)));

		collisionShapes.push_back(groundShape);

		btTransform groundTransform;
		groundTransform.setIdentity();
		groundTransform.setOrigin(btVector3(0, 0, 0));

		btScalar mass(0.);

		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		bool isDynamic = (mass != 0.f);

		btVector3 localInertia(0, 0, 0);
		if (isDynamic)
			groundShape->calculateLocalInertia(mass, localInertia);

		//using motionstate is optional, it provides interpolation capabilities, and only synchronizes 'active' objects
		btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, groundShape, localInertia);
		btRigidBody* body = new btRigidBody(rbInfo);

		//add the body to the dynamics world
		dynamicsWorld->addRigidBody(body);
	}
	
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
	player.position = { 0, 0, 0 };
	player.cam_angle_horizontal = 3.14f;
	player.cam_angle_vertical = 0.0f;
	player.cam_near_clipping_plane = 0.1f;
	player.cam_far_clipping_plane = 50.0f;
	player.fov = 70.0f;

	player.speed = 10.0f / 500.0f;
	player.mouseSpeed = 2.5f / 500.0f;

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
	float timeScale = 0.0f;
	GLuint nbFrames = 0;

#pragma endregion

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

		float dt = float(t_now - t_last) * timeScale;
		t_last = glfwGetTime();

#pragma endregion

#pragma region camera and window

		double xpos, ypos;
		int windowX, windowY;
		glfwGetWindowSize(window, &windowX, &windowY);
		glfwGetCursorPos(window, &xpos, &ypos);
		glfwSetCursorPos(window, windowX / 2, windowY / 2);

		int focused = glfwGetWindowAttrib(window, GLFW_FOCUSED);
		if (focused) {
			player.cam_angle_horizontal += player.mouseSpeed * float(windowX / 2 - xpos);
			player.cam_angle_vertical += player.mouseSpeed * float(windowY / 2 - ypos);
			if (player.cam_angle_vertical > glm::radians(89.0f)) player.cam_angle_vertical = glm::radians(89.0f);
			if (player.cam_angle_vertical < glm::radians(-89.0f)) player.cam_angle_vertical = glm::radians(-89.0f);
		}

		// View Directions
		glm::vec3 front(
			cos(player.cam_angle_vertical) * sin(player.cam_angle_horizontal),
			sin(player.cam_angle_vertical),
			cos(player.cam_angle_vertical) * cos(player.cam_angle_horizontal)
		);

		glm::vec3 right(
			sin(player.cam_angle_horizontal - 3.1415f / 2.0f),
			0.0f,
			cos(player.cam_angle_horizontal - 3.1415f / 2.0f)
		);

		glm::vec3 up = glm::cross(right, front);

#pragma endregion

#pragma region input

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			//player.position += front * player.speed;
			btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[0]; // the first index of the collision objects array is always the player
			btRigidBody* body = btRigidBody::upcast(obj);
			body->activate();
			btVector3 adjustedFront = Vec3ToBt(glm::normalize(glm::vec3(front.x, 0, front.z)));
			body->applyCentralForce(player.speed * adjustedFront * 1000);
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			//player.position += right * -player.speed;
			btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[0];
			btRigidBody* body = btRigidBody::upcast(obj);
			body->activate();
			btVector3 adjustedRight = Vec3ToBt(glm::normalize(glm::vec3(right.x, 0, right.z)));
			body->applyCentralForce(-player.speed * adjustedRight * 1000);
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			//player.position += front * -player.speed;
			btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[0];
			btRigidBody* body = btRigidBody::upcast(obj);
			body->activate();
			btVector3 adjustedFront = Vec3ToBt(glm::normalize(glm::vec3(front.x, 0, front.z)));
			body->applyCentralForce(-player.speed * adjustedFront * 1000);
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			//player.position += right * player.speed;
			btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[0];
			btRigidBody* body = btRigidBody::upcast(obj);
			body->activate();
			btVector3 adjustedRight = Vec3ToBt(glm::normalize(glm::vec3(right.x, 0, right.z)));
			body->applyCentralForce(player.speed * adjustedRight * 1000);
		}
		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
			//player.position += up * player.speed;
			btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[0];
			btRigidBody* body = btRigidBody::upcast(obj);
			body->activate();
			body->applyCentralForce(player.speed * Vec3ToBt(up) * 1000);
		}
		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
			//player.position += up * -player.speed;
			btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[0];
			btRigidBody* body = btRigidBody::upcast(obj);
			body->activate();
			body->applyCentralForce(-player.speed * Vec3ToBt(up) * 1000);
		}
		if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
			timeScale = 0.0f;
		}
		else
			timeScale = 1.0f;

#pragma endregion

#pragma region light

		LightPosition_WorldSpace = glm::vec3(cos(t_now * 1.5), 1, sin(t_now * 1.5)) * 2.5f;

		GLCALL(glUniform3f(uniLightPos, LightPosition_WorldSpace.x, LightPosition_WorldSpace.y, LightPosition_WorldSpace.z));
		GLCALL(glUniform1f(uniLightPower, LightPower));
		GLCALL(glUniform3f(uniLightColor, LightColor.x, LightColor.y, LightColor.z));

#pragma endregion

#pragma region physics

		dynamicsWorld->stepSimulation(dt);

		for (int i = dynamicsWorld->getNumCollisionObjects() - 1; i >= 1; i--) { // reserve the first spot for the player
			btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[i];
			btRigidBody* body = btRigidBody::upcast(obj);
			if (body && body->getMotionState()) {
				body->getMotionState()->getWorldTransform(meshes[i-1].transform);
			}
			else {
				meshes[i-1].transform = obj->getWorldTransform();
			}
			//printf("world pos object %d = %f,%f,%f\n", i, float(meshes[i].transform.getOrigin().getX()), float(meshes[i].transform.getOrigin().getY()), float(meshes[i].transform.getOrigin().getZ()));
		}

		btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[0];
		btRigidBody* body = btRigidBody::upcast(obj);
		if (body && body->getMotionState()) {
			body->getMotionState()->getWorldTransform(player.transform);
		}
		else {
			player.transform = obj->getWorldTransform();
		}

#pragma endregion

#pragma region MVP matrices

		// Model, view, and projection matrices
		glm::mat4 model = glm::mat4(1.0f);

		glm::mat4 view = glm::lookAt(
			BtToVec3(player.transform.getOrigin()),
			BtToVec3(player.transform.getOrigin()) + front,
			up
		);

		//glm::mat4 view = glm::lookAt(
		//	player.position,
		//	player.position + front,
		//	up
		//);

		glm::mat4 proj = glm::perspective(glm::radians(player.fov), (float)windowX / (float)windowY, player.cam_near_clipping_plane, player.cam_far_clipping_plane);

		GLCALL(glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj)));
		GLCALL(glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view)));

#pragma endregion

		for (int i = 0; i < meshCount; i++) {

			glm::mat4 specificModel = model;

			glm::vec3 p(
				meshes[i].transform.getOrigin().getX(), 
				meshes[i].transform.getOrigin().getY(), 
				meshes[i].transform.getOrigin().getZ());
			glm::quat o(
				meshes[i].transform.getRotation().getW(),
				meshes[i].transform.getRotation().getX(),
				meshes[i].transform.getRotation().getY(),
				meshes[i].transform.getRotation().getZ());	

			glm::mat4 translate = glm::translate(p);
			glm::mat4 rotate = glm::toMat4(o);
			glm::mat4 scale = glm::scale(glm::vec3(1, 1, 1));

			specificModel = translate * rotate;

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

	delete dynamicsWorld;
	delete solver;
	delete overlappingPairCache;
	delete dispatcher;
	delete collisionConfiguration;

	return 0;
}