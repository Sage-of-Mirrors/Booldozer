#include "DOM/MapCollisionDOMNode.hpp"
#include "DOM/RoomDOMNode.hpp"
#include "UIUtil.hpp"
#include "GenUtil.hpp"
#include "imgui.h"
#include "../lib/bStream/bstream.h"
#include "ImGuiFileDialog/ImGuiFileDialog.h"
#include "io/CollisionIO.hpp"
#include <thread>
#include <mutex>
#include "scene/EditorScene.hpp"

#include <Options.hpp>

namespace {
	std::thread importModelThread {};
	std::mutex importLock {};
	bool isImportingCol { false };
}

LMapCollisionDOMNode::LMapCollisionDOMNode(std::string name) : Super(name)
{
	mType = EDOMNodeType::MapCollision;
	SetIsRendered(false);
}

std::string LMapCollisionDOMNode::GetName()
{
	return "Collision"; //uh
}

void LMapCollisionDOMNode::ImportObj(std::string path){
	auto map = GetParentOfType<LMapDOMNode>(EDOMNodeType::Map);

	LCollisionIO col;
	col.LoadObj(std::filesystem::path(path), GetParentOfType<LMapDOMNode>(EDOMNodeType::Map), mMatColProp, mBakeFurniture);

	auto mapArc = map.lock()->GetArchive().lock();
	auto colFile = mapArc->GetFile("col.mp");

	bStream::CMemoryStream colStream(colFile->GetData(), colFile->GetSize(), bStream::Endianess::Big, bStream::OpenMode::In);
	Load(&colStream);

	LEditorScene::SetDirty();

	importLock.lock();
	isImportingCol = false;
	importLock.unlock();
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
		LEditorScene::SetDirty();
	}
}

void LMapCollisionDOMNode::RenderDetailsUI(float dt)
{
	std::string path;
	bool bakeColClicked = false;

	if(ImGui::BeginTabBar("##colImportFormats", ImGuiTabBarFlags_None)){
		if(ImGui::BeginTabItem("Mp")){
			if(ImGui::Button("Import")){
				LUIUtility::RenderTooltip("Import an existing col.mp");
				ImGuiFileDialog::Instance()->OpenDialog("ImportMpDlg", "Import Mp", "LM Collision Map (*.mp){.mp}", std::filesystem::current_path().string());
			}
			ImGui::EndTabItem();
		}

		if(ImGui::BeginTabItem("Obj")){

			LUIUtility::RenderCheckBox("Bake Furniture", &mBakeFurniture);

			ImGui::Text("Material Settings");
			LUIUtility::RenderTooltip("Custom OBJ Material Property names to load Collision Properties from");

			ImGui::Text("Group");
			ImGui::SameLine();
			LUIUtility::RenderTextInput("##matPropertyGroup", &mMatColProp["Group"]);
			LUIUtility::RenderTooltip("Name of material property with group bitmask");

			ImGui::Text("Sound");
			ImGui::SameLine();
			LUIUtility::RenderTextInput("##matPropertySound", &mMatColProp["Sound"]);
			LUIUtility::RenderTooltip("Name of material property with sound index data");

			ImGui::Text("Sound Echo Switch");
			ImGui::SameLine();
			LUIUtility::RenderTextInput("##matPropertySndEcho", &mMatColProp["SoundEchoSwitch"]);
			LUIUtility::RenderTooltip("Name of material property with sound echo switch");

			ImGui::Text("Friction");
			ImGui::SameLine();
			LUIUtility::RenderTextInput("##matPropertyFric", &mMatColProp["Friction"]);
			LUIUtility::RenderTooltip("Name of material property with friction value");

			ImGui::Text("Ladder");
			ImGui::SameLine();
			LUIUtility::RenderTextInput("##matPropertyLadder", &mMatColProp["Ladder"]);
			LUIUtility::RenderTooltip("Name of material property with ladder flag. Unused.");
			
			ImGui::Text("IgnorePointer");
			ImGui::SameLine();
			LUIUtility::RenderTextInput("##matPropertyIgnorePointer", &mMatColProp["IgnorePointer"]);
			LUIUtility::RenderTooltip("Name of material property with IgnorePointer");

			ImGui::Text("Surface Material");
			ImGui::SameLine();
			LUIUtility::RenderTextInput("##matPropertyColMaterial", &mMatColProp["SurfaceMaterial"]);
			LUIUtility::RenderTooltip("Unsure?");

			if(ImGui::Button("Import")){
				ImGuiFileDialog::Instance()->OpenDialog("ImportObjColDlg", "Import OBJ", "Wavefront Obj (*.obj){.obj}", std::filesystem::current_path().string());
			}
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}


    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopupModal("Importing Obj", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar))
	{
        const ImU32 col = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
		ImGui::AlignTextToFramePadding();
		ImGui::Spinner("##loadmapSpinner", 5.0f, 2, col);
		ImGui::SameLine();
		ImGui::Text("Importing Obj Collision...");
		
		importLock.lock();
		if(isImportingCol == false){
			LGenUtility::Log << "[BooldozerEditor]: Joining Import Thread" << std::endl;
			importModelThread.join();
			ImGui::CloseCurrentPopup();
		}
		importLock.unlock();

		ImGui::EndPopup();
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
	
		LEditorScene::SetDirty();

	}

	if (LUIUtility::RenderFileDialog("ImportObjColDlg", path))
	{
		// shouldn't _need_ to worry about this but just in case - lock
		importLock.lock();
		isImportingCol = true;
		importLock.unlock();

		importModelThread = std::thread(&LMapCollisionDOMNode::ImportObj, std::ref(*this), path);
		
		ImGui::OpenPopup("Importing Obj");

	}

}

bool LMapCollisionDOMNode::Load(bStream::CMemoryStream* stream)
{

	mModel.mVertices.clear();
	mModel.mNormals.clear();
	mModel.mTriangles.clear();
    
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
		mModel.mVertices.push_back(glm::vec3(stream->readFloat(), stream->readFloat(), stream->readFloat()));
	}
	
	stream->seek(normalDataOffset);
	for (size_t i = 0; i < (triangleDataOffset - normalDataOffset) / 0x0C; i++)
	{
		mModel.mNormals.push_back(glm::vec3(stream->readFloat(), stream->readFloat(), stream->readFloat()));
	}

	stream->seek(triangleDataOffset);
	for (size_t i = 0; i < (triangleGroupOffset - triangleDataOffset) / 0x18; i++)
	{
		CollisionTriangle triangle;

		triangle.mVtx1 = stream->readUInt16();
		triangle.mVtx2 = stream->readUInt16();
		triangle.mVtx3 = stream->readUInt16();

		triangle.mNormal = stream->readUInt16();

		triangle.mEdgeTan1 = stream->readUInt16();
		triangle.mEdgeTan2 = stream->readUInt16();
		triangle.mEdgeTan3 = stream->readUInt16();

		triangle.mUnkIdx = stream->readUInt16();
		triangle.mDot = stream->readFloat();
		triangle.mMask = stream->readUInt16();
		triangle.mFriction = stream->readUInt16();

		mModel.mTriangles.push_back(triangle);
	}

	LGenUtility::Log << "[CollisionDOMNode]: Finished reading Collision File" << std::endl;

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
