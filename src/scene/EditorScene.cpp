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
#include "DOM/MapCollisionDOMNode.hpp"
#include "DOM/MirrorDOMNode.hpp"

#include <J3D/Material/J3DUniformBufferObject.hpp>
#include <J3D/J3DModelLoader.hpp>
#include <J3D/Rendering/J3DRendering.hpp>

// This kind of sucks but the application code doesnt currently have a way to get EditorScene
namespace {
	bool mIsDirty { false };
	LEditorScene* mScene;
}

LEditorScene* LEditorScene::GetEditorScene(){
	return mScene;
}

LEditorScene::LEditorScene() : Initialized(false) {}

LEditorScene::~LEditorScene(){
	BIN::DestroyShaders();
	MDL::DestroyShaders();
}

void LEditorScene::LoadResFromRoot(){
	mDoorModels.reserve(14);

	for(int door_id = 0; door_id < 15; door_id++){
		if(GCResourceManager.mLoadedGameArchive == false) continue; // should be done better
		std::shared_ptr<Archive::File> doorModelFile = GCResourceManager.mGameArchive->GetFile(std::filesystem::path(std::format("iwamoto/door/door_{:02}.bin", door_id)));
		if(doorModelFile != nullptr){
			bStream::CMemoryStream bin_data(doorModelFile->GetData(), doorModelFile->GetSize(), bStream::Endianess::Big, bStream::OpenMode::In);

			auto doorModel = std::make_shared<BIN::Model>(&bin_data);
			mDoorModels.push_back(doorModel);
		}
	}

	J3DModelLoader Loader;

	if((std::filesystem::exists(std::filesystem::path(OPTIONS.mRootPath) / "files" / "Iwamoto" / "vrball_M.szp"))){

		std::shared_ptr<Archive::Rarc> skyboxArchive = Archive::Rarc::Create();

		bStream::CFileStream skyboxArchiveFile((std::filesystem::path(OPTIONS.mRootPath) / "files" / "Iwamoto" / "vrball_M.szp").string(), bStream::Endianess::Big, bStream::OpenMode::In);

		if(!skyboxArchive->Load(&skyboxArchiveFile)){
			LGenUtility::Log << "[EditorScene] Failed to load SkyBox Archive" << std::endl;
			return;
		}

		std::shared_ptr<Archive::File> skyboxModelFile = skyboxArchive->GetFile("vrball01.bmd");

		if(skyboxModelFile != nullptr){
			LGenUtility::Log << "[EditorScene] Loaded SkyBox Model" << std::endl;
			bStream::CMemoryStream modelData(skyboxModelFile->GetData(), skyboxModelFile->GetSize(), bStream::Endianess::Big, bStream::OpenMode::In);
			mSkyboxModel = Loader.Load(&modelData, 0);
			mSkyBox = mSkyboxModel->CreateInstance();
		} else {
			LGenUtility::Log << "[EditorScene] Failed to load SkyBox Model" << std::endl;
		}
	} else {
		LGenUtility::Log << "[EditorScene] Couldn't find Skybox archive " << (std::filesystem::path(OPTIONS.mRootPath) / "files" / "Iwamoto" / "vrball_M.szp").string() << std::endl;
	}
}

void LEditorScene::Init(){
	Initialized = true;

	J3DUniformBufferObject::CreateUBO();

	mPathRenderer.Init();
	mPointManager.Init(512, 9);
	mMirrorRenderer.Init((RES_BASE_PATH / "img" / "mirror.png").string());

	mPointManager.SetBillboardTexture(RES_BASE_PATH / "img" / "ice_generator.png", 0);
	mPointManager.SetBillboardTexture(RES_BASE_PATH / "img" / "fire_generator.png", 1);
	mPointManager.SetBillboardTexture(RES_BASE_PATH / "img" / "water_generator.png", 2);

	mPointManager.SetBillboardTexture(RES_BASE_PATH / "img" / "event.png", 3);
	mPointManager.SetBillboardTexture(RES_BASE_PATH / "img" / "observer.png", 4);
	//mPointManager.SetBillboardTexture(RES_BASE_PATH / "img" / "enemy_placeholder.png", 5);

	mPointManager.SetBillboardTexture(RES_BASE_PATH / "img" / "soundobj.png", 6);

	BIN::InitShaders();
	MDL::InitShaders();

	mScene = this;
}

