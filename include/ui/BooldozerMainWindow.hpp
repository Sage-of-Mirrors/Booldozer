#pragma once

#include "bigg.hpp"
#include "ui/EditorWindowMain.hpp"
#include "ui/BooldozerEditor.hpp"
#include "scene/EditorScene.hpp"
#include "Options.hpp"

// Main window's UI.
class LBooldozerMainWindow : public bigg::Application
{
  void onReset();

  void update(float dt);
  void render(float dt);

  LBooldozerEditor mEditorContext;
  LEditorScene mEditorScene;
  LEditorWindowMain mMainEditorWindow = LEditorWindowMain("Main Menu");

  LOptionsMenu mOptionsMenu;
  
public:
	LBooldozerMainWindow();

	virtual void initialize(int _argc, char** _argv) override;
};
