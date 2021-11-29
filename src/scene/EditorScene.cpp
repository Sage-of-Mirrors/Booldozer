#include "scene/EditorScene.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <fstream>
#include <iostream>
#include <filesystem>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "cube_shader_f.h"
#include "cube_shader_v.h"
#include "cube_tex.h"
#include "border.h"
#include "ObserverIcon.hpp"
#include "ImGuizmo.h"
#include "Options.hpp"
#include "modes/EditorSelection.hpp"

struct Vertex
{
	float x;
	float y;
	float z;
	int16_t u;
	int16_t v;
	static void init()
	{
		ms_layout
			.begin()
			.add( bgfx::Attrib::Position, 3, bgfx::AttribType::Float )
			.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Int16, true, true)
			.end();
	}
	static bgfx::VertexLayout ms_layout;
};

bgfx::VertexLayout Vertex::ms_layout;

static Vertex s_cubeVertices[] =
{
	{-1.0f,  1.0f,  1.0f,      0,      0 },
	{ 1.0f,  1.0f,  1.0f, 0x7fff,      0 },
	{-1.0f, -1.0f,  1.0f,      0, 0x7fff },
	{ 1.0f, -1.0f,  1.0f, 0x7fff, 0x7fff },
	{-1.0f,  1.0f, -1.0f,      0,      0 },
	{ 1.0f,  1.0f, -1.0f, 0x7fff,      0 },
	{-1.0f, -1.0f, -1.0f,      0, 0x7fff },
	{ 1.0f, -1.0f, -1.0f, 0x7fff, 0x7fff },
	{-1.0f,  1.0f,  1.0f,      0,      0 },
	{ 1.0f,  1.0f,  1.0f, 0x7fff,      0 },
	{-1.0f,  1.0f, -1.0f,      0, 0x7fff },
	{ 1.0f,  1.0f, -1.0f, 0x7fff, 0x7fff },
	{-1.0f, -1.0f,  1.0f,      0,      0 },
	{ 1.0f, -1.0f,  1.0f, 0x7fff,      0 },
	{-1.0f, -1.0f, -1.0f,      0, 0x7fff },
	{ 1.0f, -1.0f, -1.0f, 0x7fff, 0x7fff },
	{ 1.0f, -1.0f,  1.0f,      0,      0 },
	{ 1.0f,  1.0f,  1.0f, 0x7fff,      0 },
	{ 1.0f, -1.0f, -1.0f,      0, 0x7fff },
	{ 1.0f,  1.0f, -1.0f, 0x7fff, 0x7fff },
	{-1.0f, -1.0f,  1.0f,      0,      0 },
	{-1.0f,  1.0f,  1.0f, 0x7fff,      0 },
	{-1.0f, -1.0f, -1.0f,      0, 0x7fff },
	{-1.0f,  1.0f, -1.0f, 0x7fff, 0x7fff },
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
	bgfx::setVertexBuffer(0, mCubeVbh);
	bgfx::setIndexBuffer(mCubeIbh);

	bgfx::setTexture(0, mCubeTexUniform, mCubeTexture);

	uint64_t  _state = 0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_MSAA;
	bgfx::setState( _state );

	glm::mat4 t = glm::scale(*transform, glm::vec3(25,25,25));
	bgfx::setTransform(&t);
	bgfx::submit(0, mCubeShader, 0);
}

void LCubeManager::renderAltTex(glm::mat4* transform, bgfx::TextureHandle& tex){
	bgfx::setVertexBuffer(0, mCubeVbh);
	bgfx::setIndexBuffer(mCubeIbh);

	bgfx::setTexture(0, mCubeTexUniform, tex);

	uint64_t  _state = 0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_MSAA | BGFX_STATE_BLEND_ALPHA;
	bgfx::setState( _state );

	bgfx::setTransform(transform);
	bgfx::submit(0, mCubeShader, 0);
}

LCubeManager::LCubeManager(){}

void LCubeManager::init(){
	/*
		TODO: System Agnostic Paths. Embed cube shaders using header output option of shaderc? 
	*/
    Vertex::init();

	bgfx::ShaderHandle vs = bgfx::createShader(bgfx::makeRef(cube_shader_v, cube_shader_v_len));
	bgfx::ShaderHandle fs = bgfx::createShader(bgfx::makeRef(cube_shader_f, cube_shader_f_len));
	mCubeShader = bgfx::createProgram(vs, fs, true);

	//mCubeShader = bigg::loadProgram("shaders/cube_shader_v.bin", "shaders/cube_shader_f.bin");

	mCubeVbh = bgfx::createVertexBuffer(bgfx::makeRef(s_cubeVertices, sizeof(s_cubeVertices)), Vertex::ms_layout );
	mCubeIbh = bgfx::createIndexBuffer(bgfx::makeRef(s_cubeTriList, sizeof(s_cubeTriList)));
	
	int x, y, n;
	uint8_t* data = stbi_load_from_memory(&cube_png[0], cube_png_size, &x, &y, &n, 4);
	mCubeTexture = bgfx::createTexture2D((uint16_t)x, (uint16_t)y, false, 1, bgfx::TextureFormat::RGBA8, 0, bgfx::copy(data, x*y*4));
	stbi_image_free(data);

	mCubeTexUniform = bgfx::createUniform("s_texColor",  bgfx::UniformType::Sampler);
}

LCubeManager::~LCubeManager(){} //Should destroy bgfx things here, but it segfault?


LEditorScene::LEditorScene() : Initialized(false) {}

LEditorScene::~LEditorScene(){}

