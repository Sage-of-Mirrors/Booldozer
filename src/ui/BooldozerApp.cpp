#include "ui/BooldozerApp.hpp"
#include "ui/LInput.hpp"
#include "TimeUtil.hpp"

#include <GLFW/glfw3.h>
#include <glad/gl.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
#include <iostream>

constexpr int GL_VERSION_MAJOR = 4;
constexpr int GL_VERSION_MINOR = 6;
constexpr int GL_PROFILE = GLFW_OPENGL_CORE_PROFILE;
constexpr bool GL_IS_DEBUG_CONTEXT = true;

LBooldozerApp::LBooldozerApp() : mWindow(nullptr) {}

void LBooldozerApp::Render(float deltaTime) {
	mEditorContext.Render(deltaTime, &mEditorScene);
}

void DealWithGLErrors(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	std::cout << "GL CALLBACK: " << message << std::endl;
}

bool LBooldozerApp::Setup() {
	// Init GLFW
	if (!glfwInit()) {
		std::cout << "Failed to init GLFW!" << std::endl;
		return false;
	}

	// Set some GLFW info
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GL_VERSION_MAJOR);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GL_VERSION_MINOR);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GL_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_IS_DEBUG_CONTEXT);

	// Create GLFW window
	mWindow = glfwCreateWindow(640, 480, "Booldozer", nullptr, nullptr);
	if (mWindow == nullptr) {
		glfwTerminate();

		std::cout << "Failed to create GLFW window!" << std::endl;
		return false;
	}

	// Set up input callbacks
	glfwSetKeyCallback(mWindow, LInput::GLFWKeyCallback);
	glfwSetCursorPosCallback(mWindow, LInput::GLFWMousePositionCallback);
	glfwSetMouseButtonCallback(mWindow, LInput::GLFWMouseButtonCallback);
	glfwSetScrollCallback(mWindow, LInput::GLFWMouseScrollCallback);

	// Set up GLAD
	glfwMakeContextCurrent(mWindow);
	gladLoadGL(glfwGetProcAddress);
	glClearColor(0.5f, 1.0f, 0.5f, 1.0f);
	glfwSwapInterval(1);

	// Set up GL debug error handling.
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(DealWithGLErrors, nullptr);

	// Init imgui
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(mWindow, true);
	ImGui_ImplOpenGL3_Init("#version 150");

	return true;
}

bool LBooldozerApp::Teardown() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(mWindow);
	glfwTerminate();

	return true;
}

void LBooldozerApp::Run() {
	Clock::time_point lastFrame, thisFrame;

	while (true) {
		lastFrame = thisFrame;
		thisFrame = LTimeUtility::GetTime();

		if (!Execute(LTimeUtility::GetDeltaTime(lastFrame, thisFrame)))
			break;
	}
}

bool LBooldozerApp::Execute(float deltaTime) {
	// Try to make sure we return an error if anything's fucky
	if (mWindow == nullptr || glfwWindowShouldClose(mWindow))
		return false;

	mEditorScene.update(mWindow, deltaTime, mEditorContext.GetSelectionManager());

	// Begin actual rendering
	glfwMakeContextCurrent(mWindow);
	glfwPollEvents();

	LInput::UpdateInputState();

	// The context renders both the ImGui elements and the background elements.
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// Update buffer size
	int width, height;
	glfwGetFramebufferSize(mWindow, &width, &height);
	glViewport(0, 0, width, height);

	// Clear buffers
	glClearColor(0.353f, 0.294f, 0.647f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Render(deltaTime);

	// Render imgui
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	// Swap buffers
	glfwSwapBuffers(mWindow);

	return true;
}
