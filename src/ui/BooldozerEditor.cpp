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
	CurrentMode = EEditorMode::Actor_Mode;
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
	mLoadedMap->LoadMap(std::filesystem::path("D:\\SZS Tools\\Luigi's Mansion\\root\\files\\Map\\map2.szp")); /* Gamma PC */
	//mLoadedMap->LoadMap(std::filesystem::path("/home/mu/GC Games/Luigi's Mansion/files/Map/map2.szp")); /* Gamma Laptop */ 
	//mLoadedMap->LoadMap(std::filesystem::path("/home/spacey/Projects/LuigisMansion/Mods/LMArcade/files/Map/map2.szp")); /* Space */
}

void LBooldozerEditor::onOpenRoomsCB()
{
	std::cout << "User selected open room(s)!" << std::endl;
}

void LBooldozerEditor::onSaveMapCB()
{
	if (mLoadedMap != nullptr)
		mLoadedMap->SaveMap(std::filesystem::path("D:\\SZS Tools\\Luigi's Mansion\\Booldozer\\jmptest"));
}

void LBooldozerEditor::ChangeMode()
{
	if (mCurrentMode != nullptr)
		mCurrentMode->OnBecomeInactive();
	
	switch(CurrentMode)
	{
		case EEditorMode::Actor_Mode:
			mCurrentMode = &mActorMode;
			break;
		case EEditorMode::Collision_Mode:
			break;
		case EEditorMode::Door_Mode:
			break;
		case EEditorMode::Enemy_Mode:
			mCurrentMode = &mEnemyMode;
			break;
		case EEditorMode::Event_Mode:
			break;
		case EEditorMode::Item_Mode:
			break;
		default:
			mCurrentMode = nullptr;
			break;
	}

	if (mCurrentMode != nullptr)
		mCurrentMode->OnBecomeActive();
}
