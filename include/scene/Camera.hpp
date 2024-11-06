#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <utility>

struct GLFWwindow;

constexpr glm::vec3 ZERO = glm::vec3(0.0f, 0.0f, 0.0f);
constexpr glm::vec3 UNIT_X = glm::vec3(1.0f, 0.0f, 0.0f);
constexpr glm::vec3 UNIT_Y = glm::vec3(0.0f, 1.0f, 0.0f);
constexpr glm::vec3 UNIT_Z = glm::vec3(0.0f, 0.0f, 1.0f);

constexpr float LOOK_UP_MIN = -glm::half_pi<float>() + glm::epsilon<float>();
constexpr float LOOK_UP_MAX = glm::half_pi<float>() - glm::epsilon<float>();

enum class ECamMode {
	FLY,
	ORBIT,
	ANIMATION
};

class LSceneCamera
{
/*=== Matrix Calculation Members ===*/
	glm::vec3 mEye;
	glm::vec3 mCenter;

	float mPitch;
	float mYaw;
	glm::vec3 mForward;
	glm::vec3 mRight;
	glm::vec3 mUp;

	glm::vec3 mOldForward;
	glm::vec3 mOldRight;
	glm::vec3 mOldUp;

/*=== Movement ===*/
	bool mAllowUpdates;
	float mMoveSpeed;
	float mMouseSensitivity;

	void Rotate(float deltaTime, glm::vec2 mouseDelta);

public:

	void UnRotate();
	void ReRotate();

	float NearPlane;
	float FarPlane;
	float Fovy;
	float AspectRatio;

	LSceneCamera();

	ECamMode mCamMode;

	// Handles user input for the given window.
	void Update(GLFWwindow* window, float dt);

	// Calculates and returns the view matrix for this camera's position and look-at direction.
	glm::mat4 GetViewMatrix() { return glm::lookAtLH(mEye, mCenter, mUp); }
	// Calculates and returns the projection matrix from this camera's settings.
	glm::mat4 GetProjectionMatrix() { return glm::perspectiveLH<float>(Fovy, AspectRatio, NearPlane, FarPlane); }
	glm::mat4 GetProjectionMatrixOrtho() { return glm::orthoLH<float>(0, 1920, 0, 720*Fovy, NearPlane, FarPlane); }

	glm::vec3 GetEye() { return mEye; }
	glm::vec3 GetCenter() { return mCenter; }
	
	glm::vec3 GetUp() { return mUp; }

	void SetCenter(glm::vec3 center) { mCenter = center; }
	void SetEye(glm::vec3 eye) { mEye = eye; }

	std::pair<glm::vec3, glm::vec3> Raycast(double mouseX, double mouseY, glm::vec4 viewport);

};