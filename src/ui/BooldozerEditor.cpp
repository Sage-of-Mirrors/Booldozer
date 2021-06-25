#include "ui/BooldozerEditor.hpp"
#include "JmpIO.hpp"
#include "../lib/libgctools/include/compression.h"
#include "../lib/bStream/bstream.h"
#include "DOM/FurnitureDOMNode.hpp"
#include <iostream>
#include <vector>
#include <memory>

LBooldozerEditor::LBooldozerEditor()
{

}

void LBooldozerEditor::onOpenMapCB()
{
	mLoadedMap.LoadMap(std::filesystem::path("/home/spacey/Projects/LuigisMansion/Mods/LMArcade/files/Map/map2.szp"));
}

void LBooldozerEditor::onOpenRoomsCB()
{
	std::cout << "User selected open room(s)!" << std::endl;
}
