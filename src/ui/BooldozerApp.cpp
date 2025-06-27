#include "ui/BooldozerApp.hpp"
#include "GenUtil.hpp"
#include "ui/LInput.hpp"
#include "TimeUtil.hpp"
#include "UIUtil.hpp"


#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"
#include <fstream>
#include <iostream>

#include <IconsForkAwesome.h>
#include "stb_image.h"
#include "icon.h"
#include "constants.hpp"

constexpr int GL_VERSION_MAJOR = 4;
constexpr int GL_VERSION_MINOR = 6;
constexpr int GL_PROFILE = GLFW_OPENGL_CORE_PROFILE;
constexpr bool GL_IS_DEBUG_CONTEXT = true;

namespace {
	bool ShowLoading = false;
}

LBooldozerApp::LBooldozerApp() : mWindow(nullptr) {}

void DealWithGLErrors(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	//LGenUtility::Log << "GL CALLBACK: " << message << std::endl;
}

bool LBooldozerApp::Setup() {
    InitResourcePaths();

    LGenUtility::Log = std::fstream((USER_DATA_PATH / "booldozer.log").string(), std::ios::out);

	// Init GLFW
	if (!glfwInit()) {
		LGenUtility::Log << "[Booldozer]: Failed to init GLFW!" << std::endl;
		return false;
	}

	// Set some GLFW info
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GL_VERSION_MAJOR);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GL_VERSION_MINOR);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GL_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_IS_DEBUG_CONTEXT);

	// Create GLFW window
	mWindow = glfwCreateWindow(1280, 720, "Booldozer", nullptr, nullptr);
	if (mWindow == nullptr) {
		const char* err;
		glfwGetError(&err);

		LGenUtility::Log << "[Booldozer]: Failed to create GLFW window!" << std::endl;
		LGenUtility::Log << "[Booldozer]: " << err << std::endl;
		glfwTerminate();
		return false;
	}

	GLFWimage images[1];
	images[0].pixels = stbi_load_from_memory(icon_png, icon_png_size, &images[0].width, &images[0].height, nullptr, 4);
	glfwSetWindowIcon(mWindow, 1, images);
	stbi_image_free(images[0].pixels);

	// Set up input callbacks
	glfwSetKeyCallback(mWindow, LInput::GLFWKeyCallback);
	glfwSetCursorPosCallback(mWindow, LInput::GLFWMousePositionCallback);
	glfwSetMouseButtonCallback(mWindow, LInput::GLFWMouseButtonCallback);
	glfwSetScrollCallback(mWindow, LInput::GLFWMouseScrollCallback);

	// Set up GLAD
	glfwMakeContextCurrent(mWindow);
	gladLoadGL();

	glfwSwapInterval(1);

	// Set up GL debug error handling.
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(DealWithGLErrors, nullptr);

	// Init imgui
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(mWindow, true);
	ImGui_ImplOpenGL3_Init("#version 150");


	if(std::filesystem::exists((RES_BASE_PATH / "font" / "NotoSansJP-Regular.otf"))){
		io.Fonts->AddFontFromFileTTF((RES_BASE_PATH / "font" / "NotoSansJP-Regular.otf").string().c_str(), 16.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
	}

	if(std::filesystem::exists((RES_BASE_PATH / "font" / "forkawesome.ttf"))){
		static const ImWchar icons_ranges[] = { ICON_MIN_FK, ICON_MAX_16_FK, 0 };
		ImFontConfig icons_config;
		icons_config.MergeMode = true;
		icons_config.PixelSnapH = true;
		icons_config.GlyphMinAdvanceX = 14.0f;
		io.Fonts->AddFontFromFileTTF((RES_BASE_PATH / "font" / "forkawesome.ttf").string().c_str(), icons_config.GlyphMinAdvanceX, &icons_config, icons_ranges );
	}

	mEditorScene.Init();

	return true;
}

bool LBooldozerApp::Teardown() {

	/*
	Discord_ClearPresence();
	Discord_Shutdown();
	*/

	GCResourceManager.Cleanup();

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

	mEditorScene.Update(mWindow, deltaTime, mEditorContext.GetSelectionManager());

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
	glClearColor(0.100f, 0.261f, 0.402f, 1.0f);
	glClear(GL_DEPTH_BUFFER_BIT);

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
    mOptionsMenu.RenderOptionsPopup(&mEditorScene);
}

void LBooldozerApp::RenderUI(float deltaTime) {
    // Mode combo

    bool openOptionsMenu = false;

    // Menu bar
    if (ImGui::BeginMainMenuBar())
    {
		if (ImGui::MenuItem("Projects")){
			mEditorContext.mOpenProjectManager = true;
		}
        if (ImGui::BeginMenu("Map"))
        {
            if (ImGui::MenuItem(ICON_FK_MAP "  Open"))
                mEditorContext.onOpenMapCB();
            if (ImGui::MenuItem(ICON_FK_PLUS "  Append"))
                mEditorContext.onAppendMapCB();

            ImGui::Separator();
			if (ImGui::MenuItem(ICON_FK_ERASER "  Clear"))
                mEditorContext.onClearMapCB();

			ImGui::Separator();

			if (ImGui::MenuItem(ICON_FK_FLOPPY_O "  Save"))
                mEditorContext.onSaveMapArchiveCB();

			if (ImGui::MenuItem(ICON_FK_SHARE "  Export GCM"))
                mEditorContext.onGCMExportCB();

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem(ICON_FK_COG "  Options"))
                openOptionsMenu = true;


            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Tools"))
        {
			if	(ImGui::MenuItem(ICON_FK_C "  Ghost Config Editor")){
				mEditorContext.mOpenActorEditor = true;
			}
			if	(ImGui::MenuItem(ICON_FK_DATABASE "  Banner Editor")){
				mEditorContext.mOpenBannerEditor = true;
			}
			if	(ImGui::MenuItem(ICON_FK_WINDOW_MAXIMIZE "  Menu Editor")){
				mEditorContext.mOpenMenuEditor = true;
			}
            if (ImGui::MenuItem(ICON_FK_PLAY "  Playtest")){
                mEditorContext.onPlaytestCB();
			}

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help"))
        {
            if(ImGui::MenuItem(ICON_FK_GAMEPAD "  Controls")){
				mEditorContext.mOpenControlsDialog = true;
			}
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

	if(openOptionsMenu){
		mOptionsMenu.OpenMenu();
	}

    ImGuizmo::BeginFrame();
}
