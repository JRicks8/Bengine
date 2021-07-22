#pragma once

#include <glm.hpp>
#include <gtc/quaternion.hpp>

class Transform {
public:
	
	Transform(glm::vec3 p, glm::quat o, glm::vec3 s) :
		position(p),
		orientation(o),
		scale(s) {}

	Transform(glm::vec3 p) :
		position(p) {}

	Transform() {}

	inline glm::vec3 GetPosition() const { return position; }
	inline glm::quat GetOrientation() const { return orientation; }
	inline glm::vec3 GetScale() const { return scale; }

	void SetPosition(glm::vec3 input) { position = input; }
	void SetOrientation(glm::quat input) { orientation = input; }
	void SetScale(glm::vec3 input) { scale = input; }

private:
	glm::vec3 position{glm::vec3(0, 0, 0)};
	glm::quat orientation{glm::quat(1, 0, 0, 0)};
	glm::vec3 scale{glm::vec3(1, 1, 1)};
};