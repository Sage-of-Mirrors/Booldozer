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
						doorModel->TranslateRoot(glm::vec3(0,350,0));
					} else {
						doorModel->TranslateRoot(glm::vec3(0,-150,0));
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

	Camera.AspectRatio = m_width / m_height;

	glm::mat4 view = Camera.GetViewMatrix();
	glm::mat4 proj = Camera.GetProjectionMatrix();

	bgfx::setViewTransform(0, &view[0][0], &proj[0][0]);
    bgfx::setViewRect(0, 0, 0, uint16_t(m_width), uint16_t(m_height));
    bgfx::touch(0);

	std::vector<glm::mat4> roomBounds;

	for(auto door : mRoomDoors){
		if(!door.expired()){
			glm::mat4 identity = glm::identity<glm::mat4>();
			identity = glm::translate(identity, door.lock()->GetPosition());
			if(door.lock()->GetOrientation() == EDoorOrientation::Side_Facing) identity = glm::rotate(identity, glm::radians(90.0f), glm::vec3(0,1,0));
			if(door.lock()->GetModel() != EDoorModel::None){
				mDoorModels[((uint8_t)door.lock()->GetModel()) - 1]->Draw(&identity, mShader, mTexUniform);
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
					
					case EDOMNodeType::Observer:
						mCubeManager.renderAltTex(node->GetMat(), mObserverTex);
						break;

					case EDOMNodeType::RoomData:
						roomBounds.push_back(glm::scale(glm::translate(transform, node->GetPosition()), node->GetScale()));
						break;

					case EDOMNodeType::Character:
					case EDOMNodeType::Generator:
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
				bStream::CFileStream bin(resPath, bStream::Endianess::Big, bStream::OpenMode::In);
				mRoomModels.push_back(std::make_shared<BGFXBin>(&bin));
			}
		}
	}
}

void LEditorScene::update(GLFWwindow* window, float dt)
{
	Camera.Update(window, dt);
}
