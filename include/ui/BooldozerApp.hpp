#pragma once

#include "scene/EditorScene.hpp"
#include "ui/BooldozerEditor.hpp"
#include "Options.hpp"

// Main window's UI.
class LBooldozerApp
{
	struct GLFWwindow* mWindow;
	int mWidth, mHeight;

	LBooldozerEditor mEditorContext;
	LEditorScene mEditorScene;

	//DiscordEventHandlers mDiscordHandlers = {0};

	LOptionsMenu mOptionsMenu;

	bool Execute(float deltaTime);
	void Render(float deltaTime);
	void RenderUI(float deltaTime);

public:
	LBooldozerApp();
	virtual ~LBooldozerApp() {}

	bool Setup();
	void Run();
	bool Teardown();
};
