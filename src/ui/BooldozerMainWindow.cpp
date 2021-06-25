#include "ui/BooldozerMainWindow.hpp"

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
	mMainEditorWindow.update(dt);

	// Now render!
	render(dt);
}

void LBooldozerMainWindow::render(float dt)
{
	bgfx::touch(0);

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

    ImGui::Begin("Scene Hierarchy");
    mEditorContext.RenderSceneHierarchyUI(dt);
    ImGui::End();

    ImGui::Begin("Selected Object Details");
    ImGui::End();
}