void LEditorScene::Clear(){
	mRoomDoors.clear();
	mCurrentRooms.clear();
	mPathRenderer.mPaths.clear();
	mPointManager.mBillboards.clear();
}

glm::mat4 LEditorScene::getCameraView(){
	return Camera.GetViewMatrix();
}

glm::mat4 LEditorScene::getCameraProj(){
	return Camera.GetProjectionMatrix();
}

void LEditorScene::UpdateRenderers(){
	//LGenUtility::Log << "calling update renderers. this should only happen a few times!" << std::endl;
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
							int32_t pickID = curRoom->GetID();
							glm::vec4 color = (curRoom->GetRoomNumber() == mSelectedRoomNumber ? glm::vec4(0.0f, 1.0f, 0.0f, 1.0f) : glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

							glm::vec3 min = static_cast<LRoomDataDOMNode*>(node.get())->GetMin();
							glm::vec3 max = static_cast<LRoomDataDOMNode*>(node.get())->GetMax();

							std::vector<CPathPoint> bounds_bottom = {
								{min, color, 51200, pickID},
								{{min.x, min.y, max.z}, color, 51200, pickID},
								{{max.x, min.y, max.z}, color, 51200, pickID},
								{{max.x, min.y, min.z}, color, 51200, pickID},
								{min, color, 51200, pickID},
							};

							std::vector<CPathPoint> bounds_top = {
								{max, color, 51200, pickID},
								{{max.x, max.y, min.z}, color, 51200, pickID},
								{{min.x, max.y, min.z}, color, 51200, pickID},
								{{min.x, max.y, max.z}, color, 51200, pickID},
								{max, color, 51200, pickID},
							};

							std::vector<CPathPoint> bounds_edge1 = {{min, color, 51200, pickID}, {{min.x, max.y, min.z}, color, 51200, pickID}};
							std::vector<CPathPoint> bounds_edge2 = {{{max.x, min.y, min.z}, color, 51200, pickID}, {{max.x, max.y, min.z}, color, 51200, pickID}};
							std::vector<CPathPoint> bounds_edge3 = {{{min.x, min.y, max.z}, color, 51200, pickID}, {{min.x, max.y, max.z}, color, 51200, pickID}};
							std::vector<CPathPoint> bounds_edge4 = {{max, color, 51200, pickID}, {{max.x, min.y, max.z}, color, 51200, pickID}};

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
								path.push_back({point->GetPosition(), static_cast<LPathDOMNode*>(node.get())->mPathColor, 12800, point->GetID()});
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

	if(!mCurrentRooms[0].expired()){
		std::shared_ptr<LRoomDOMNode> firstRoom = mCurrentRooms[0].lock();
		std::shared_ptr<LMapDOMNode> mapNode = firstRoom->GetParentOfType<LMapDOMNode>(EDOMNodeType::Map).lock();
		std::shared_ptr<LMapCollisionDOMNode> col = mapNode->GetChildrenOfType<LMapCollisionDOMNode>(EDOMNodeType::MapCollision)[0];
		if(col->GetIsRendered()){
		// More Stupid Code. Why?

			for(auto triangle : col->mModel.mTriangles){
				std::vector<CPathPoint> renderTri = {
					{ col->mModel.mVertices[triangle.mVtx1], {0.15, 0.675, 0.8, 1.0}, 0, -1 },
					{ col->mModel.mVertices[triangle.mVtx1] + (col->mModel.mNormals[triangle.mNormal] * 9.0f), {0.0, 0.775, 0.32, 1.0}, 800, -1 },
					{ col->mModel.mVertices[triangle.mVtx1], {0.15, 0.675, 0.8, 1.0}, 0, -1 },
					{ col->mModel.mVertices[triangle.mVtx2], {0.15, 0.675, 0.8, 1.0}, 0, -1 },
					{ col->mModel.mVertices[triangle.mVtx2] + (col->mModel.mNormals[triangle.mNormal] * 9.0f), {0.0, 0.775, 0.32, 1.0}, 800, -1 },
					{ col->mModel.mVertices[triangle.mVtx2], {0.15, 0.675, 0.8, 1.0}, 0, -1 },
					{ col->mModel.mVertices[triangle.mVtx3], {0.15, 0.675, 0.8, 1.0}, 0, -1 },
					{ col->mModel.mVertices[triangle.mVtx3] + (col->mModel.mNormals[triangle.mNormal] * 9.0f), {0.0, 0.775, 0.32, 1.0}, 800, -1 },
					{ col->mModel.mVertices[triangle.mVtx3], {0.15, 0.675, 0.8, 1.0}, 0, -1 },
					{ col->mModel.mVertices[triangle.mVtx1], {0.15, 0.675, 0.8, 1.0}, 0, -1 },
				};
				mPathRenderer.mPaths.push_back(renderTri);
			}
		}
	}


	mPointManager.UpdateData();
	mPathRenderer.UpdateData();
}


void LEditorScene::SetDirty(){
	mIsDirty = true;
}

// This whole thing is so so SO awful.
void LEditorScene::RenderSubmit(uint32_t m_width, uint32_t m_height){

	if(mIsDirty){
		UpdateRenderers();
		mIsDirty = false;
	}

	if (m_height == 0)
		m_height = 1;

	Camera.AspectRatio = float(m_width) / float(m_height);

	glm::mat4 view = Camera.GetViewMatrix();
	glm::mat4 proj = Camera.GetProjectionMatrix();

	J3DUniformBufferObject::SetProjAndViewMatrices(proj, view);
	J3DUniformBufferObject::SubmitUBO();

	std::vector<std::shared_ptr<J3DModelInstance>> renderables;
	renderables.reserve(50);

	for(std::weak_ptr<LRoomDOMNode> room : mCurrentRooms){
		if(!room.expired() && Initialized)
		{
			std::shared_ptr<LRoomDOMNode> roomLocked = room.lock();
			//vrball01.bmd
			if(roomLocked->GetSkyboxEnabled() && mSkyBox != nullptr){
				mSkyBox->SetTranslation(roomLocked->GetPosition());
				mSkyBox->SetRotation({0,0,0});
				mSkyBox->SetScale({10,10,10});
				renderables.push_back(mSkyBox);
				break;
			}
		}
	}

	glDisable(GL_CULL_FACE);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	for(std::weak_ptr<LDoorDOMNode> doorRef : mRoomDoors){
		std::shared_ptr<LDoorDOMNode> door = doorRef.lock();
		if (!doorRef.expired())
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
				doorMat = glm::translate(doorMat, glm::vec3(0, 0, 100));

			// Double doors need to be rendered twice, with the two halves moved accordingly.
			if (doorType == EDoorModel::Parlor_Double_Door || doorType == EDoorModel::Hearts_Double_Door)
			{
				glm::vec3 doubleDoorOffset = glm::vec3(0, 0, 100);

				// Offset the first door (right/forward) and render it.
				doorMat = glm::translate(doorMat, doubleDoorOffset);
				mDoorModels[(uint8_t)doorType - 1]->Draw(&doorMat, door->GetID(), door->GetIsSelected());

				// Now offset the second door (left/backward) and rotate it 180 degrees.
				doubleDoorOffset *= 2;
				doorMat = glm::translate(doorMat, -doubleDoorOffset);
				doorMat = glm::rotate(doorMat, glm::radians(180.f), glm::vec3(0, 1, 0));

				// Render second door.
				mDoorModels[(uint8_t)doorType - 1]->Draw(&doorMat, door->GetID(), door->GetIsSelected());
			}
			// Single door can just be rendered without hassle.
			else
			{
				mDoorModels[(uint8_t)doorType - 1]->Draw(&doorMat, door->GetID(), door->GetIsSelected());
			}
		}
	}

	for(std::weak_ptr<LRoomDOMNode> roomRef : mCurrentRooms){

		//for (std::shared_ptr<BinModel> room : mRoomModels)
		//{
		//	room->Draw(&identity, -1, false);
		//}

		std::shared_ptr<LRoomDOMNode> curRoom;

		if((curRoom = roomRef.lock()) && Initialized)
		{
			// this sucks
			auto roomData = curRoom->GetChildrenOfType<LRoomDataDOMNode>(EDOMNodeType::RoomData).front();

			if(mRoomModels.contains(roomData->GetResourcePath())){
				glm::mat4 identity = glm::identity<glm::mat4>();
				identity = glm::translate(identity, curRoom->GetRoomModelDelta());
				mRoomModels.at(roomData->GetResourcePath())->Draw(&identity, curRoom->GetID(), false);
			}

			// is okay
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
							mRoomFurniture[std::static_pointer_cast<LFurnitureDOMNode>(node)->GetModelName()]->Draw(node->GetMat(), node->GetID(), node->GetIsSelected());
						}
					case EDOMNodeType::Object:
						if(node->GetName() == "coin"){
							//auto coin = mCoinModel->CreateInstance();
							//coin->SetTransform(*node->GetMat());
							//renderables.push_back(coin);
						}
						break;
					case EDOMNodeType::Mirror:
						mMirrorRenderer.Draw(node->GetMat(), node->GetID(), node->GetIsSelected(), std::dynamic_pointer_cast<LMirrorDOMNode>(node)->GetResWidth(), std::dynamic_pointer_cast<LMirrorDOMNode>(node)->GetResHeight());
						break;
					case EDOMNodeType::Character:
					case EDOMNodeType::Enemy:
					case EDOMNodeType::Observer:
					case EDOMNodeType::Generator:
					case EDOMNodeType::Key:
						if(mActorModels.contains(node->GetName())){
							if(mMaterialAnimations.contains(node->GetName())){
								mActorModels[node->GetName()]->Draw(node->GetMat(), node->GetID(), node->GetIsSelected(), mMaterialAnimations[node->GetName()].get());
							} else {
								mActorModels[node->GetName()]->Draw(node->GetMat(), node->GetID(), node->GetIsSelected(), nullptr);
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


	auto packets = J3D::Rendering::SortPackets(renderables, Camera.GetCenter());
	J3D::Rendering::Render(1, view, proj, packets);
}

void LEditorScene::LoadActor(std::string name, bool log){
	std::tuple<std::string, std::string, bool> actorRef = LResUtility::GetActorModelFromName(name, log);

	if(mActorModels.count(name) != 0 && (mMaterialAnimations.count(name) != 0 && std::get<1>(actorRef) != "")) return;

	std::filesystem::path modelPath = std::filesystem::path(OPTIONS.mRootPath) / "files" / "model" / (std::get<0>(actorRef) + ".szp");

	if(!std::get<2>(actorRef) && std::filesystem::exists(modelPath)){
		std::string actorName = std::get<0>(actorRef);
		std::string txpName = std::get<1>(actorRef);
		std::shared_ptr<Archive::Rarc> modelArchive = Archive::Rarc::Create();
		bStream::CFileStream modelArchiveStream(modelPath.string(), bStream::Endianess::Big, bStream::OpenMode::In);
		if(!modelArchive->Load(&modelArchiveStream)){
			LGenUtility::Log << "[Editor Scene]: Unable to load model archive " << modelPath.string() << std::endl;
			return;
		}
		if(mActorModels.count(name) == 0){
			std::shared_ptr<Archive::File> modelFile = modelArchive->GetFile(std::filesystem::path("model") / (actorName + ".mdl"));
			if(modelFile == nullptr){
				LGenUtility::Log << "[Editor Scene]: Couldn't find model/" << actorName << ".mdl in archive" << std::endl;
			} else {
				bStream::CMemoryStream modelData(modelFile->GetData(), modelFile->GetSize(), bStream::Endianess::Big, bStream::OpenMode::In);
				mActorModels[name] = std::make_unique<MDL::Model>();
				mActorModels[name]->Load(&modelData);
			}
		}
		if(mMaterialAnimations.count(name) == 0 && txpName != ""){

			std::shared_ptr<Archive::File> txpFile = modelArchive->GetFile(std::filesystem::path("txp") / (txpName + ".txp"));
			if(txpFile == nullptr){
				LGenUtility::Log << "[Editor Scene]: Couldn't find txp/" << txpName << ".txp in archive" << std::endl;
			} else {
				LGenUtility::Log << "[Editor Scene]: Loading txp " << txpName << std::endl;
				bStream::CMemoryStream txpData(txpFile->GetData(), txpFile->GetSize(), bStream::Endianess::Big, bStream::OpenMode::In);
				mMaterialAnimations[name] = std::make_unique<TXP::Animation>();
				mMaterialAnimations[name]->Load(&txpData);
			}
		}
	} else {
		std::filesystem::path fullModelPath = std::filesystem::path("model") / (std::get<0>(actorRef) + ".arc") / "model" / (std::get<0>(actorRef) + ".mdl");

		if(GCResourceManager.mLoadedGameArchive){
			std::shared_ptr<Archive::File> modelFile = GCResourceManager.mGameArchive->GetFile(fullModelPath);

			if(modelFile == nullptr){
				if(log) LGenUtility::Log << "[Editor Scene]: Couldn't find " << std::get<0>(actorRef) << ".mdl in game archive" << std::endl;
			} else {
				bStream::CMemoryStream modelData(modelFile->GetData(), modelFile->GetSize(), bStream::Endianess::Big, bStream::OpenMode::In);
				mActorModels[name] = std::make_unique<MDL::Model>();
				mActorModels[name]->Load(&modelData);
			}
		}
	}
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

	mCurrentRooms.clear();

	//This is ensured to exist, but check it anyway
	if(roomData.size() != 0)
	{
		mCurrentRooms.push_back(room);
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
					LoadActor(node->GetName());
				}
			});

			std::shared_ptr<LRoomDataDOMNode> curRoomData = adjacentRoom->GetChildrenOfType<LRoomDataDOMNode>(EDOMNodeType::RoomData).front();

			std::filesystem::path resPath = std::filesystem::path(OPTIONS.mRootPath) / "files" / std::filesystem::path(curRoomData->GetResourcePath()).relative_path();

			if(!std::filesystem::exists(resPath)) continue;

			if(resPath.extension() == ".arc")
			{
				std::shared_ptr<Archive::Rarc> roomArc = Archive::Rarc::Create();
				bStream::CFileStream roomArchiveStream(resPath.string(), bStream::Endianess::Big, bStream::OpenMode::In);
				if(!roomArc->Load(&roomArchiveStream)){
					LGenUtility::Log << "[Editor Scene]: Unable to load room archive " << resPath << std::endl;
					continue;
				}

				for(auto file : roomArc->GetRoot()->GetFiles()){
					bStream::CMemoryStream bin(file->GetData(), file->GetSize(), bStream::Endianess::Big, bStream::OpenMode::In);

					std::string modelName = file->GetName();
					auto modelNameExtIter = modelName.find(".bin");
					if(modelNameExtIter == std::string::npos) continue;

					if (modelName != "room.bin")
					{
						mRoomFurniture[modelName.substr(0, modelNameExtIter)] = std::make_shared<BIN::Model>(&bin);
						LGenUtility::Log << "[Editor Scene]: completed loading " << modelName.substr(0, modelNameExtIter) << std::endl;
					} else {
						mRoomModels.insert({curRoomData->GetResourcePath(), std::make_shared<BIN::Model>(&bin)});
						LGenUtility::Log << "[Editor Scene]: completed loading room model" << std::endl;
					}
				}
			} else {
				//If this is happening the map only has room models, no furniture.
				bStream::CFileStream bin(resPath.string(), bStream::Endianess::Big, bStream::OpenMode::In);
				#ifdef _WIN32
				std::string resPath = curRoomData->GetResourcePath();
				std::replace(resPath.begin(), resPath.end(), '/', '\\');
				mRoomModels.insert({ resPath, std::make_shared<BIN::Model>(&bin)});
				#else
				mRoomModels.insert({ curRoomData->GetResourcePath(), std::make_shared<BIN::Model>(&bin)});
				#endif
			}
		}

	}

	UpdateRenderers();
}

void LEditorScene::Update(GLFWwindow* window, float dt, LEditorSelection* selection)
{
	if(mActive){
		Camera.Update(window, dt);
	}
	// Easter egg where luigi occasionally blinks
}