void LEditorScene::init(){
	Initialized = true;
	mCubeManager.init();
	
	bgfx::ShaderHandle vs = bgfx::createShader(bgfx::makeRef(cube_shader_v, cube_shader_v_len));
	bgfx::ShaderHandle fs = bgfx::createShader(bgfx::makeRef(cube_shader_f, cube_shader_f_len));
	mShader = bgfx::createProgram(vs, fs, true);
	mTexUniform = bgfx::createUniform("s_texColor",  bgfx::UniformType::Sampler);
	
	int x, y, n;
	uint8_t* data = stbi_load_from_memory(&outline_png[0], outline_png_size, &x, &y, &n, 4);
	mBorderTex = bgfx::createTexture2D((uint16_t)x, (uint16_t)y, false, 1, bgfx::TextureFormat::RGBA8, 0, bgfx::copy(data, x*y*4));
	stbi_image_free(data);

	data = stbi_load_from_memory(&Icon_Observer_png[0], Icon_Observer_png_size, &x, &y, &n, 4);
	mObserverTex = bgfx::createTexture2D((uint16_t)x, (uint16_t)y, false, 1, bgfx::TextureFormat::RGBA8, 0, bgfx::copy(data, x*y*4));
	stbi_image_free(data);

	BGFXBin::InitBinVertex();

	mDoorModels.reserve(14);
	for (size_t f = 0; f < GCResourceManager.mGameArchive.dirnum; f++)
	{
		if(std::string(GCResourceManager.mGameArchive.dirs[f].name) == "door"){
			for (size_t i = GCResourceManager.mGameArchive.dirs[f].fileoff; i < GCResourceManager.mGameArchive.dirs[f].fileoff + GCResourceManager.mGameArchive.dirs[f].filenum; i++)
			{
				bStream::CMemoryStream bin((uint8_t*)GCResourceManager.mGameArchive.files[i].data, GCResourceManager.mGameArchive.files[i].size, bStream::Endianess::Big, bStream::OpenMode::In);
				if(std::filesystem::path(GCResourceManager.mGameArchive.files[i].name).extension() == ".bin"){
					auto doorModel = std::make_shared<BGFXBin>(&bin);
					//Haha holy shit the door models are so fucking broken.
					if(std::filesystem::path(GCResourceManager.mGameArchive.files[i].name).filename().stem() == "door_01"){
						//doorModel->TranslateRoot(glm::vec3(200,0,0));
					} else {
						//doorModel->TranslateRoot(glm::vec3(0,-150,0));
					}
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

	bgfx::setViewTransform(0, &view[0][0], &proj[0][0]);
    bgfx::setViewRect(0, 0, 0, uint16_t(m_width), uint16_t(m_height));
    bgfx::touch(0);

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
				mDoorModels[(uint8_t)doorType - 1]->Draw(&doorMat, mShader, mTexUniform);

				// Now offset the second door (left/backward) and rotate it 180 degrees.
				doubleDoorOffset *= 2;
				doorMat = glm::translate(doorMat, -doubleDoorOffset);
				doorMat = glm::rotate(doorMat, glm::radians(180.f), glm::vec3(0, 1, 0));

				// Render second door.
				mDoorModels[(uint8_t)doorType - 1]->Draw(&doorMat, mShader, mTexUniform);
			}
			// Single door can just be rendered without hassle.
			else
			{
				mDoorModels[(uint8_t)doorType - 1]->Draw(&doorMat, mShader, mTexUniform, bIgnoreTransforms);
			}
		}
	}

	for(auto room : mCurrentRooms){
		glm::mat4 identity = glm::identity<glm::mat4>();
		for (auto room : mRoomModels)
		{
			room->Draw(&identity, mShader, mTexUniform);
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
							mRoomFurniture[node->GetName()]->Draw(node->GetMat(), mShader, mTexUniform);
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
	for(auto& bb : roomBounds){
		mCubeManager.renderAltTex(&bb, mBorderTex);
	}
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
							mRoomFurniture[curPath.filename().stem().string()] = std::make_shared<BGFXBin>(&bin);
							std::cout << "completed loading " << curPath.filename() << std::endl;
						} else {
							mRoomModels.push_back(std::make_shared<BGFXBin>(&bin));
							std::cout << "completed loading room model" << std::endl;
						}
						
					}
				}
				std::cout << "all models locked and loaded" << std::endl;
			} else {
				//If this is happening the map only has room models, no furniture.
				bStream::CFileStream bin(resPath.u8string(), bStream::Endianess::Big, bStream::OpenMode::In);
				mRoomModels.push_back(std::make_shared<BGFXBin>(&bin));
			}
		}
	}
}

void LEditorScene::update(GLFWwindow* window, float dt, LEditorSelection* selection)
{
	Camera.Update(window, dt);

	int w, h;
	int vx, vy;
	double x, y;
	glfwGetCursorPos(window, &x, &y);

	glfwGetWindowSize(window, &w, &h);
	glfwGetWindowPos(window, &vx, &vy);

	if(Camera.GetClicked()){
		selection->ClearSelection();
		auto ray = Camera.Raycast(x, y, glm::vec4(0,0,w,h));
		for(auto room : mCurrentRooms){			
			if(!room.expired() && Initialized)
			{
				auto curRoom = room.lock();

				curRoom->ForEachChildOfType<LBGRenderDOMNode>(EDOMNodeType::BGRender, [&](auto node){
					auto check = ray.first + (ray.second * glm::distance(node->GetPosition(), ray.first));

					
					if(glm::distance(node->GetPosition(), check) < 150.0f){
						std::cout << "clicked on " << node->GetName() << std::endl;
						if(selection != nullptr){
							selection->AddToSelection(node);
						}
					}
				});

			}
		}
	}
}
