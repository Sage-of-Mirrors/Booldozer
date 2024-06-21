#include "DOM/MapCollisionDOMNode.hpp"
#include "DOM/RoomDOMNode.hpp"
#include "UIUtil.hpp"
#include "GenUtil.hpp"
#include "imgui.h"
#include "../lib/bStream/bstream.h"
#include "ImGuiFileDialog/ImGuiFileDialog.h"
#include "io/CollisionIO.hpp"

LMapCollisionDOMNode::LMapCollisionDOMNode(std::string name) : Super(name)
{
	mType = EDOMNodeType::MapCollision;
	SetIsRendered(false);
}

std::string LMapCollisionDOMNode::GetName()
{
	return "Collision"; //uh
}

void LMapCollisionDOMNode::RenderHierarchyUI(std::shared_ptr<LDOMNodeBase> self, LEditorSelection* mode_selection)
{
	LUIUtility::RenderCheckBox(this);
	ImGui::SameLine();
	ImGui::Text("Collision");
	if(ImGui::IsItemClicked(0)){
		mode_selection->ClearSelection();
		mode_selection->AddToSelection(self);
	}
	if(mWasRendered != GetIsRendered()){
		mWasRendered = GetIsRendered();
		mDirty = true;
	}
}

void LMapCollisionDOMNode::RenderDetailsUI(float dt)
{
	std::string path;
	if(ImGui::Button("Import Mp")){
		ImGuiFileDialog::Instance()->OpenDialog("ImportMpDlg", "Import Mp", "LM Collision Map (*.mp){.mp}", std::filesystem::current_path().string());
	}

	if(ImGui::Button("Import OBJ")){
		ImGuiFileDialog::Instance()->OpenDialog("ImportObjColDlg", "Import OBJ", "Wavefront Obj (*.obj){.obj}", std::filesystem::current_path().string());
	}

	if (LUIUtility::RenderFileDialog("ImportMpDlg", path))
	{
		auto map = GetParentOfType<LMapDOMNode>(EDOMNodeType::Map);

		LCollisionIO col;
		col.LoadMp(std::filesystem::path(path), GetParentOfType<LMapDOMNode>(EDOMNodeType::Map));

		auto mapArc = map.lock()->GetArchive().lock();
		auto colFile = mapArc->GetFile("col.mp");

		bStream::CMemoryStream colStream(colFile->GetData(), colFile->GetSize(), bStream::Endianess::Big, bStream::OpenMode::In);
		Load(&colStream);
		mDirty = true;
	}

	if (LUIUtility::RenderFileDialog("ImportObjColDlg", path))
	{
		auto map = GetParentOfType<LMapDOMNode>(EDOMNodeType::Map);

		LCollisionIO col;
		col.LoadObj(std::filesystem::path(path), GetParentOfType<LMapDOMNode>(EDOMNodeType::Map));

		auto mapArc = map.lock()->GetArchive().lock();
		auto colFile = mapArc->GetFile("col.mp");

		bStream::CMemoryStream colStream(colFile->GetData(), colFile->GetSize(), bStream::Endianess::Big, bStream::OpenMode::In);
		Load(&colStream);
		mDirty = true;

	}

}

bool LMapCollisionDOMNode::Load(bStream::CMemoryStream* stream)
{
    
	mGridScale = glm::vec3(stream->readFloat(), stream->readFloat(), stream->readFloat());
	mMinBounds = glm::vec3(stream->readFloat(), stream->readFloat(), stream->readFloat());
	mAxisLengths = glm::vec3(stream->readFloat(), stream->readFloat(), stream->readFloat());

	uint32_t vertexDataOffset = stream->readUInt32();
	uint32_t normalDataOffset = stream->readUInt32();
	uint32_t triangleDataOffset = stream->readUInt32();
	uint32_t triangleGroupOffset = stream->readUInt32();
	uint32_t gridIndexOffset = stream->readUInt32();
	
	//	duplicated for some reason
	gridIndexOffset = stream->readUInt32();

	uint32_t endOffset = stream->readUInt32();

	stream->seek(vertexDataOffset);

	for (size_t i = 0; i < (normalDataOffset - vertexDataOffset) / 0x0C; i++)
	{
		mPositionData.push_back(glm::vec3(stream->readFloat(), stream->readFloat(), stream->readFloat()));
	}
	
	stream->seek(normalDataOffset);
	for (size_t i = 0; i < (triangleDataOffset - normalDataOffset) / 0x0C; i++)
	{
		mNormalData.push_back(glm::vec3(stream->readFloat(), stream->readFloat(), stream->readFloat()));
	}

	stream->seek(triangleDataOffset);
	for (size_t i = 0; i < (triangleGroupOffset - triangleDataOffset) / 0x18; i++)
	{
		std::shared_ptr<LCollisionTriangle> triangle = std::make_shared<LCollisionTriangle>();

		triangle->name = fmt::format("Triangle {}", i);

		triangle->positionIdx[0] = stream->readUInt16();
		triangle->positionIdx[1] = stream->readUInt16();
		triangle->positionIdx[2] = stream->readUInt16();

		triangle->normalIdx = stream->readUInt16();

		triangle->edgeTanIdx[0] = stream->readUInt16();
		triangle->edgeTanIdx[1] = stream->readUInt16();
		triangle->edgeTanIdx[2] = stream->readUInt16();

		triangle->unkIdx = stream->readUInt16();
		triangle->dot = stream->readFloat();
		triangle->enabledMask = stream->readUInt16();
		triangle->friction = stream->readUInt16();

		mTriangles.push_back(triangle);
	}

	std::cout << "[CollisionDOMNode]: Finished reading Collision File" << std::endl;

	return false;
}

bool LMapCollisionDOMNode::Save(bStream::CMemoryStream* stream)
{
	return false;
}

void LMapCollisionDOMNode::PostProcess()
{

}

void LMapCollisionDOMNode::PreProcess()
{

}
