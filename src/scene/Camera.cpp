#include "scene/Camera.hpp"
#include "ui/LInput.hpp"
#include <imgui.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/quaternion.hpp>
#include <algorithm>

LSceneCamera::LSceneCamera() :
	NearPlane(0.1f), FarPlane(100000.0f), Fovy(glm::radians(60.0f)), mCenter(ZERO), mEye(ZERO),
	mPitch(0.0f), mYaw(glm::half_pi<float>()), mUp(UNIT_Y), mRight(UNIT_X), mForward(UNIT_Z),
	AspectRatio(16.0f / 9.0f), mAllowUpdates(true), mMoveSpeed(1000.0f), mMouseSensitivity(1.0f),
	mCamMode(ECamMode::FLY)
{
	mCenter = mEye - mForward;
}

void LSceneCamera::Update(GLFWwindow* window, float dt)
{
	if (!mAllowUpdates || ImGui::GetIO().WantCaptureKeyboard || ImGui::GetIO().WantCaptureMouse)
		return;

	glm::vec3 moveDir = glm::vec3(0.0f, 0.0f, 0.0f);

	// Decide which direction to move
	if(mCamMode == FLY){
		if (LInput::GetKey(GLFW_KEY_W))
			moveDir -= mForward;
		if (LInput::GetKey(GLFW_KEY_S))
			moveDir += mForward;
		if (LInput::GetKey(GLFW_KEY_D))
			moveDir -= mRight;
		if (LInput::GetKey(GLFW_KEY_A))
			moveDir += mRight;
	}
	else if(mCamMode == ORBIT) {
		if (LInput::GetKey(GLFW_KEY_W))
			moveDir += glm::normalize(mCenter - mEye);
		if (LInput::GetKey(GLFW_KEY_S))
			moveDir -= glm::normalize(mCenter - mEye);
		if (LInput::GetKey(GLFW_KEY_D))
			moveDir -= glm::normalize(glm::cross(mCenter - mEye, UNIT_Y));
		if (LInput::GetKey(GLFW_KEY_A))
			moveDir += glm::normalize(glm::cross(mCenter - mEye, UNIT_Y));
		if (LInput::GetKey(GLFW_KEY_E))
			moveDir += mUp;
		if (LInput::GetKey(GLFW_KEY_Q))
			moveDir -= mUp;
	}

	mMoveSpeed += LInput::GetMouseScrollDelta() * 100 * dt;
	mMoveSpeed = std::clamp(mMoveSpeed, 100.f, 50000.f);
	float actualMoveSpeed = LInput::GetKey(GLFW_KEY_LEFT_SHIFT) ? mMoveSpeed * 3.f : mMoveSpeed;

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
	{
		glm::vec2 mouseDelta = LInput::GetMouseDelta();

		if(mCamMode != ORBIT){
			Rotate(dt, mouseDelta);
		}
		else {
			if(mouseDelta.x < 0){
				moveDir -= glm::normalize(glm::cross(mCenter - mEye, UNIT_Y));
			} else if(mouseDelta.x > 0) {
				moveDir += glm::normalize(glm::cross(mCenter - mEye, UNIT_Y));
			}

			if(mouseDelta.y < 0){
				moveDir -= mUp;
			} else if(mouseDelta.y > 0) {
				moveDir += mUp;
			}

		}
	}

	if (glm::length(moveDir) != 0.0f)
		moveDir = glm::normalize(moveDir);

	mEye += moveDir * actualMoveSpeed * dt;
	if(mCamMode != ECamMode::ORBIT)
		mCenter = mEye - mForward;
}

void LSceneCamera::Rotate(float deltaTime, glm::vec2 mouseDelta)
{
	if (mouseDelta.x == 0.f && mouseDelta.y == 0.f)
		return;

	mPitch += mouseDelta.y * deltaTime * mMouseSensitivity;
	mYaw += mouseDelta.x * deltaTime * mMouseSensitivity;

	mPitch = std::clamp(mPitch, LOOK_UP_MIN, LOOK_UP_MAX);

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