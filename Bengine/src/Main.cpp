#include "headers/Main.hpp"

constexpr auto VERTEX_SIZE = 8;

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

static void CreateObject(btVector3 origin, btScalar mass,
	btCollisionShape* shape, btAlignedObjectArray<btCollisionShape*>& collisionShapes,
	btDiscreteDynamicsWorld* world) {
	
	collisionShapes.push_back(shape);

	btTransform transform;
	transform.setIdentity();
	transform.setOrigin(origin);

	//rigidbody is dynamic if and only if mass is non zero, otherwise static
	bool isDynamic = (mass != 0.f);

	btVector3 localInertia(0, 0, 0);
	if (isDynamic)
		shape->calculateLocalInertia(mass, localInertia);

	//using motionstate is optional, it provides interpolation capabilities, and only synchronizes 'active' objects
	btDefaultMotionState* myMotionState = new btDefaultMotionState(transform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, shape, localInertia);
	btRigidBody* body = new btRigidBody(rbInfo);

	//add the body to the dynamics world
	world->addRigidBody(body);
}

static Mesh CreateMesh(const char* name, int texID, int meshIndex, std::vector<Mesh>& meshes) {
	Mesh newMesh;
	newMesh.name = name;
	newMesh.textureID = texID;
	newMesh.meshIndex = meshIndex;
	newMesh.bufferIndex = meshes.size();
	return newMesh;
}

