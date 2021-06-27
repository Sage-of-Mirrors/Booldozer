#include "EditorScene.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <fstream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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

LSceneModel::LSceneModel(){ mNextID = 0; }
LSceneModel::~LSceneModel(){}

size_t LSceneModel::addInstance(glm::mat4 transform){
	mInstanceData.insert(std::pair(mNextID, transform));
	return mNextID++;
}

void LSceneModel::setTransform(size_t id, glm::mat4 transform){
	mInstanceData[id] = transform;
}

LCubeManager::LCubeManager(){}

void LCubeManager::init(){
	/*
		TODO: System Agnostic Paths. Embed cube shaders using header output option of shaderc? 
	*/
    Vertex::init();

	mCubeShader = bigg::loadProgram("shaders/cube_shader_v.bin", "shaders/cube_shader_f.bin");

	mCubeVbh = bgfx::createVertexBuffer(bgfx::makeRef(s_cubeVertices, sizeof(s_cubeVertices)), Vertex::ms_layout );
	mCubeIbh = bgfx::createIndexBuffer(bgfx::makeRef(s_cubeTriList, sizeof(s_cubeTriList)));
	
	int x, y, n;
	uint8_t* data = stbi_load("cube.png", &x, &y, &n, 4);
	
	mCubeTexture = bgfx::createTexture2D((uint16_t)x, (uint16_t)y, false, 1, bgfx::TextureFormat::RGBA8, 0, bgfx::copy(data, x*y*4));
	
	stbi_image_free(data);

	mCubeTexUniform = bgfx::createUniform("s_texColor",  bgfx::UniformType::Sampler);

	mNextId = 0;
}

void LCubeManager::updateInstanceBuffer(){
	if(mInstanceData.size() == 0) return;
	size_t test = sizeof(glm::mat4);
	uint32_t drawableCubes = bgfx::getAvailInstanceDataBuffer(mInstanceData.size(), mCubeInstanceStride);
	bgfx::allocInstanceDataBuffer(&mCubeInstances, drawableCubes, mCubeInstanceStride);

	uint8_t* instanceData = mCubeInstances.data;

	for (auto & [ key, value ] : mInstanceData) {
		memcpy(instanceData, &value[0][0], mCubeInstanceStride);

		instanceData += mCubeInstanceStride;
	}
}

size_t LCubeManager::addCube(glm::mat4 transform){
	size_t id = mNextId;
	mInstanceData.insert(std::pair(id, transform));
	mNextId++;
	return id;
}

void LCubeManager::setTransform(size_t id, glm::mat4 transform){
	mInstanceData[id] = transform;
}

void LCubeManager::removeCube(size_t id){
	mInstanceData.erase(id);
}

void LCubeManager::render(){
	if(mInstanceData.size() == 0 || mCubeInstances.data == nullptr) return;
	bgfx::setVertexBuffer(0, mCubeVbh);
	bgfx::setIndexBuffer(mCubeIbh);

	bgfx::setTexture(0, mCubeTexUniform, mCubeTexture);

	updateInstanceBuffer();
	bgfx::setInstanceDataBuffer(&mCubeInstances);	
	
	bgfx::setState( BGFX_STATE_DEFAULT );
	bgfx::submit(0, mCubeShader);
}

LCubeManager::~LCubeManager(){ 
	bgfx::destroy(mCubeShader);
	bgfx::destroy(mCubeVbh);
	bgfx::destroy(mCubeIbh);
	bgfx::destroy(mCubeTexture);
	bgfx::destroy(mCubeTexUniform);
}


LEditorScene::LEditorScene() : Initialized(false) {}

LEditorScene::~LEditorScene(){}

void LEditorScene::init(){
	Initialized = true;
	mCubeManager.init();
}

size_t LEditorScene::RegisterModel(std::string name, glm::mat4 transform){
	if (mSceneModels.count(name) == 0){
		
		//TODO: Try to load model here!

		return mCubeManager.addCube(transform);
	} else {
		return mSceneModels[name].addInstance(transform);
	}
}

void LEditorScene::UpdateModelPosition(std::string name, size_t id, glm::mat4 transform){
	if (mSceneModels.count(name) == 0){
		mCubeManager.setTransform(id, transform);
	} else {
		mSceneModels[name].setTransform(id, transform);
	}
}

void LEditorScene::RenderSubmit(uint32_t m_width, uint32_t m_height){
    
	glm::mat4 view = glm::lookAt(glm::vec3( 0.0f, 0.0f, -35.0f ), glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3( 0.0f, 1.0f, 0.0f ));
	glm::mat4 proj = glm::perspective(glm::radians( 60.0f ), float(m_width) / m_height, 0.1f, 100.0f);

	bgfx::setViewTransform(0, &view[0][0], &proj[0][0]);
    bgfx::setViewRect(0, 0, 0, uint16_t(m_width), uint16_t(m_height));
    bgfx::touch(0);
    
	mCubeManager.render();
}