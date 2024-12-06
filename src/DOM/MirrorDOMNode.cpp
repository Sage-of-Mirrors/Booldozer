#include "DOM/MirrorDOMNode.hpp"
#include "DOM/RoomDOMNode.hpp"
#include "io/StaticMapDataIO.hpp"
#include "UIUtil.hpp"
#include "GenUtil.hpp"
#include "imgui.h"
#include "../lib/bStream/bstream.h"

LMirrorDOMNode::LMirrorDOMNode(std::string name) : Super(name),
	mCameraHeightOffset(0), mResolutionWidth(128), mResolutionHeight(128),
	mZoom(20.0f), mGBHOnly(false), mCameraDistance(550.0f)
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
	
	// Integers
	ImGui::InputInt("Resolution Width", &mResolutionWidth);
	LUIUtility::RenderTooltip("The width component of the resolution of the texture the mirror renders to.");

	ImGui::InputInt("Resolution Height", &mResolutionHeight);
	LUIUtility::RenderTooltip("The height component of the resolution of the texture the mirror renders to.");

	ImGui::InputInt("Camera Height Offset", &mCameraHeightOffset);
	LUIUtility::RenderTooltip("How far above or below the mirror's position the render camera is located.");

	// Floats
	ImGui::InputFloat("Zoom", &mZoom);
	LUIUtility::RenderTooltip("How much the render camera is zoomed in or out.");

	ImGui::InputFloat("Camera Distance", &mCameraDistance);
	LUIUtility::RenderTooltip("How far the render camera is from the mirror.");

	// Bools
	LUIUtility::RenderCheckBox("GBH View Only", &mGBHOnly);
	LUIUtility::RenderTooltip("Whether this mirror is visible only when using the GameBoy Horror. Used for mirrors on the fourth wall.");
}

bool LMirrorDOMNode::Load(const nlohmann::ordered_json& jsonEntry)
{
	auto position = jsonEntry.at("Position");
	mPosition = glm::vec3(position[2], position[1], position[0]);

	auto rotation = jsonEntry.at("Rotation");
	mRotation = glm::vec3(rotation[2], rotation[1], rotation[0]);
	mRotation.y = -mRotation.y;
	
	auto scale = jsonEntry.at("Scale");
	mScale = glm::vec3(scale[0], scale[1], scale[2]);

	jsonEntry.at("ImageBaseWidth").get_to(mResolutionWidth);
	jsonEntry.at("ImageBaseHeight").get_to(mResolutionHeight);

	jsonEntry.at("RenderCameraZoom").get_to(mZoom);
	jsonEntry.at("RenderCameraVerticalOffset").get_to(mCameraHeightOffset);
	jsonEntry.at("RenderCameraDistance").get_to(mCameraDistance);

	jsonEntry.at("RenderGBHOnly").get_to(mGBHOnly);

	return true;
}

bool LMirrorDOMNode::Load(bStream::CFileStream* stream)
{
	try
	{
		float posX, posY, posZ;
		posX = stream->readFloat();
		posY = stream->readFloat();
		posZ = stream->readFloat();
		mPosition = glm::vec3(posZ, posY, posX);

		float scaleX, scaleY, scaleZ;
		scaleX = stream->readFloat();
		scaleY = stream->readFloat();
		scaleZ = stream->readFloat();
		mScale = glm::vec3(scaleX, scaleY, scaleZ);

		float rotX, rotY, rotZ;
		rotX = stream->readInt32() * (180.0f / 32768.0f);
		rotY = -stream->readInt32() * (180.0f / 32768.0f);
		rotZ = stream->readInt32() * (180.0f / 32768.0f);
		mRotation = glm::vec3(rotZ, rotY, rotX);

		mCameraHeightOffset = stream->readInt32();
		mCameraDistance = stream->readFloat();

		mResolutionWidth = stream->readUInt16();
		mResolutionHeight = stream->readUInt16();

		mZoom = stream->readFloat();
		mGBHOnly = stream->readInt32();
	}
	catch (std::exception e)
	{
		return false;
	}
	

	return true;
}

bool LMirrorDOMNode::Save(bStream::CMemoryStream* stream)
{
	try
	{
		stream->writeFloat(mPosition.z);
		stream->writeFloat(mPosition.y);
		stream->writeFloat(mPosition.x);

		stream->writeFloat(mScale.x);
		stream->writeFloat(mScale.y);
		stream->writeFloat(mScale.z);

		stream->writeInt32(mRotation.z * (32678.0f / 180.0f));
		stream->writeInt32(-mRotation.y * (32678.0f / 180.0f));
		stream->writeInt32(mRotation.x * (32678.0f / 180.0f));

		stream->writeInt32(mCameraHeightOffset);
		stream->writeFloat(mCameraDistance);

		stream->writeUInt16(mResolutionWidth);
		stream->writeUInt16(mResolutionHeight);

		stream->writeFloat(mZoom);
		stream->writeInt32(mGBHOnly);
	}
	catch (std::exception e)
	{
		return false;
	}

	return true;
}

void LMirrorDOMNode::PostProcess()
{
	auto mapNode = GetParentOfType<LMapDOMNode>(EDOMNodeType::Map);
	if (auto mapNodeLocked = mapNode.lock())
	{
		auto rooms = mapNodeLocked->GetChildrenOfType<LRoomDOMNode>(EDOMNodeType::Room);
		std::shared_ptr<LRoomDOMNode> containingRoom = nullptr;

		for (auto r : rooms)
		{
			auto roomData = r->GetChildrenOfType<LRoomDataDOMNode>(EDOMNodeType::RoomData)[0];

			if (roomData->CheckPointInBounds(mPosition))
			{
				containingRoom = r;
				break;
			}
		}

		if (containingRoom != nullptr)
		{
			LGenUtility::Log << std::format("[MirrorDOMNode]: Mirror going to {0}", containingRoom->GetName()) << std::endl;
			containingRoom->AddChild(GetSharedPtr<LMirrorDOMNode>(EDOMNodeType::Mirror));
			mapNodeLocked->RemoveChild(GetSharedPtr<LMirrorDOMNode>(EDOMNodeType::Mirror));
		}
	}
}

void LMirrorDOMNode::PreProcess()
{

}
