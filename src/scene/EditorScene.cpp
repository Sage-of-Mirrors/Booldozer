#include "scene/EditorScene.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <fstream>
#include <iostream>
#include <filesystem>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "border.h"
#include "ObserverIcon.hpp"
#include "imgui.h"
#include "ImGuizmo.h"
#include "Options.hpp"
#include "modes/EditorSelection.hpp"
#include "GLFW/glfw3.h"

struct cube_vertex {
	float x, y, z, u, v;
};

//hardcoded cube rendering data
static cube_vertex s_cubeVertices[] =
{
	{-1.0f,  1.0f,  1.0f,  0.0f,  0.0f },
	{ 1.0f,  1.0f,  1.0f,  1.0f,  0.0f },
	{-1.0f, -1.0f,  1.0f,  0.0f,  1.0f },
	{ 1.0f, -1.0f,  1.0f,  1.0f,  1.0f },
	{-1.0f,  1.0f, -1.0f,  0.0f,  0.0f},
	{ 1.0f,  1.0f, -1.0f,  1.0f,  0.0f},
	{-1.0f, -1.0f, -1.0f,  0.0f,  1.0f },
	{ 1.0f, -1.0f, -1.0f,  1.0f,  1.0f },
	{-1.0f,  1.0f,  1.0f,  0.0f,  0.0f},
	{ 1.0f,  1.0f,  1.0f,  1.0f,  0.0f},
	{-1.0f,  1.0f, -1.0f,  0.0f,  1.0f },
	{ 1.0f,  1.0f, -1.0f,  1.0f,  1.0f },
	{-1.0f, -1.0f,  1.0f,  0.0f,  0.0f},
	{ 1.0f, -1.0f,  1.0f,  1.0f,  0.0f},
	{-1.0f, -1.0f, -1.0f,  0.0f,  1.0f },
	{ 1.0f, -1.0f, -1.0f,  1.0f,  1.0f },
	{ 1.0f, -1.0f,  1.0f,  0.0f,  0.0f},
	{ 1.0f,  1.0f,  1.0f,  1.0f,  0.0f},
	{ 1.0f, -1.0f, -1.0f,  0.0f,  1.0f },
	{ 1.0f,  1.0f, -1.0f,  1.0f,  1.0f },
	{-1.0f, -1.0f,  1.0f,  0.0f,  0.0f},
	{-1.0f,  1.0f,  1.0f,  1.0f,  0.0f},
	{-1.0f, -1.0f, -1.0f,  0.0f,  1.0f },
	{-1.0f,  1.0f, -1.0f,  1.0f,  1.0f },
};

static const uint16_t s_cubeTriList[] = { 
	0,  2,  1,
	1,  2,  3,
	4,  5,  6,
	5,  7,  6,

	8, 10,  9,
	9, 10, 11,
	12, 13, 14,
	13, 15, 14,

	16, 18, 17,
	17, 18, 19,
	20, 21, 22,
	21, 23, 22,
};

void LCubeManager::render(glm::mat4* transform){

}

LCubeManager::LCubeManager(){}

void LCubeManager::init(){
	/*
	int x, y, n;
	uint8_t* data = stbi_load_from_memory(&cube_png[0], cube_png_size, &x, &y, &n, 4);
	stbi_image_free(data);
	*/
}

LCubeManager::~LCubeManager(){}


LEditorScene::LEditorScene() : Initialized(false) {}

LEditorScene::~LEditorScene(){}

void LEditorScene::init(){
	Initialized = true;
	mCubeManager.init();

	mDoorModels.reserve(14);
	for (size_t f = 0; f < GCResourceManager.mGameArchive.dirnum; f++)
	{
		if(std::string(GCResourceManager.mGameArchive.dirs[f].name) == "door"){
			for (size_t i = GCResourceManager.mGameArchive.dirs[f].fileoff; i < GCResourceManager.mGameArchive.dirs[f].fileoff + GCResourceManager.mGameArchive.dirs[f].filenum; i++)
			{
				bStream::CMemoryStream bin_data((uint8_t*)GCResourceManager.mGameArchive.files[i].data, GCResourceManager.mGameArchive.files[i].size, bStream::Endianess::Big, bStream::OpenMode::In);
				if(std::filesystem::path(GCResourceManager.mGameArchive.files[i].name).extension() == ".bin"){
					auto doorModel = std::make_shared<BinModel>(&bin_data);
					mDoorModels.push_back(doorModel);
				}
			}
			
		}
	}
	
}

