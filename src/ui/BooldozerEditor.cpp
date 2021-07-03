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

void LBooldozerEditor::Render(float dt, LEditorScene* renderer_scene)
{
	if (mLoadedMap == nullptr || mLoadedMap->Children.empty() || mCurrentMode == nullptr)
		return;

	mCurrentMode->Render(mLoadedMap, renderer_scene);
}

void LBooldozerEditor::onOpenMapCB()
{
	mLoadedMap = std::make_shared<LMapDOMNode>();
	//mLoadedMap->LoadMap(std::filesystem::path("D:\\SZS Tools\\Luigi's Mansion\\root\\files\\Map\\map2.szp")); /* Gamma PC */
	mLoadedMap->LoadMap(std::filesystem::path("/home/mu/GC Games/Luigi's Mansion/files/Map/map2.szp")); /* Gamma Laptop */ 
	//mLoadedMap->LoadMap(std::filesystem::path("/home/spacey/Projects/LuigisMansion/Mods/LMArcade/files/Map/map2.szp")); /* Space */
}

void LBooldozerEditor::onOpenRoomsCB()
{
	std::cout << "User selected open room(s)!" << std::endl;
}
