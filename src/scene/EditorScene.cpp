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
	{-25.0f,  25.0f,  25.0f,      0,      0 },
	{ 25.0f,  25.0f,  25.0f, 0x7fff,      0 },
	{-25.0f, -25.0f,  25.0f,      0, 0x7fff },
	{ 25.0f, -25.0f,  25.0f, 0x7fff, 0x7fff },
	{-25.0f,  25.0f, -25.0f,      0,      0 },
	{ 25.0f,  25.0f, -25.0f, 0x7fff,      0 },
	{-25.0f, -25.0f, -25.0f,      0, 0x7fff },
	{ 25.0f, -25.0f, -25.0f, 0x7fff, 0x7fff },
	{-25.0f,  25.0f,  25.0f,      0,      0 },
	{ 25.0f,  25.0f,  25.0f, 0x7fff,      0 },
	{-25.0f,  25.0f, -25.0f,      0, 0x7fff },
	{ 25.0f,  25.0f, -25.0f, 0x7fff, 0x7fff },
	{-25.0f, -25.0f,  25.0f,      0,      0 },
	{ 25.0f, -25.0f,  25.0f, 0x7fff,      0 },
	{-25.0f, -25.0f, -25.0f,      0, 0x7fff },
	{ 25.0f, -25.0f, -25.0f, 0x7fff, 0x7fff },
	{ 25.0f, -25.0f,  25.0f,      0,      0 },
	{ 25.0f,  25.0f,  25.0f, 0x7fff,      0 },
	{ 25.0f, -25.0f, -25.0f,      0, 0x7fff },
	{ 25.0f,  25.0f, -25.0f, 0x7fff, 0x7fff },
	{-25.0f, -25.0f,  25.0f,      0,      0 },
	{-25.0f,  25.0f,  25.0f, 0x7fff,      0 },
	{-25.0f, -25.0f, -25.0f,      0, 0x7fff },
	{-25.0f,  25.0f, -25.0f, 0x7fff, 0x7fff },
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

	bgfx::setTransform(transform);
	bgfx::submit(0, mCubeShader, 0, BGFX_DISCARD_NONE);
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
	
	BGFXBin::InitBinVertex();

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

	if(mCurrentRoom.lock() != nullptr && Initialized)
	{
		auto furnitureNodes = mCurrentRoom.lock()->GetChildrenOfType<LFurnitureDOMNode>(EDOMNodeType::Furniture);
		for (auto furniture : furnitureNodes)
		{
			if(furniture->GetIsRendered())
			{
				if(mRoomFurniture.count(furniture->GetName()) != 0)
				{
					mRoomFurniture[furniture->GetName()]->Draw(furniture->GetMat(), mShader, mTexUniform);
				} else {
					mCubeManager.render(furniture->GetMat());
				}
			}

		}

		glm::mat4 identity = glm::identity<glm::mat4>();
		for (auto room : mRoomModels)
		{
			room->Draw(&identity, mShader, mTexUniform);
		}
		
	}
    
	//mCubeManager.render();
}

void LEditorScene::SetRoom(std::shared_ptr<LRoomDOMNode> room)
{
	mCurrentRoom = room;
	auto roomData = room->GetChildrenOfType<LRoomDataDOMNode>(EDOMNodeType::RoomData);

	if(roomData.size() != 0)
	{
		std::filesystem::path resPath = std::filesystem::path(OPTIONS.mRootPath) / "files" / std::filesystem::path(roomData.front()->GetResourcePath()).relative_path();
		
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
		}
	}
}

void LEditorScene::update(GLFWwindow* window, float dt)
{
	Camera.Update(window, dt);
}
