#pragma once

#include "bigg.hpp"
#include "ui/EditorWindowMain.hpp"
#include "ui/BooldozerEditor.hpp"

// Main window's UI.
class LBooldozerMainWindow : public bigg::Application
{
  void onReset();

  void update(float dt);
  void render(float dt);

  LBooldozerEditor mEditorContext;
  LEditorWindowMain mMainEditorWindow = LEditorWindowMain("Main Menu");
  
public:
	LBooldozerMainWindow();
};