glm::mat4 LEditorScene::getCameraView(){
	return Camera.GetViewMatrix();
}

glm::mat4 LEditorScene::getCameraProj(){
	return Camera.GetProjectionMatrix();
}

void LEditorScene::RenderSubmit(uint32_t m_width, uint32_t m_height){
	if (m_height == 0)
		m_height = 1;

	Camera.AspectRatio = float(m_width) / float(m_height);

	glm::mat4 view = Camera.GetViewMatrix();
	glm::mat4 proj = Camera.GetProjectionMatrix();

	//replace with gl equiv
	//bgfx::setViewTransform(0, &view[0][0], &proj[0][0]);
    //bgfx::setViewRect(0, 0, 0, uint16_t(m_width), uint16_t(m_height));
    //bgfx::touch(0);

	std::vector<glm::mat4> roomBounds;

	for(auto door : mRoomDoors){
		if (auto doorLocked = door.lock())
		{
			auto doorType = doorLocked->GetModel();
			if (doorType == EDoorModel::None)
				continue;

			// Construct transform matrix...
			glm::mat4 doorMat = glm::identity<glm::mat4>();

			// Translation. We need the translation from the transform matrix (column at [3]) as well as the Y scale (float at [1][1])
			doorMat = glm::translate(doorMat, glm::vec3((*doorLocked->GetMat())[3]) - glm::vec3(0, (*doorLocked->GetMat())[1][1] / 2.f, 0));

			// Rotation is based on the door's orientation type.
			if (doorLocked->GetOrientation() == EDoorOrientation::Side_Facing)
				doorMat = glm::rotate(doorMat, glm::radians(90.0f), glm::vec3(0, 1, 0));

			// The Square Mansion Door model is fucked, so this is a hack to make sure it shows up (mostly) correctly in the editor.
			bool bIgnoreTransforms = doorType == EDoorModel::Square_Mansion_Door;
			if (bIgnoreTransforms)
				doorMat = glm::translate(doorMat, glm::vec3(0, 0, 100));

			// Double doors need to be rendered twice, with the two halves moved accordingly.
			if (doorType == EDoorModel::Parlor_Double_Door || doorType == EDoorModel::Hearts_Double_Door)
			{
				glm::vec3 doubleDoorOffset = glm::vec3(0, 0, 100);

				// Offset the first door (right/forward) and render it.
				doorMat = glm::translate(doorMat, doubleDoorOffset);
				mDoorModels[(uint8_t)doorType - 1]->Draw(&doorMat);

				// Now offset the second door (left/backward) and rotate it 180 degrees.
				doubleDoorOffset *= 2;
				doorMat = glm::translate(doorMat, -doubleDoorOffset);
				doorMat = glm::rotate(doorMat, glm::radians(180.f), glm::vec3(0, 1, 0));

				// Render second door.
				mDoorModels[(uint8_t)doorType - 1]->Draw(&doorMat);
			}
			// Single door can just be rendered without hassle.
			else
			{
				mDoorModels[(uint8_t)doorType - 1]->Draw(&doorMat, bIgnoreTransforms);
			}
		}
	}

	for(auto room : mCurrentRooms){
		glm::mat4 identity = glm::identity<glm::mat4>();
		for (auto room : mRoomModels)
		{
			room->Draw(&identity);
		}
		
		if(!room.expired() && Initialized)
		{
			auto curRoom = room.lock();

			curRoom->ForEachChildOfType<LBGRenderDOMNode>(EDOMNodeType::BGRender, [&](auto node){
					glm::mat4 transform = glm::identity<glm::mat4>();
					switch (node->GetNodeType())
					{
					case EDOMNodeType::Furniture:
						if(mRoomFurniture.count(node->GetName()) == 0)
						{
							mCubeManager.render(node->GetMat());
						} else {
							mRoomFurniture[node->GetName()]->Draw(node->GetMat());
						}
						break;
					
					case EDOMNodeType::RoomData:
						roomBounds.push_back(glm::scale(glm::translate(transform, node->GetPosition()), node->GetScale()));
						break;

					case EDOMNodeType::Character:
					case EDOMNodeType::Generator:
					case EDOMNodeType::Observer:
					case EDOMNodeType::Object:
					case EDOMNodeType::Enemy:
						mCubeManager.render(node->GetMat());
						break;
				}
			});

		}
	}
	//Draw GL Lines Based thing for room boundaries
}

