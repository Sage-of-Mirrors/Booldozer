#include "scene/EditorScene.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "imgui.h"
#include "ImGuizmo.h"
#include "Options.hpp"
#include "modes/EditorSelection.hpp"
#include "GLFW/glfw3.h"
#include "io/BinIO.hpp"
#include <io/MdlIO.hpp>

#include "DOM/EventDOMNode.hpp"
#include "DOM/GeneratorDOMNode.hpp"
#include "DOM/PathDOMNode.hpp"
#include "DOM/EnemyDOMNode.hpp"

#include <J3D/J3DUniformBufferObject.hpp>
#include <J3D/J3DModelLoader.hpp>
#include <J3D/J3DRendering.hpp>

LEditorScene::LEditorScene() : Initialized(false) {}

LEditorScene::~LEditorScene(){
	BinModel::DestroyShaders();
	MDL::DestroyShaders();
}

void LEditorScene::init(){
	Initialized = true;

	mPathRenderer.Init();
	mPointManager.Init(512, 9);
	
	mPointManager.SetBillboardTexture(std::filesystem::current_path() / "res" / "img" / "ice_generator.png", 0);
	mPointManager.SetBillboardTexture(std::filesystem::current_path() / "res" / "img" / "fire_generator.png", 1);
	mPointManager.SetBillboardTexture(std::filesystem::current_path() / "res" / "img" / "water_generator.png", 2);

	mPointManager.SetBillboardTexture(std::filesystem::current_path() / "res" / "img" / "event.png", 3);
	mPointManager.SetBillboardTexture(std::filesystem::current_path() / "res" / "img" / "observer.png", 4);
	mPointManager.SetBillboardTexture(std::filesystem::current_path() / "res" / "img" / "enemy_placeholder.png", 5);

	mPointManager.SetBillboardTexture(std::filesystem::current_path() / "res" / "img" / "soundobj.png", 6);

	BinModel::InitShaders();
	MDL::InitShaders();

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
	
	J3DModelLoader Loader;
	
	GCarchive vrballArchive;

	if(GCResourceManager.mGameArchive.filenum != 0){
		GCarcfile* coinModelFile = GCResourceManager.GetFile(&GCResourceManager.mGameArchive, std::filesystem::path("kt_static") / "coin.bmd");
		if(coinModelFile != nullptr){
			bStream::CMemoryStream modelData((uint8_t*)coinModelFile->data, coinModelFile->size, bStream::Endianess::Big, bStream::OpenMode::In);
			mCoinModel = Loader.Load(&modelData, 0);
		} else {
			std::cout << "Couldn't find coin" << std::endl;
		}
	}

	if((std::filesystem::exists(std::filesystem::path(OPTIONS.mRootPath) / "files" / "Iwamoto" / "vrball_M.szp"))){

		if(!GCResourceManager.LoadArchive((std::filesystem::path(OPTIONS.mRootPath) / "files" / "Iwamoto" / "vrball_M.szp").string().c_str(), &vrballArchive)){
			std::cout << "skybox problem oop" << std::endl;
		}

		if(vrballArchive.filenum != 0){
			GCarcfile* skyboxModel = GCResourceManager.GetFile(&vrballArchive, std::filesystem::path("vrball01.bmd"));

			if(skyboxModel != nullptr){
				bStream::CMemoryStream modelData((uint8_t*)skyboxModel->data, skyboxModel->size, bStream::Endianess::Big, bStream::OpenMode::In);
				mSkyboxModel = Loader.Load(&modelData, 0);
				mSkyBox = mSkyboxModel->GetInstance();
			} else {
				std::cout << "Couldn't find skybox" << std::endl;
			}

			gcFreeArchive(&vrballArchive);
		}
	}
}

glm::mat4 LEditorScene::getCameraView(){
	return Camera.GetViewMatrix();
}

glm::mat4 LEditorScene::getCameraProj(){
	return Camera.GetProjectionMatrix();
}

