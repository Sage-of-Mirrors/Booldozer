#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct GLFWwindow;

constexpr glm::vec3 ZERO = glm::vec3(0.0f, 0.0f, 0.0f);
constexpr glm::vec3 UNIT_X = glm::vec3(1.0f, 0.0f, 0.0f);
constexpr glm::vec3 UNIT_Y = glm::vec3(0.0f, 1.0f, 0.0f);
constexpr glm::vec3 UNIT_Z = glm::vec3(0.0f, 0.0f, 1.0f);

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

/*=== Movement ===*/
	bool mAllowUpdates;
	bool mClickedThisFrame;
	float mMoveSpeed;
	float mMouseSensitivity;

	float mPrevMouseX;
	float mPrevMouseY;

	void Rotate(float dt, float x_delta, float y_delta);

public:
	float NearPlane;
	float FarPlane;
	float Fovy;
	float AspectRatio;

	LSceneCamera();

	// Handles user input for the given window.
	void Update(GLFWwindow* window, float dt);

	// Calculates and returns the view matrix for this camera's position and look-at direction.
	glm::mat4 GetViewMatrix() { return glm::lookAt(mEye, mCenter, mUp); }
	// Calculates and returns the projection matrix from this camera's settings.
	glm::mat4 GetProjectionMatrix() { return glm::perspective<float>(Fovy, AspectRatio, NearPlane, FarPlane); }
};