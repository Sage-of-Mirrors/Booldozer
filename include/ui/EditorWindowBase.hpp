#pragma once

#include <string>

class LEditorWindowBase
{
protected:
	std::string mTitle;

public:
	LEditorWindowBase(std::string title) { mTitle = title; }

	virtual void update(float dt) = 0;
	virtual void render(float dt) = 0;

	virtual ~LEditorWindowBase() = default;
};
