#include "ui/BooldozerEditor.hpp"
#include "JmpIO.hpp"
#include "../lib/libgctools/include/compression.h"
#include "../lib/bStream/bstream.h"
#include "DOM/FurnitureDOMNode.hpp"
#include <iostream>
#include <vector>
#include <memory>
#include "imgui.h"

LBooldozerEditor::LBooldozerEditor()
{
	mCurrentMode = &mActorMode;
}

void LBooldozerEditor::Render(float dt)
{
	if (mLoadedMap == nullptr || mLoadedMap->Children.empty() || mCurrentMode == nullptr)
		return;

	mCurrentMode->Render(mLoadedMap);
}

void LBooldozerEditor::onOpenMapCB()
{
	mLoadedMap = std::shared_ptr<LMapDOMNode>(new LMapDOMNode());
	mLoadedMap->LoadMap(std::filesystem::path("D:\\SZS Tools\\Luigi's Mansion\\root\\files\\Map\\map2.szp"));
}

void LBooldozerEditor::onOpenRoomsCB()
{
	std::cout << "User selected open room(s)!" << std::endl;
}