static btTriangleMesh* GenerateTriangleCollisionMesh(std::vector<std::vector<unsigned short>> EBOs, std::vector<std::vector<float>> VBOs, Mesh source) {
	// wtf? this was a massive headache
	btTriangleMesh* triMesh = new btTriangleMesh();
	for (int i = 0; i < EBOs[source.bufferIndex].size();) {
		int bufferIndex = source.bufferIndex;
		btVector3 v1 = btVector3(
			VBOs[bufferIndex][EBOs[bufferIndex][i] * VERTEX_SIZE],
			VBOs[bufferIndex][EBOs[bufferIndex][i] * VERTEX_SIZE + 1],
			VBOs[bufferIndex][EBOs[bufferIndex][i] * VERTEX_SIZE + 2]);
		i++;
		btVector3 v2 = btVector3(
			VBOs[bufferIndex][EBOs[bufferIndex][i] * VERTEX_SIZE],
			VBOs[bufferIndex][EBOs[bufferIndex][i] * VERTEX_SIZE + 1],
			VBOs[bufferIndex][EBOs[bufferIndex][i] * VERTEX_SIZE + 2]);
		i++;
		btVector3 v3 = btVector3(
			VBOs[bufferIndex][EBOs[bufferIndex][i] * VERTEX_SIZE],
			VBOs[bufferIndex][EBOs[bufferIndex][i] * VERTEX_SIZE + 1],
			VBOs[bufferIndex][EBOs[bufferIndex][i] * VERTEX_SIZE + 2]);
		i++;

		triMesh->addTriangle(v1, v2, v3, true);
	}
	return triMesh;
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

#pragma region Declare meshes

	std::vector<Mesh> meshes;
	const char* meshFilePaths[]{
		"models/smooth_suzanne.obj",
		"models/cylinder.obj",
		"models/icosphere.obj",
		"models/player_head.obj",
		"models/player_arm.obj",
		"models/player_hand.obj",
		"models/player_joint.obj",
		"models/plane.obj",
		"models/cube_rod.obj",
		"models/farm_area.obj",
		"models/farm_house.obj",
		"models/farm_house_roof.obj"
	};

	const enum meshTypes {
		SMOOTH_SUZANNE,
		CYLINDER,
		ICOSPHERE,
		PLAYER_HEAD,
		PLAYER_ARM,
		PLAYER_HAND,
		PLAYER_JOINT,
		PLANE,
		CUBE_ROD,
		FARM_AREA,
		FARM_HOUSE,
		FARM_ROOF
	};

#pragma region Create Mesh Objects

	unsigned int counter = 0;

	//smooth suzanne
	meshes.push_back(CreateMesh("suzanne", 0, SMOOTH_SUZANNE, meshes));
	Mesh* suzanne = &meshes[meshes.size() - 1];
	
	//cylinder
	meshes.push_back(CreateMesh("cylinder", 0, CYLINDER, meshes));
	Mesh* cylinder = &meshes[meshes.size() - 1];
	
	//icosphere
	meshes.push_back(CreateMesh("icosphere", 0, ICOSPHERE, meshes));
	Mesh* icosphere = &meshes[meshes.size() - 1];

	//player head
	meshes.push_back(CreateMesh("player_head", 0, PLAYER_HEAD, meshes));
	Mesh* head = &meshes[meshes.size() - 1];

	//player arms
	meshes.push_back(CreateMesh("player_arm0", 0, PLAYER_ARM, meshes));
	Mesh* player_arm0 = &meshes[meshes.size() - 1];
	meshes.push_back(CreateMesh("player_arm1", 0, PLAYER_ARM, meshes));
	Mesh* player_arm1 = &meshes[meshes.size() - 1];
	meshes.push_back(CreateMesh("player_arm2", 0, PLAYER_ARM, meshes));
	Mesh* player_arm2 = &meshes[meshes.size() - 1];
	meshes.push_back(CreateMesh("player_arm3", 0, PLAYER_ARM, meshes));
	Mesh* player_arm3 = &meshes[meshes.size() - 1];

	//player hand
	Mesh hand0 = CreateMesh("player_hand0", 1, PLAYER_HAND, meshes);
	meshes.push_back(hand0);
	Mesh hand1 = CreateMesh("player_hand1", 1, PLAYER_HAND, meshes);
	meshes.push_back(hand1);

	//player joint
	for (int i = 0; i < 4; i++) {
		Mesh joint = CreateMesh("player_joint" + i, 0, PLAYER_JOINT, meshes);
		meshes.push_back(joint);
	}

		//Static members

	//plane
	Mesh plane = CreateMesh("plane", 0, PLANE, meshes);
	meshes.push_back(plane);

	//farm area
	Mesh farm_area = CreateMesh("farm_area", 0, FARM_AREA, meshes);
	meshes.push_back(farm_area);

	// farm house
	Mesh farm_house = CreateMesh("farm_house", 0, FARM_HOUSE, meshes);
	meshes.push_back(farm_house);

	// farm house roof
	Mesh farm_house_roof = CreateMesh("farm_house_roof", 0, FARM_ROOF, meshes);
	meshes.push_back(farm_house_roof);

	//create cube rod stairs
	for (int i = 0; i < 10; i++) {
		Mesh cube_rod = CreateMesh("cube rod", 1, CUBE_ROD, meshes);
		meshes.push_back(cube_rod);
	}

#pragma region Load model files and create buffer objects

	std::vector<GLfloat> raw_vertex_data;
	std::vector<std::vector<GLfloat>> VBOs;
	std::vector<std::vector<unsigned short>> EBOs;

	for (int i = 0; i < meshes.size(); i++) {
		//load models into raw vertex data vector
		VBOs.push_back(std::vector<GLfloat>());
		EBOs.push_back(std::vector<unsigned short>());
		loadOBJ(meshFilePaths[meshes[i].meshIndex], raw_vertex_data);
		indexVBO(raw_vertex_data, EBOs[i], VBOs[i], VERTEX_SIZE);
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
	GLCALL(glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, VERTEX_SIZE * sizeof(GLfloat), (void*)0));
	GLCALL(glEnableVertexAttribArray(posAttrib));

	GLint uvAttrib = glGetAttribLocation(shaderProgram, "uv");
	GLCALL(glVertexAttribPointer(uvAttrib, 2, GL_FLOAT, GL_FALSE, VERTEX_SIZE * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat))));
	GLCALL(glEnableVertexAttribArray(uvAttrib));

	GLint normalAttrib = glGetAttribLocation(shaderProgram, "normal");
	GLCALL(glVertexAttribPointer(normalAttrib, 3, GL_FLOAT, GL_FALSE, VERTEX_SIZE * sizeof(GLfloat), (void*)(5 * sizeof(GLfloat))));
	GLCALL(glEnableVertexAttribArray(normalAttrib));

