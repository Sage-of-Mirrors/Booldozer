#include "scene/Camera.hpp"
#include <imgui.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/quaternion.hpp>

LSceneCamera::LSceneCamera() :
	mNearPlane(0.1f), mFarPlane(10000.0f), mFovy(glm::radians(60.0f)), mCenter(ZERO), mEye(ZERO),
	mPitch(0.0f), mYaw(0.0f), mUp(UNIT_Y), mRight(UNIT_X), mForward(UNIT_Z),
	mAspectRatio(16.0f / 9.0f), mAllowUpdates(true), mMoveSpeed(1000.0f), mMouseSensitivity(15.0f),
	mPrevMouseX(0.0f), mPrevMouseY(0.0f), mClickedThisFrame(true)
{
	mCenter = mEye + mForward;
}

void LSceneCamera::Update(GLFWwindow* window, float dt)
{
	if (!mAllowUpdates || ImGui::GetIO().WantCaptureKeyboard || ImGui::GetIO().WantCaptureMouse)
		return;

	glm::vec3 moveDir = glm::vec3(0.0f, 0.0f, 0.0f);

	// Decide which direction to move
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		moveDir += mForward;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		moveDir -= mForward;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		moveDir += mRight;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		moveDir -= mRight;

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
	{
		double x, y;
		glfwGetCursorPos(window, &x, &y);

		if (mClickedThisFrame)
		{
			mPrevMouseX = (float)x;
			mPrevMouseY = (float)y;

			mClickedThisFrame = false;
		}

		Rotate(dt, x - mPrevMouseX, mPrevMouseY - y);

		mPrevMouseX = (float)x;
		mPrevMouseY = (float)y;
	}
	else
	{
		mClickedThisFrame = true;
	}

	if (glm::length(moveDir) != 0.0f)
		moveDir = glm::normalize(moveDir);

	mEye += moveDir * (mMoveSpeed * dt);
	mCenter = mEye + mForward;
}

void LSceneCamera::Rotate(float dt, float x, float y)
{
	// Delta is 0, so return.
	if (x == 0.0f && y == 0.0f)
		return;

	mPitch += y * dt * mMouseSensitivity;
	mYaw += x * dt * mMouseSensitivity;

	mForward.x = cos(mYaw) * cos(mPitch);
	mForward.y = sin(mPitch);
	mForward.z = sin(mYaw) * cos(mPitch);

	mForward = glm::normalize(mForward);

	mRight = glm::normalize(glm::cross(mForward, UNIT_Y));
	mUp = glm::normalize(glm::cross(mRight, mForward));
}