void LEditorScene::UpdateRenderers(){
	std::cout << "calling update renderers. this should only happen a few times!" << std::endl;
	mPathRenderer.mPaths.clear();
	mPointManager.mBillboards.clear();
	for(auto room : mCurrentRooms){
		if(!room.expired() && Initialized)
		{
			auto curRoom = room.lock();

			curRoom->ForEachChildOfType<LBGRenderDOMNode>(EDOMNodeType::BGRender, [&](auto node){
					if(!node->GetIsRendered()) return;
					switch (node->GetNodeType())
					{
					case EDOMNodeType::RoomData:
						{
							glm::vec4 color = (curRoom->GetRoomNumber() == mSelectedRoomNumber ? glm::vec4(0.0f, 1.0f, 0.0f, 1.0f) : glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

							glm::vec3 min = static_cast<LRoomDataDOMNode*>(node.get())->GetMin();
							glm::vec3 max = static_cast<LRoomDataDOMNode*>(node.get())->GetMax();
							std::vector<CPathPoint> bounds_bottom = {
								{min, color, 51200},
								{{min.x, min.y, max.z}, color, 51200},
								{{max.x, min.y, max.z}, color, 51200},
								{{max.x, min.y, min.z}, color, 51200},
								{min, color, 51200},
							};

							std::vector<CPathPoint> bounds_top = {
								{max, color, 51200},
								{{max.x, max.y, min.z}, color, 51200},
								{{min.x, max.y, min.z}, color, 51200},
								{{min.x, max.y, max.z}, color, 51200},
								{max, color, 51200},
							};

							std::vector<CPathPoint> bounds_edge1 = {{min, color, 51200}, {{min.x, max.y, min.z}, color, 51200}};
							std::vector<CPathPoint> bounds_edge2 = {{{max.x, min.y, min.z}, color, 51200}, {{max.x, max.y, min.z}, color, 51200}};
							std::vector<CPathPoint> bounds_edge3 = {{{min.x, min.y, max.z}, color, 51200}, {{min.x, max.y, max.z}, color, 51200}};
							std::vector<CPathPoint> bounds_edge4 = {{max, color, 51200}, {{max.x, min.y, max.z}, color, 51200}};

							mPathRenderer.mPaths.push_back(bounds_bottom);
							mPathRenderer.mPaths.push_back(bounds_top);

							mPathRenderer.mPaths.push_back(bounds_edge1);
							mPathRenderer.mPaths.push_back(bounds_edge2);
							mPathRenderer.mPaths.push_back(bounds_edge3);
							mPathRenderer.mPaths.push_back(bounds_edge4);
						}
						break;
					case EDOMNodeType::Path:
						{
							std::vector<CPathPoint> path;
							
							auto points = node->template GetChildrenOfType<LPathPointDOMNode>(EDOMNodeType::PathPoint);
							
							for(auto& point : points){
								path.push_back({point->GetPosition(), static_cast<LPathDOMNode*>(node.get())->mPathColor, 12800});
							}
							
							mPathRenderer.mPaths.push_back(path);
						}
						break;
					case EDOMNodeType::Generator:
						if (node->GetName() == "elice"){
							mPointManager.mBillboards.push_back({node->GetPosition(), 51200, 0, false, node->GetID()});
						} else if(node->GetName() == "elfire"){
							mPointManager.mBillboards.push_back({node->GetPosition(), 51200, 1, false, node->GetID()});
						} else if(node->GetName() == "elwater") {
							mPointManager.mBillboards.push_back({node->GetPosition(), 51200, 2, false, node->GetID()});
						}
						break;
					case EDOMNodeType::Event:
						mPointManager.mBillboards.push_back({node->GetPosition(), 51200, 3, false, node->GetID()});
						break;
					case EDOMNodeType::Observer:
						if(node->GetName() == "Sound Effect Player"){
							mPointManager.mBillboards.push_back({node->GetPosition(), 51200, 6, false, node->GetID()});
						} else {
							mPointManager.mBillboards.push_back({node->GetPosition(), 51200, 4, false, node->GetID()});
						}
						break;
					//case EDOMNodeType::Object:
					//	break;
					//case EDOMNodeType::Enemy:
					//	mPointManager.mBillboards.push_back({node->GetPosition(), 51200, 5, false, false});
					//	break;
				}
			});

		}
	}
	mPointManager.UpdateData();
	mPathRenderer.UpdateData();
}

// This whole thing is so so SO awful.

void LEditorScene::RenderSubmit(uint32_t m_width, uint32_t m_height){
	if (m_height == 0)
		m_height = 1;

	Camera.AspectRatio = float(m_width) / float(m_height);

	glm::mat4 view = Camera.GetViewMatrix();
	glm::mat4 proj = Camera.GetProjectionMatrix();

	J3DUniformBufferObject::SetProjAndViewMatrices(&proj, &view);

	std::vector<std::shared_ptr<J3DModelInstance>> renderables;

	for(std::weak_ptr<LRoomDOMNode> room : mCurrentRooms){
		if(!room.expired() && Initialized)
		{
			std::shared_ptr<LRoomDOMNode> roomLocked = room.lock();
			//vrball01.bmd
			if(roomLocked->GetSkyboxEnabled() && mSkyBox != nullptr){
				mSkyBox->SetTranslation(roomLocked->GetPosition());
				mSkyBox->SetRotation({0,0,0});
				mSkyBox->SetScale({15,15,15});
				renderables.push_back(mSkyBox);
				break;
			}
		}
	}

	glFrontFace(GL_CW);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	for(std::weak_ptr<LDoorDOMNode> doorRef : mRoomDoors){
		if (std::shared_ptr<LDoorDOMNode> door = doorRef.lock())
		{
			EDoorModel doorType = door->GetModel();
			if (doorType == EDoorModel::None)
				continue;

			// Construct transform matrix...
			glm::mat4 doorMat = glm::identity<glm::mat4>();

			// Translation. We need the translation from the transform matrix (column at [3]) as well as the Y scale (float at [1][1])
			doorMat = glm::translate(doorMat, glm::vec3((*door->GetMat())[3]) - glm::vec3(0, (*door->GetMat())[1][1] / 2.f, 0));

			// Rotation is based on the door's orientation type.
			if (door->GetOrientation() == EDoorOrientation::Side_Facing)
				doorMat = glm::rotate(doorMat, glm::radians(90.0f), glm::vec3(0, 1, 0));

			// The Square Mansion Door model is fucked, so this is a hack to make sure it shows up (mostly) correctly in the editor.
			bool bIgnoreTransforms = doorType == EDoorModel::Square_Mansion_Door;
			if (bIgnoreTransforms)
				doorMat = glm::translate(doorMat, glm::vec3(0, 0, 1000));

			// Double doors need to be rendered twice, with the two halves moved accordingly.
			if (doorType == EDoorModel::Parlor_Double_Door || doorType == EDoorModel::Hearts_Double_Door)
			{
				glm::vec3 doubleDoorOffset = glm::vec3(0, 0, 1000);

				// Offset the first door (right/forward) and render it.
				doorMat = glm::translate(doorMat, doubleDoorOffset);
				mDoorModels[(uint8_t)doorType - 1]->Draw(&doorMat, door->GetID());

				// Now offset the second door (left/backward) and rotate it 180 degrees.
				doubleDoorOffset *= 2;
				doorMat = glm::translate(doorMat, -doubleDoorOffset);
				doorMat = glm::rotate(doorMat, glm::radians(180.f), glm::vec3(0, 1, 0));

				// Render second door.
				mDoorModels[(uint8_t)doorType - 1]->Draw(&doorMat, door->GetID());
			}
			// Single door can just be rendered without hassle.
			else
			{
				mDoorModels[(uint8_t)doorType - 1]->Draw(&doorMat, door->GetID(), bIgnoreTransforms);
			}
		}
	}

	for(std::weak_ptr<LRoomDOMNode> roomRef : mCurrentRooms){

		glm::mat4 identity = glm::identity<glm::mat4>();
		for (std::shared_ptr<BinModel> room : mRoomModels)
		{
			room->Draw(&identity, -1);
		}
		
		std::shared_ptr<LRoomDOMNode> curRoom;

		if((curRoom = roomRef.lock()) && Initialized)
		{

			curRoom->ForEachChildOfType<LBGRenderDOMNode>(EDOMNodeType::BGRender, [&](auto node){
					if(!node->GetIsRendered()) return;
					
					//TODO: Render MDL if type is character

					switch (node->GetNodeType())
					{
					case EDOMNodeType::Furniture:
						if(mRoomFurniture.count(std::static_pointer_cast<LFurnitureDOMNode>(node)->GetModelName()) == 0)
						{
							//render some other way
						} else {
							mRoomFurniture[std::static_pointer_cast<LFurnitureDOMNode>(node)->GetModelName()]->Draw(node->GetMat(), node->GetID());
						}
					case EDOMNodeType::Object:
						if(node->GetName() == "coin"){
							auto coin = mCoinModel->GetInstance();
							coin->SetTransform(*node->GetMat());
							renderables.push_back(coin);
						}
						break;
					case EDOMNodeType::Character:
					case EDOMNodeType::Enemy:
					case EDOMNodeType::Observer:
					case EDOMNodeType::Generator:
					case EDOMNodeType::Key:
						if(mActorModels.contains(node->GetName())){
							if(mMaterialAnimations.contains(node->GetName())){
								mActorModels[node->GetName()]->Draw(node->GetMat(), node->GetID(), mMaterialAnimations[node->GetName()].get());
							} else {
								mActorModels[node->GetName()]->Draw(node->GetMat(), node->GetID(), nullptr);
							}
						}
						break;
					}
			});

		}
	}
	//Draw GL Lines Based thing for room boundaries

	mPointManager.Draw(&Camera);
	mPathRenderer.Draw(&Camera);

	// j3d
	J3DRendering::Render(0, Camera.GetCenter(), view, proj, renderables);
}

bool LEditorScene::HasRoomLoaded(int32_t roomNumber){
	for (std::weak_ptr<LRoomDOMNode>& room : mCurrentRooms)
	{
		if(!room.expired() && room.lock()->GetRoomNumber() == roomNumber) return true;
	}
	return false;
}

void LEditorScene::SetRoom(std::shared_ptr<LRoomDOMNode> room)
{
	// Get the select room's data so we can get the preload list
	std::vector<std::shared_ptr<LRoomDataDOMNode>> roomData = room->GetChildrenOfType<LRoomDataDOMNode>(EDOMNodeType::RoomData);
	mSelectedRoomNumber = room->GetRoomNumber();

	mRoomModels.clear();
	mRoomFurniture.clear();

	mActorModels.clear();
	mMaterialAnimations.clear();

	//This is ensured to exist, but check it anyway
	if(roomData.size() != 0)
	{
		mCurrentRooms = roomData.front()->GetAdjacencyList();
			
		mRoomDoors = roomData.front()->GetDoorList();

		for (std::weak_ptr<LRoomDOMNode>& adjacentRoomRef :  roomData.front()->GetAdjacencyList())
		{
			std::shared_ptr<LRoomDOMNode> adjacentRoom;

			if(!(adjacentRoom = adjacentRoomRef.lock())){
				continue;
			}

			adjacentRoom->ForEachChildOfType<LBGRenderDOMNode>(EDOMNodeType::BGRender, [&](auto node){
				if(node->GetNodeType() == EDOMNodeType::Character || node->GetNodeType() == EDOMNodeType::Enemy || node->GetNodeType() == EDOMNodeType::Observer || node->GetNodeType() == EDOMNodeType::Generator || node->GetNodeType() == EDOMNodeType::Key){
					std::string name = node->GetName();
					
					std::tuple<std::string, std::string, bool> actorRef = LResUtility::GetActorModelFromName(name);

					if(mActorModels.count(name) != 0 && (mMaterialAnimations.count(name) != 0 && std::get<1>(actorRef) != "")) return;

					std::filesystem::path modelPath = std::filesystem::path(OPTIONS.mRootPath) / "files" / "model" / (std::get<0>(actorRef) + ".szp");

					if(!std::get<2>(actorRef) && std::filesystem::exists(modelPath)){
						std::string actorName = std::get<0>(actorRef);
						std::string txpName = std::get<1>(actorRef);

						GCarchive modelArchive;
						if(!GCResourceManager.LoadArchive(modelPath.string().data(), &modelArchive)){
							std::cout << "Unable to load model archive " << modelPath.string() << std::endl;
							return;
						}

						if(mActorModels.count(name) == 0){
							GCarcfile* modelFile = GCResourceManager.GetFile(&modelArchive, std::filesystem::path("model") / (actorName + ".mdl"));

							if(modelFile == nullptr){
								std::cout << "Couldn't find model/" << actorName << ".mdl in archive" << std::endl;
							} else {
								bStream::CMemoryStream modelData((uint8_t*)modelFile->data, modelFile->size, bStream::Endianess::Big, bStream::OpenMode::In);
								mActorModels[name] = std::make_unique<MDL::Model>();
								mActorModels[name]->Load(&modelData);
							}
						}

						if(mMaterialAnimations.count(name) == 0 && txpName != ""){
							GCarcfile* txpFile = GCResourceManager.GetFile(&modelArchive, std::filesystem::path("txp") / (txpName + ".txp"));
							if(txpFile == nullptr){
								std::cout << "Couldn't find txp/" << txpName << ".txp in archive" << std::endl;
							} else {
								std::cout << "Loading txp " << txpName << std::endl;
								bStream::CMemoryStream txpData((uint8_t*)txpFile->data, txpFile->size, bStream::Endianess::Big, bStream::OpenMode::In);
								mMaterialAnimations[name] = std::make_unique<TXP::Animation>();
								mMaterialAnimations[name]->Load(&txpData);
							}
						}
					} else {
						// look in the game archive
						//todo: load archive from memstream
						GCarcfile* gameArchiveModelFile = GCResourceManager.GetFile(&GCResourceManager.mGameArchive, std::filesystem::path("model") / (std::get<0>(actorRef) + ".arc"));
						
						if(gameArchiveModelFile != nullptr){
							GCarchive modelArchive;
							gcInitArchive(&modelArchive, gameArchiveModelFile->ctx);
							
							if(gcLoadArchive(&modelArchive, gameArchiveModelFile->data, gameArchiveModelFile->size) == GC_ERROR_SUCCESS){
								GCarcfile* modelFile = GCResourceManager.GetFile(&modelArchive, std::filesystem::path("model") / (std::get<0>(actorRef) + ".mdl"));

								if(modelFile == nullptr){
									std::cout << "Couldn't find " << std::get<0>(actorRef) << ".mdl in game archive" << std::endl;
								} else {
									std::cout << "loading model from game archive..." << std::endl;
									bStream::CMemoryStream modelData((uint8_t*)modelFile->data, modelFile->size, bStream::Endianess::Big, bStream::OpenMode::In);
									mActorModels[name] = std::make_unique<MDL::Model>();
									mActorModels[name]->Load(&modelData);
								}
								gcFreeArchive(&modelArchive);
							}
						} else {
							std::cout << "Couldn't find model/" << std::get<0>(actorRef) << ".arc in game archive" << std::endl;
						}
					}
				}
			});

			std::shared_ptr<LRoomDataDOMNode> curRoomData = adjacentRoom->GetChildrenOfType<LRoomDataDOMNode>(EDOMNodeType::RoomData).front();

			std::filesystem::path resPath = std::filesystem::path(OPTIONS.mRootPath) / "files" / std::filesystem::path(curRoomData->GetResourcePath()).relative_path();

			if(!std::filesystem::exists(resPath)) continue;
			
			if(resPath.extension() == ".arc")
			{
				GCarchive roomArc;
				if(!GCResourceManager.LoadArchive(resPath.string().data(), &roomArc)){
					std::cout << "Unable to load room archive " << resPath << std::endl;
					continue;
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
				gcFreeArchive(&roomArc);
			} else {
				//If this is happening the map only has room models, no furniture.
				bStream::CFileStream bin(resPath.string(), bStream::Endianess::Big, bStream::OpenMode::In);
				mRoomModels.push_back(std::make_shared<BinModel>(&bin));
			}
		}

	}

	UpdateRenderers();
}

void LEditorScene::update(GLFWwindow* window, float dt, LEditorSelection* selection)
{
	if(Camera.mCamMode == ECamMode::ORBIT){
		//oh god no
		Camera.SetCenter(mCurrentRooms[0].lock().get()->GetChildrenOfType<LRoomDataDOMNode>(EDOMNodeType::RoomData)[0].get()->GetPosition());
	}

	Camera.Update(window, dt);

	// Easter egg where luigi occasionally blinks
	if(mActorModels.count("luige") > 0){
		if(mMaterialAnimations["luige"]->GetFrame() < mMaterialAnimations["luige"]->GetFrameCount()-1){
			mMaterialAnimations["luige"]->Update(dt);
		} else {
			if(rand() % 5000 == 1) mMaterialAnimations["luige"]->SetFrame(0);
		}
	}

}