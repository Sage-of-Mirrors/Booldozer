#include "ui/BooldozerApp.hpp"
#include "ui/LInput.hpp"
#include "TimeUtil.hpp"
#include "UIUtil.hpp"

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
	glfwGetFramebufferSize(mWindow, &mWidth, &mHeight);
	glViewport(0, 0, mWidth, mHeight);

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

void LBooldozerApp::Render(float deltaTime) {
    RenderUI(deltaTime);

	mEditorContext.Render(deltaTime, &mEditorScene);
    mOptionsMenu.RenderOptionsPopup();
}

void LBooldozerApp::RenderUI(float deltaTime) {
    // Mode combo
    ImGui::SetNextWindowPos(ImVec2(5, 25));
    ImGui::SetNextWindowSize(ImVec2(195, 40));

    ImGui::Begin("mode combo window", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);
    ImGui::SetNextItemWidth(150);

    if (LUIUtility::RenderComboEnum<EEditorMode>("##modecombo", mEditorContext.CurrentMode))
        mEditorContext.ChangeMode();

    LUIUtility::RenderTooltip("Editor mode. Determines what objects are visible and what can be edited.");
    ImGui::End();

    // Render distance slider
    ImGui::SetNextWindowPos(ImVec2(205, 25));
    ImGui::SetNextWindowSize(ImVec2(150, 40));

    ImGui::Begin("render distance window", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);
    ImGui::SetNextItemWidth(105);

    ImGui::SliderFloat("##renderdistance", &mEditorScene.Camera.FarPlane, 1.0f, 20000.0f);

    LUIUtility::RenderTooltip("Render distance. Determines how far away objects must be before they are no longer visible.");
    ImGui::End();

    // Gizmo mode selection 
    ImGui::SetNextWindowPos(ImVec2(360, 25));
    ImGui::SetNextWindowSize(ImVec2(85, 40));

    ImGui::Begin("gizmo mode window", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);
    ImGui::SetNextItemWidth(105);

    if (ImGui::Button("T"))
    {
        mEditorContext.SetGizmo(ImGuizmo::TRANSLATE);
    }
    else if (ImGui::SameLine(); ImGui::Button("R")) {
        mEditorContext.SetGizmo(ImGuizmo::ROTATE);
    }
    else if (ImGui::SameLine(); ImGui::Button("S")) {
        mEditorContext.SetGizmo(ImGuizmo::SCALE);
    }

    ImGui::End();

    // Camera mode selection
    ImGui::SetNextWindowPos(ImVec2(450, 25));
    ImGui::SetNextWindowSize(ImVec2(110, 40));

    ImGui::Begin("cammove mode window", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);
    ImGui::SetNextItemWidth(105);

    //TODO: Disable if no room is loaded
    if (ImGui::Button("Orbit"))
    {
        mEditorScene.Camera.mCamMode = ECamMode::ORBIT;
    }
    else if (ImGui::SameLine(); ImGui::Button("Fly")) {
        mEditorScene.Camera.mCamMode = ECamMode::FLY;
    }

    ImGui::End();

    bool openOptionsMenu = false;

    // Menu bar
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open Map..."))
                mEditorContext.onOpenMapCB();
            if (ImGui::MenuItem("Open Room(s)..."))
                mEditorContext.onOpenRoomsCB();

            ImGui::Separator();

            if (ImGui::MenuItem("Save Map..."))
                mEditorContext.onSaveMapCB();

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Options"))
                openOptionsMenu = true;

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Tools"))
        {
            if (ImGui::MenuItem("Playtest"))
                mEditorContext.onPlaytestCB();

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help"))
        {
            ImGui::Text("Help Stuff");
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    ImGuizmo::BeginFrame();
    ImGuizmo::SetRect(0, 0, (float)mWidth, (float)mHeight);
}
