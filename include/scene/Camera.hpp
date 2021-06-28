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

	float mNearPlane;
	float mFarPlane;
	float mFovy;
	float mAspectRatio;

/*=== Movement ===*/
	bool mAllowUpdates;
	bool mClickedThisFrame;
	float mMoveSpeed;
	float mMouseSensitivity;

	float mPrevMouseX;
	float mPrevMouseY;

	void Rotate(float dt, float x_delta, float y_delta);

public:
	LSceneCamera();

	// Handles user input for the given window.
	void Update(GLFWwindow* window, float dt);

	// Calculates and returns the view matrix for this camera's position and look-at direction.
	glm::mat4 GetViewMatrix() { return glm::lookAt(mEye, mCenter, mUp); }
	// Calculates and returns the projection matrix from this camera's settings.
	glm::mat4 GetProjectionMatrix() { return glm::perspective<float>(mFovy, mAspectRatio, mNearPlane, mFarPlane); }

	// Returns this camera's near plane value.
	float GetNearPlane() { return mNearPlane; }
	// Returns this camera's far plane value.
	float GetFarPlane() { return mFarPlane; }
	// Returns this camera's field of view.
	float GetFovy() { return mFovy; }
	// Returns this camera's aspect ratio.
	float GetAspectRatio() { return mAspectRatio; }

	// Sets this camera's near plane to the given value.
	void SetNearPlane(float near) { mNearPlane = near; }
	// Sets this camera's far plane to the given value.
	void SetFarPlane(float far) { mFarPlane = far; }
	// Sets this camera's field of view to the given value.
	void setFovy(float fovy) { mFovy = fovy; }
	// Sets this camera's aspect ratio to the ratio of the given width and height.
	void SetAspectRatio(float width, float height) { mAspectRatio = width / height; }
};