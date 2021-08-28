#define _USE_MATH_DEFINES
#include <cmath>

#include "ui/BooldozerMainWindow.hpp"
#include "ImGuizmo.h"
#include "UIUtil.hpp"
#include "GhostImg.h"
#include "stb_image.h"

LBooldozerMainWindow::LBooldozerMainWindow() : bigg::Application("Booldozer")
{
}

void LBooldozerMainWindow::onReset()
{
  bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0xC0C0C0FF, 1.0f, 0);
  bgfx::setViewRect(0, 0, 0, uint16_t(getWidth()), uint16_t( getHeight() ));
}

void LBooldozerMainWindow::update(float dt)
{
	// Update state here
	mEditorScene.update(mWindow, dt);
	mMainEditorWindow.update(dt);

	// Now render!
	render(dt);
}

static bgfx::TextureHandle sGhostImg;

void LBooldozerMainWindow::initialize(int _argc, char** _argv)
{
	mEditorScene.init();

	//mEditorScene.InstanceModel("literallly any string, it will load cubes for models it cant load or dont exist", glm::identity<glm::mat4>());
	int x, y, n;
	uint8_t* data = stbi_load_from_memory(&ghostImgData[0], ghostImgData_size, &x, &y, &n, 4);
	sGhostImg = bgfx::createTexture2D((uint16_t)x, (uint16_t)y, false, 1, bgfx::TextureFormat::RGBA8, 0, bgfx::copy(data, x*y*4));
	stbi_image_free(data);
}


static bool AngleVisualizer(const char* label, float* angle)
{
	ImGuiIO& io = ImGui::GetIO();
	ImGuiStyle& style = ImGui::GetStyle();

	ImVec2 pos = ImGui::GetCursorScreenPos();
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImVec2 center = ImVec2(pos.x + 32, pos.y + 32);

	float angle_cos = cosf((*angle) * (M_PI / 180)), angle_sin = sinf((*angle) * (M_PI / 180));
	float radius_inner = 16.0f;
	draw_list->AddLine(ImVec2(center.x - angle_sin*radius_inner, center.y + angle_cos*radius_inner), ImVec2(center.x - angle_sin*(30), center.y + angle_cos*(30)), ImGui::GetColorU32(ImGuiCol_SliderGrabActive), 2.0f);
	draw_list->AddLine(ImVec2(center.x, center.y + radius_inner), ImVec2(center.x, center.y + 30), ImGui::GetColorU32(ImGuiCol_SliderGrabActive), 2.0f);
	draw_list->AddImage((void*)(intptr_t)sGhostImg.idx, ImVec2(center.x - 8, center.y - 8), ImVec2(center.x + 8, center.y + 8));
	//draw_list->AddCircleFilled(center, 10.0f, ImGui::GetColorU32(ImGuiCol_FrameBgActive), 16);

	return true;
}

void LBooldozerMainWindow::render(float dt)
{
	bgfx::touch(0);

    mEditorScene.RenderSubmit(getWidth(), getHeight());

    // Mode combo
    ImGui::SetNextWindowPos(ImVec2(5, 25));
    ImGui::SetNextWindowSize(ImVec2(195, 35));

    ImGui::Begin("mode combo window", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);
    ImGui::SetNextItemWidth(150);
    
    if (LUIUtility::RenderComboEnum<EEditorMode>("##modecombo", mEditorContext.CurrentMode))
        mEditorContext.ChangeMode();
    
    LUIUtility::RenderTooltip("Editor mode. Determines what objects are visible and what can be edited.");
    ImGui::End();

    // Render distance slider
    ImGui::SetNextWindowPos(ImVec2(205, 25));
    ImGui::SetNextWindowSize(ImVec2(150, 35));

    ImGui::Begin("render distance window", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);
    ImGui::SetNextItemWidth(105);

    ImGui::SliderFloat("##renderdistance", &mEditorScene.Camera.FarPlane, 1.0f, 20000.0f);

    LUIUtility::RenderTooltip("Render distance. Determines how far away objects must be before they are no longer visible.");
    ImGui::End();

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
                mEditorContext.onOpenOptionsCB();

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Tools"))
        {
            ImGui::Text("Tool Stuff");
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
	ImGuizmo::SetRect(0,0, (float)getWidth(), (float)getHeight());
    mEditorContext.Render(dt, &mEditorScene);
}