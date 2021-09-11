#include "scene/Camera.hpp"
#include <imgui.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/quaternion.hpp>
#include <bx/bx.h>
#include <bx/math.h>

LSceneCamera::LSceneCamera() :
	NearPlane(0.1f), FarPlane(10000.0f), Fovy(glm::radians(60.0f)), mCenter(ZERO), mEye(ZERO),
	mPitch(0.0f), mYaw(0.0f), mUp(UNIT_Y), mRight(UNIT_X), mForward(UNIT_Z),
	AspectRatio(16.0f / 9.0f), mAllowUpdates(true), mMoveSpeed(1000.0f), mMouseSensitivity(1.0f),
	mPrevMouseX(0.0f), mPrevMouseY(0.0f), mClickedThisFrame(true)
{
	mCenter = mEye + mForward;
}

void LSceneCamera::Update(GLFWwindow* window, float dt)
{
	mLeftClickedThisFrame = false;
	if (!mAllowUpdates || ImGui::GetIO().WantCaptureKeyboard || ImGui::GetIO().WantCaptureMouse)
		return;

	glm::vec3 moveDir = glm::vec3(0.0f, 0.0f, 0.0f);

	// Decide which direction to move
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		moveDir += mForward;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		moveDir -= mForward;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		moveDir -= mRight;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		moveDir += mRight;

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

		Rotate(dt, mPrevMouseX - x, mPrevMouseY - y);

		mPrevMouseX = (float)x;
		mPrevMouseY = (float)y;
	} else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		mLeftClickedThisFrame = true;
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

std::pair<glm::vec3, glm::vec3> LSceneCamera::Raycast(double mouseX, double mouseY, glm::vec4 viewport){
	auto near = glm::unProject(glm::vec3(mouseX, viewport.w - mouseY, 0.0f), GetViewMatrix(), GetProjectionMatrix(), viewport);
	auto far = glm::unProject(glm::vec3(mouseX, viewport.w - mouseY, 1.0f), GetViewMatrix(), GetProjectionMatrix(), viewport);

	auto dir = far - near;
	dir = glm::normalize(dir);

	return std::pair(near, dir);
}