#include <vector>

#include "glm.hpp"
#include "headers/BoxShape.hpp"
#include "headers/Rigidbody.hpp"
#include "headers/Mesh.hpp"

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

		rb.orientation = glm::mat3(0.0f);
		rb.linearMomentum = { 0.0f, 0.0f, 0.0f };
		rb.angularMomentum = { 0.0f, 0.0f, 0.0f };

		rb.linearVelocity = rb.linearMomentum / rb.shape.mass; // v(t) = P(t) / M
		rb.ITensor = rb.orientation * rb.shape.iBodyInv * glm::transpose(rb.orientation); // ITensor = R * iBodyInv * transpose(R);
		rb.angularVelocity = rb.ITensor * rb.angularMomentum; // omega = ITensor * L

		rb.force = { 0.0f, 0.0f, 0.0f };
		rb.torque = { 0.0f, 0.0f, 0.0f };
	}
}

void ComputeForceAndTorque(Rigidbody &rigidbody) {
	glm::vec3 f = { 0.0f, 0.5f, 0.0f };
	rigidbody.force += f;
	// r is the vector that goes from the COM to the point of force
	glm::vec3 r = { 0.0f, 0.0f, 0.0f };

	glm::vec3 t = glm::cross(r - rigidbody.position, f);
	rigidbody.torque += t;
}

void StepRigidbodySimulation(std::vector<Mesh> &meshes, float dt) {

	for (int i = 0; i < meshes.size(); i++) {
		Rigidbody& rb = meshes[i].rigidbody;

		if (rb.shape.mass == 0.0f) continue;

		ComputeForceAndTorque(rb);

		rb.ITensor = rb.orientation * rb.shape.iBodyInv * glm::transpose(rb.orientation);

		rb.linearMomentum = rb.force / rb.shape.mass; // P = F/M
		rb.linearVelocity += rb.linearMomentum * dt; // add momentum * dt to velocity
		rb.position += rb.linearVelocity * dt;

		rb.angularMomentum = rb.torque * rb.ITensor;
		rb.angularVelocity += rb.angularMomentum * dt;
		rb.orientation += rb.angularVelocity * dt;
	}

	//PrintRigidBody(rigidbodies[0]);
}