#pragma endregion

#pragma region Collision Bodies

	//collision shapes
	btAlignedObjectArray<btCollisionShape*> collisionShapes;
	btCollisionShape* sphereShape = new btSphereShape(btScalar(1.));
	btCollisionShape* playerJointShape = new btSphereShape(btScalar(0.215));
	btCollisionShape* cylinderShape = new btCylinderShape(btVector3(1, 1, 1));
	btCollisionShape* playerArmShape = new btCylinderShape(btVector3(0.2, 0.6, 0.2));
	btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(10.), btScalar(0.05), btScalar(10.)));
	btCollisionShape* cubeRodShape = new btBoxShape(btVector3(btScalar(1.), btScalar(0.2), btScalar(0.2)));

	btBvhTriangleMeshShape* farm_areaShape = new btBvhTriangleMeshShape(GenerateTriangleCollisionMesh(EBOs, VBOs, farm_area), true);
	btBvhTriangleMeshShape* farm_houseShape = new btBvhTriangleMeshShape(GenerateTriangleCollisionMesh(EBOs, VBOs, farm_house), true);

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
		body->setFriction(btScalar(0));

		dynamicsWorld->addRigidBody(body);
	}

	//smooth suzanne
	CreateObject(btVector3(-3, 3, 0), 1.0f,
		sphereShape, collisionShapes, dynamicsWorld);

	//cylinder
	CreateObject(btVector3(1, 5, 0), 1.0f,
		cylinderShape, collisionShapes, dynamicsWorld);

	//icosphere
	CreateObject(btVector3(-2, 7, 0), 1.0f,
		sphereShape, collisionShapes, dynamicsWorld);

	//player head
	CreateObject(btVector3(-4, 7, 0), 1.0f,
		sphereShape, collisionShapes, dynamicsWorld);

	//player arms
	for (int i = 0; i < 4; i++) {
		CreateObject(btVector3(-6, 7, 0), 1.0f,
			playerArmShape, collisionShapes, dynamicsWorld);
	}

	//player hands
	for (int i = 0; i < 2; i++) {
		CreateObject(btVector3(-8, 7, 0), 1.0f,
			playerArmShape, collisionShapes, dynamicsWorld);
	}

	//player joints
	for (int i = 0; i < 4; i++) {
		CreateObject(btVector3(-6, 7, 2), 1.0f,
			playerJointShape, collisionShapes, dynamicsWorld);
	}

		//Static members

	//plane
	CreateObject(btVector3(0, 0, 0), 0.0f,
		groundShape, collisionShapes, dynamicsWorld);

	// farm area
	CreateObject(btVector3(60, -1, 0), 0.0f,
		farm_areaShape, collisionShapes, dynamicsWorld);

	// farm house
	CreateObject(btVector3(72, 0, -5), 0.0f,
		farm_houseShape, collisionShapes, dynamicsWorld);

	// farm house roof
	CreateObject(btVector3(79, 18.317, 0), 0.0f,
		groundShape, collisionShapes, dynamicsWorld);

	//create cube rod stairs
	for (int i = 0; i < 10; i++) {
		CreateObject(btVector3(-9 + (float)i * 2, 0.5 + (float)i / 2, 9.8), 0.0f,
			cubeRodShape, collisionShapes, dynamicsWorld);
	}

#pragma endregion

