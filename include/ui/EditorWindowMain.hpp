#pragma once

#include "ui/EditorWindowBase.hpp"

class LEditorWindowMain : public LEditorWindowBase
{
public:
	LEditorWindowMain(std::string title);

	virtual void update(float dt) override;
	virtual void render(float dt) override;
};