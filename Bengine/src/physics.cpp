#include <vector>
#include <glm.hpp>
#include <gtx/quaternion.hpp>

#include "headers/BoxShape.hpp"
#include "headers/Rigidbody.hpp"
#include "headers/Mesh.hpp"

glm::mat3 QuaternionToMatrix(glm::quat q) {
	return glm::mat3( // What the fuck?
		1 - 2 * q.y * q.y - 2 * q.z * q.z, 2 * q.x * q.y - 2 * q.w * q.z, 2 * q.x * q.z + 2 * q.w * q.y,
		2 * q.x * q.y + 2 * q.w * q.z, 1 - 2 * q.x * q.x - 2 * q.z * q.z, 2 * q.y * q.z - 2 * q.w * q.x,
		2 * q.x * q.z - 2 * q.w * q.y, 2 * q.y * q.z + 2 * q.w * q.x, 1 - 2 * q.x * q.x - 2 * q.y * q.y
	);
}

glm::quat EulerToQuaternion(glm::vec3 euler) // yaw (Z), pitch (Y), roll (X)
{
	// Abbreviations for the various angular functions
	double cy = cos(euler.z * 0.5);
	double sy = sin(euler.z * 0.5);
	double cp = cos(euler.y * 0.5);
	double sp = sin(euler.y * 0.5);
	double cr = cos(euler.x * 0.5);
	double sr = sin(euler.x * 0.5);

	glm::quat q;
	q.w = cr * cp * cy + sr * sp * sy;
	q.x = sr * cp * cy - cr * sp * sy;
	q.y = cr * sp * cy + sr * cp * sy;
	q.z = cr * cp * sy - sr * sp * cy;

	return q;
}

void CalculateBoxInertia(BoxShape &boxShape) {
	float m = boxShape.mass;
	float x = boxShape.width;
	float y = boxShape.height;
	float z = boxShape.depth;
	boxShape.iBody = glm::mat3(y * y + z * z, 0.0f, 0.0f,
								0.0f, x * x + z * z, 0.0f,
								0.0f, 0.0f, x * x + y * y) * (m / 12);
	boxShape.iBodyInv = glm::inverse(boxShape.iBody);
}

void PrintRigidBody(Rigidbody rb) {
	printf("body p = (%.2f, %.2f, %.2f)\n", rb.position.x, rb.position.y, rb.position.z);
	printf("body lin momentum = (%.2f, %.2f, %.2f)\n", rb.linearMomentum.x, rb.linearMomentum.y, rb.linearMomentum.z);
}

void InitializeRigidBodies(std::vector<Mesh> &meshes) {
	for (int i = 0; i < meshes.size(); i++) {
		Rigidbody& rb = meshes[i].rigidbody;

		CalculateBoxInertia(rb.shape); // calculate iBody and iBodyInv

		rb.orientation = glm::quat(0, 0, 0, 0);
		rb.linearMomentum = { 0.0f, 0.0f, 0.0f };
		rb.angularMomentum = { 0.0f, 0.0f, 0.0f };

		rb.rotationMatrix = QuaternionToMatrix(rb.orientation);
		rb.linearVelocity = rb.linearMomentum / rb.shape.mass; // v(t) = P(t) / M
		rb.ITensor = rb.rotationMatrix * rb.shape.iBodyInv * glm::transpose(rb.rotationMatrix); // ITensor = R * iBodyInv * transpose(R);
		rb.angularVelocity = rb.ITensor * rb.angularMomentum; // omega = ITensor * L

		rb.force = { 0.0f, 0.0f, 0.0f };
		rb.torque = { 0.0f, 0.0f, 0.0f };
	}
}

void ComputeForceAndTorque(Rigidbody &rigidbody) {
	glm::vec3 f = { 0.0f, 0.0f, 0.05f };
	rigidbody.force += f;
	// r is the vector that goes from the COM to the point of force
	glm::vec3 r = { 0.0f, 0.5f, 0.0f };

	glm::vec3 t = glm::cross(r - rigidbody.position, f);
	rigidbody.torque += t;
}

void StepRigidbodySimulation(std::vector<Mesh> &meshes, float dt) {

	for (int i = 0; i < meshes.size(); i++) {
		Rigidbody& rb = meshes[i].rigidbody;

		if (rb.shape.mass == 0.0f) continue;

		ComputeForceAndTorque(rb);

		rb.ITensor = rb.rotationMatrix * rb.shape.iBodyInv * glm::transpose(rb.rotationMatrix);

		rb.linearMomentum = rb.force / rb.shape.mass; // P = F/M
		rb.linearVelocity += rb.linearMomentum * dt; // add momentum * dt to velocity
		rb.position += rb.linearVelocity * dt;

		rb.angularMomentum = rb.torque * rb.ITensor;
		rb.angularVelocity += rb.angularMomentum * dt;
		rb.orientation *= EulerToQuaternion(rb.angularVelocity) * dt;
	}

	//PrintRigidBody(rigidbodies[0]);
}