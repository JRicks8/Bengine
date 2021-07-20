#pragma once

#include <vector>
#include <gtx/quaternion.hpp>

#include "Transform.hpp"
#include "SphereCollider.hpp"

class Rigidbody {
public:

	Rigidbody() {}

	Rigidbody(Transform transform) :
	position(transform.GetPosition()),
	orientation(transform.GetOrientation()),
	scale(transform.GetScale()) {}

	Transform GetTransform() { return Transform(position, orientation, scale); }
	void SetTransform(Transform input) { 
		position = input.GetPosition();
		orientation = input.GetOrientation();
		scale = input.GetScale();
	}

	// Getters

	SphereCollider* GetSphereCollider() { return &collider; }

	float* GetMass() { return &mass; }

	glm::mat3* GetIBody() { return &IBody; }
	glm::mat3* GetIBodyInv() { return &IBodyInv; }

	glm::vec3* GetVelocity() { return &velocity; }
	glm::vec3* GetAngularVelocity() { return &angularVelocity; }

	glm::mat3* GetRotationMatrix() { return &rotation_matrix; }
	glm::mat3* GetITensorInv() { return &ITensorInv; }

	glm::vec3* GetForce() { return &force; }
	glm::vec3* GetTorque() { return &torque; }

	std::vector<glm::vec3>* GetPendingForces() { return &pending_forces; }
	std::vector<glm::vec3>* GetPendingForceLocations() { return &pending_force_locations; }

	// Setters

	void SetSphereCollider(const SphereCollider input) { collider = input; }

	void SetMass(const float input) { mass = input; }

	void SetIBody(const glm::mat3 input) { IBody = input; }
	void SetIBodyInv(const glm::mat3 input) { IBodyInv = input; }

	void SetVelocity(const glm::vec3 input) { velocity = input; }
	void SetAngularVelocity(const glm::vec3 input) { angularVelocity = input; }
	
	void SetRotationMatrix(const glm::mat3 input) { rotation_matrix = input; }
	void SetITensorInv(const glm::mat3 input) { ITensorInv = input; }

	glm::vec3 GetPointVelocity(glm::vec3 point_position);
	void Simulate(float dt);

	/// <summary>
	/// Calculates initial constant variables (like inertia)
	/// </summary>
	void InitializeRigidbody();

	void ApplyForceAtLocalPosition(glm::vec3 f, glm::vec3 p);

private:

	void CalculateForceAndTorque();

	SphereCollider collider;

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
};