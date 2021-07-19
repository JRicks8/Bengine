#pragma once

#include "Transform.hpp"
#include "Colliders/BoxCollider.hpp"

class Rigidbody {
public:

	Rigidbody(Transform transform) :
	position(transform.GetPosition()),
	orientation(transform.GetOrientation()),
	scale(transform.GetScale()) {}

	Rigidbody() {}

	void AddForce(glm::vec3 input) { force += input; }
	void AddTorque(glm::vec3 input) { torque += input; }
	
	Transform GetTransform() {
		return Transform(position, orientation, scale);
	}

	void Simulate(float dt) {
		if (mass <= 0) return;

		rotation_matrix = glm::toMat3(orientation);
		ITensorInv = rotation_matrix * IBodyInv * glm::transpose(rotation_matrix);

		CalculateForceAndTorque();

		velocity += (force / mass) * dt;
		position += velocity * dt;

		angularVelocity += ITensorInv * torque * dt;
		orientation += glm::quat(0, angularVelocity) * orientation * 0.5f * dt;
		orientation = glm::normalize(orientation);

		//std::cout << (glm::quat(0, angularVelocity) * orientation).w << " " <<
		//	(glm::quat(0, angularVelocity) * orientation).x << " " <<
		//	(glm::quat(0, angularVelocity) * orientation).y << " " <<
		//	(glm::quat(0, angularVelocity) * orientation).z << " " << std::endl;
	}

	/// <summary>
	/// Calculates initial constant variables (like inertia)
	/// </summary>
	void InitializeRigidbody() {
		IBody = collider.CalculateInertia(mass);
		IBodyInv = glm::inverse(IBody);
	}

	void ApplyForceAtLocalPosition(glm::vec3 f, glm::vec3 p) {
		if (mass <= 0) return;
		pending_forces.push_back(f);
		pending_force_locations.push_back(p);
	}

	BoxCollider collider;

	// constants
	float	  mass{ 1.0f };
	glm::mat3 IBody{ glm::mat3(0) };
	glm::mat3 IBodyInv{ glm::mat3(0) };

	// states
	glm::vec3 position{ glm::vec3(0, 0, 0) };
	glm::quat orientation{ glm::quat(1, 0, 0, 0) };

	glm::vec3 scale{ glm::vec3(1, 1, 1) };

	// derived quantities
	glm::vec3 velocity{ glm::vec3(0, 0, 0) };
	glm::vec3 angularVelocity{ glm::vec3(0, 0, 0) };
	glm::mat3 rotation_matrix{ glm::mat3(0) };
	glm::mat3 ITensorInv{ glm::mat3(0) };

	// received quantities
	glm::vec3 force{ glm::vec3(0, 0, 0) };
	glm::vec3 torque{ glm::vec3(0, 0, 0) };
	std::vector<glm::vec3> pending_forces;
	std::vector<glm::vec3> pending_force_locations;

private:
	void CalculateForceAndTorque() {
		force = glm::vec3(0);
		torque = glm::vec3(0);

		glm::vec3 gravity_force = { 0, -1, 0 }; // jerry-rigged gravity
		//gravity force does not need local position, because gravity is gravity dummy

		force += gravity_force;
		
		for (int i = 0; i < pending_forces.size(); i++) {
			force += pending_forces[i];
			torque += glm::cross(pending_forces[i], pending_force_locations[i]);
		}
		pending_forces.clear();
		pending_force_locations.clear();
	}
};