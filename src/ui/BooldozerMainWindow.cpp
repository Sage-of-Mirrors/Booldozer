#include "ui/BooldozerMainWindow.hpp"
#include "ImGuizmo.h"
#include "UIUtil.hpp"

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

void LBooldozerMainWindow::initialize(int _argc, char** _argv)
{
    mEditorScene.init();

    //mEditorScene.InstanceModel("literallly any string, it will load cubes for models it cant load or dont exist", glm::identity<glm::mat4>());
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

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            ImGui::Text("Edit Stuff");
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
