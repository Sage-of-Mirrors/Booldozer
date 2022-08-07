#include "DOM/MirrorDOMNode.hpp"
#include "DOM/RoomDOMNode.hpp"
#include "io/StaticMapDataIO.hpp"
#include "UIUtil.hpp"
#include "GenUtil.hpp"
#include "imgui.h"

LMirrorDOMNode::LMirrorDOMNode(std::string name) : Super(name),
	mCameraHeightOffset(0), mResolutionWidth(128), mResolutionWidth(128),
	mZoom(0.0f), mGBHOnly(false), mUnkValue1(0)
{
	mType = EDOMNodeType::Mirror;
}

std::string LMirrorDOMNode::GetName()
{
	return "Mirror"; //uh
}

void LMirrorDOMNode::RenderDetailsUI(float dt)
{
	LUIUtility::RenderTransformUI(mTransform.get(), mPosition, mRotation, mScale);

	LUIUtility::RenderCheckBox("GBH View Only", mGBHOnly);
}

bool LMirrorDOMNode::Load() //Load from file
{

	return true;
}

bool LMirrorDOMNode::Save() //Write to file
{

	return true;
}