bool LEditorScene::HasRoomLoaded(int32_t roomNumber){
	for (auto& room : mCurrentRooms)
	{
		if(room.lock()->GetRoomNumber() == roomNumber) return true;
	}
	return false;
}

void LEditorScene::SetRoom(std::shared_ptr<LRoomDOMNode> room)
{
	// Get the select room's data so we can get the preload list
	auto roomData = room->GetChildrenOfType<LRoomDataDOMNode>(EDOMNodeType::RoomData);

	mRoomModels.clear();
	mRoomFurniture.clear();

	//This is ensured to exist, but check it anyway
	if(roomData.size() != 0)
	{
		mCurrentRooms = roomData.front()->GetAdjacencyList();
			
		mRoomDoors = roomData.front()->GetDoorList();

		for (auto& aroom :  roomData.front()->GetAdjacencyList())
		{
			
			auto curRoomData = aroom.lock()->GetChildrenOfType<LRoomDataDOMNode>(EDOMNodeType::RoomData).front();

			std::filesystem::path resPath = std::filesystem::path(OPTIONS.mRootPath) / "files" / std::filesystem::path(curRoomData->GetResourcePath()).relative_path();
			
			if(resPath.extension() == ".arc")
			{
				GCarchive roomArc;
				if(!GCResourceManager.LoadArchive(resPath.string().data(), &roomArc)){
					std::cout << "Unable to load room archive " << resPath << std::endl;
				}

				for(int file = 0; file < roomArc.filenum; file++)
				{
					std::filesystem::path curPath = std::filesystem::path(roomArc.files[file].name);
					if(curPath.extension() == ".bin")
					{
						bStream::CMemoryStream bin((uint8_t*)roomArc.files[file].data, roomArc.files[file].size, bStream::Endianess::Big, bStream::OpenMode::In);
						std::cout << "loading " << curPath.filename() << std::endl;
						if (curPath.filename().stem() != "room")
						{
							mRoomFurniture[curPath.filename().stem().string()] = std::make_shared<BinModel>(&bin);
							std::cout << "completed loading " << curPath.filename() << std::endl;
						} else {
							mRoomModels.push_back(std::make_shared<BinModel>(&bin));
							std::cout << "completed loading room model" << std::endl;
						}
						
					}
				}
				std::cout << "all models locked and loaded" << std::endl;
			} else {
				//If this is happening the map only has room models, no furniture.
				bStream::CFileStream bin(resPath.u8string(), bStream::Endianess::Big, bStream::OpenMode::In);
				mRoomModels.push_back(std::make_shared<BinModel>(&bin));
			}
		}
	}
}

void LEditorScene::update(GLFWwindow* window, float dt, LEditorSelection* selection)
{
	if(Camera.mCamMode == ECamMode::ORBIT){
		//oh god no
		Camera.SetCenter(mCurrentRooms[0].lock().get()->GetChildrenOfType<LRoomDataDOMNode>(EDOMNodeType::RoomData)[0].get()->GetPosition());
	}

	Camera.Update(window, dt);

	int w, h;
	int vx, vy;
	double x, y;
	glfwGetCursorPos(window, &x, &y);

	glfwGetWindowSize(window, &w, &h);
	glfwGetWindowPos(window, &vx, &vy);

	//TODO: replace with depth picking or other
}
