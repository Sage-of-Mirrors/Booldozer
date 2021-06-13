#include "ui/BooldozerEditor.hpp"
#include <iostream>

LBooldozerEditor::LBooldozerEditor()
{

}

void LBooldozerEditor::onOpenMapCB()
{
	std::cout << "User selected open map!" << std::endl;
}

void LBooldozerEditor::onOpenRoomsCB()
{
	std::cout << "User selected open room(s)!" << std::endl;
}
