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
	if (!mAllowUpdates)
		return;

	glm::vec3 moveDir = glm::vec3(0.0f, 0.0f, 0.0f);

	// Decide which direction to move
	if(mCamMode == ECamMode::FLY){
		if (ImGui::IsKeyDown(ImGuiKey_W))
			moveDir -= mForward;
		if (ImGui::IsKeyDown(ImGuiKey_S))
			moveDir += mForward;
		if (ImGui::IsKeyDown(ImGuiKey_D))
			moveDir += mRight;
		if (ImGui::IsKeyDown(ImGuiKey_A))
			moveDir -= mRight;
	}
	else if(mCamMode == ECamMode::ORBIT) {
		if (ImGui::IsKeyDown(ImGuiKey_W))
			moveDir += glm::normalize(mCenter - mEye);
		if (ImGui::IsKeyDown(ImGuiKey_S))
			moveDir -= glm::normalize(mCenter - mEye);
		if (ImGui::IsKeyDown(ImGuiKey_D))
			moveDir += glm::normalize(glm::cross(mCenter - mEye, UNIT_Y));
		if (ImGui::IsKeyDown(ImGuiKey_A))
			moveDir -= glm::normalize(glm::cross(mCenter - mEye, UNIT_Y));
		if (ImGui::IsKeyDown(ImGuiKey_Q))
			moveDir += mUp;
		if (ImGui::IsKeyDown(ImGuiKey_E))
			moveDir -= mUp;
	}

	float actualMoveSpeed = ImGui::IsKeyDown(ImGuiKey_LeftShift) ? mMoveSpeed * 3.f : mMoveSpeed;

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
	{
		glm::vec2 mouseDelta = LInput::GetMouseDelta();

		if(mCamMode != ECamMode::ORBIT){
			Rotate(dt, mouseDelta);
		}
		else {
			if(mouseDelta.x < 0){
				moveDir -= glm::normalize(glm::cross(mCenter - mEye, UNIT_Y));
			} else if(mouseDelta.x > 0) {
				moveDir += glm::normalize(glm::cross(mCenter - mEye, UNIT_Y));
			}

			if(mouseDelta.y < 0){
				moveDir += mUp;
			} else if(mouseDelta.y > 0) {
				moveDir -= mUp;
			}

		}
	}

	if (glm::length(moveDir) != 0.0f)
		moveDir = glm::normalize(moveDir);

	mEye += moveDir * actualMoveSpeed * dt;
	if(mCamMode != ECamMode::ORBIT)
		mCenter = mEye - mForward;
}

void LSceneCamera::UnRotate(){
	mOldForward = mForward;
	mOldRight = mRight;
	mOldUp = mUp;
	mForward = glm::normalize(mEye - mCenter);
	mRight = glm::normalize(glm::cross(mForward, UNIT_Y));
	mUp = glm::normalize(glm::cross(mRight, mForward));
}

void LSceneCamera::ReRotate(){
	mForward = mOldForward;
	mRight = mOldRight;
	mUp = mOldUp;
}

void LSceneCamera::Rotate(float deltaTime, glm::vec2 mouseDelta)
{
	if (mouseDelta.x == 0.f && mouseDelta.y == 0.f)
		return;

	mPitch += mouseDelta.y * deltaTime * mMouseSensitivity;
	mYaw += -mouseDelta.x * deltaTime * mMouseSensitivity;

	mPitch = std::clamp(mPitch, LOOK_UP_MIN, LOOK_UP_MAX);

	mForward.x = cos(mYaw) * cos(mPitch);
	mForward.y = sin(mPitch);
	mForward.z = sin(mYaw) * cos(mPitch);

	mForward = glm::normalize(mForward);

	mRight = glm::normalize(glm::cross(mForward, UNIT_Y));
	mUp = glm::normalize(glm::cross(mRight, mForward));
}