#pragma region Load Textures

	const GLuint textureCount = 2;
	GLuint textures[textureCount];
	const char* textureFilePaths[textureCount] {
		"textures/checker.png",
		"textures/white.png"
	};

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
	player.cam_far_clipping_plane = 500.0f;
	player.cam_offset = glm::vec3(0, 1.5f, 0);
	player.fov = 70.0f;

	player.speed = 10.0f / 500.0f;
	player.mouseSpeed = 2.5f / 500.0f;

	player.grounded = false;

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
			btVector3 adjustedUp = Vec3ToBt(glm::normalize(glm::vec3(0, up.y, 0)));
			body->applyCentralForce(player.speed * adjustedUp * 1000);
		}
		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
			//player.position += up * -player.speed;
			btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[0];
			btRigidBody* body = btRigidBody::upcast(obj);
			body->activate();
			btVector3 adjustedUp = Vec3ToBt(glm::normalize(glm::vec3(0, up.y, 0)));
			body->applyCentralForce(-player.speed * adjustedUp * 1000);
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
				body->getMotionState()->getWorldTransform(meshes[i - 1].transform);
			}
			else {
				meshes[i - 1].transform = obj->getWorldTransform();
			}
			//printf("world pos object %d = %f,%f,%f\n", i, float(meshes[i].transform.getOrigin().getX()), float(meshes[i].transform.getOrigin().getY()), float(meshes[i].transform.getOrigin().getZ()));
		}

		{
			btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[0];
			btRigidBody* body = btRigidBody::upcast(obj);
			if (body && body->getMotionState()) {
				body->getMotionState()->getWorldTransform(player.transform);
			}
			else {
				player.transform = obj->getWorldTransform();
			}
			
			btVector3 rayStart = player.transform.getOrigin();
			btVector3 rayEnd = player.transform.getOrigin() + btVector3(0, -2, 0);
			btCollisionWorld::ClosestRayResultCallback rayCallback(rayStart, rayEnd);
			dynamicsWorld->rayTest(rayStart, rayEnd, rayCallback);

			if (rayCallback.hasHit())
				player.grounded = true;
			else
				player.grounded = false;
			//printf("%i\n", player.grounded);
			if (player.grounded) {
				body->setLinearVelocity(btVector3(body->getLinearVelocity().getX() * 0.995,
					body->getLinearVelocity().getY(),
					body->getLinearVelocity().getZ() * 0.995));
			}
			else
				body->setLinearVelocity(btVector3(body->getLinearVelocity().getX() * 0.998,
					body->getLinearVelocity().getY(),
					body->getLinearVelocity().getZ() * 0.998));

			//printf("world pos object = %f,%f,%f\n", float(player.transform.getOrigin().getX()), float(player.transform.getOrigin().getY()), float(player.transform.getOrigin().getZ()));
			//std::cout << "start: " << rayStart.getX() << " " << rayStart.getY() << " " << rayStart.getZ() << std::endl;
			//std::cout << "end: " << rayEnd.getX() << " " << rayEnd.getY() << " " << rayEnd.getZ() << std::endl;
			//std::cout << rayCallback.hasHit() << std::endl;
		}

#pragma endregion

#pragma region MVP matrices

		// Model, view, and projection matrices
		glm::mat4 model = glm::mat4(1.0f);

		glm::mat4 view = glm::lookAt(
			BtToVec3(player.transform.getOrigin()) + player.cam_offset,
			BtToVec3(player.transform.getOrigin()) + front + player.cam_offset,
			up + player.cam_offset
		);

		glm::mat4 proj = glm::perspective(glm::radians(player.fov), (float)windowX / (float)windowY, player.cam_near_clipping_plane, player.cam_far_clipping_plane);

		GLCALL(glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj)));
		GLCALL(glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view)));

#pragma endregion

#pragma region Player

		player.position = BtToVec3(player.transform.getOrigin());

#pragma endregion

		for (int i = 0; i < meshes.size(); i++) {

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