#include "headers/Rigidbody.hpp"

void Rigidbody::Simulate(float dt) {
	if (mass <= 0) return;

	rotation_matrix = glm::toMat3(orientation);
	ITensorInv = rotation_matrix * IBodyInv * glm::transpose(rotation_matrix);

	CalculateForceAndTorque();

	velocity += (force / mass) * dt;
	position += velocity * dt;

	angularVelocity += ITensorInv * torque * dt;
	orientation += glm::quat(0, angularVelocity) * orientation * 0.5f * dt;
	orientation = glm::normalize(orientation);
}

glm::vec3 Rigidbody::GetPointVelocity(glm::vec3 point_position) {
	return velocity + glm::cross(angularVelocity, point_position - position);
}

void Rigidbody::InitializeRigidbody() {
	IBody = collider.CalculateInertia(mass);
	IBodyInv = glm::inverse(IBody);
}

void Rigidbody::ApplyForceAtLocalPosition(glm::vec3 f, glm::vec3 p) {
	if (mass <= 0) return;
	pending_forces.push_back(f);
	pending_force_locations.push_back(p);
}

void Rigidbody::CalculateForceAndTorque() {
	force = glm::vec3(0);
	torque = glm::vec3(0);

	glm::vec3 gravity_force = { 0, -1, 0 }; // jerry-rigged gravity
	//gravity force does not need local position, because gravity is gravity dummy

	force += gravity_force;

	for (unsigned int i = 0; i < pending_forces.size(); i++) {
		force += pending_forces[i];
		torque += glm::cross(pending_forces[i], pending_force_locations[i]);
	}
	pending_forces.clear();
	pending_force_locations.clear();
}