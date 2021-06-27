#include "EditorScene.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <fstream>

struct PosColorVertex
{
	float x;
	float y;
	float z;
	uint32_t abgr;
	static void init()
	{
		ms_layout
			.begin()
			.add( bgfx::Attrib::Position, 3, bgfx::AttribType::Float )
			.add( bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true )
			.end();
	}
	static bgfx::VertexLayout ms_layout;
};
bgfx::VertexLayout PosColorVertex::ms_layout;

static PosColorVertex s_cubeVertices[] =
{
	{-1.0f,  1.0f,  1.0f, 0xff000000 },
	{ 1.0f,  1.0f,  1.0f, 0xff0000ff },
	{-1.0f, -1.0f,  1.0f, 0xff00ff00 },
	{ 1.0f, -1.0f,  1.0f, 0xff00ffff },
	{-1.0f,  1.0f, -1.0f, 0xffff0000 },
	{ 1.0f,  1.0f, -1.0f, 0xffff00ff },
	{-1.0f, -1.0f, -1.0f, 0xffffff00 },
	{ 1.0f, -1.0f, -1.0f, 0xffffffff },
};
static const uint16_t s_cubeTriList[] = { 2, 1, 0, 2, 3, 1, 5, 6, 4, 7, 6, 5, 4, 2, 0, 6, 2, 4, 3, 5, 1, 3, 7, 5, 1, 4, 0, 1, 5, 4, 6, 3, 2, 7, 3, 6 };

LEditorScene::LEditorScene() : Initialized(false) {}

LCubeManager::LCubeManager(){}

void LCubeManager::init(){
	/*
		TODO: System Agnostic Paths. Embed cube shaders using header output option of shaderc? 
	*/
    PosColorVertex::init();

	mCubeShader = bigg::loadProgram("shaders/cube_shader_v.bin", "shaders/cube_shader_f.bin");

	mCubeVbh = bgfx::createVertexBuffer(bgfx::makeRef(s_cubeVertices, sizeof(s_cubeVertices)), PosColorVertex::ms_layout );
	mCubeIbh = bgfx::createIndexBuffer(bgfx::makeRef(s_cubeTriList, sizeof(s_cubeTriList)));
	
	mNextId = 0;
}

void LCubeManager::updateInstanceBuffer(){
	if(mInstanceData.size() == 0) return;

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
	updateInstanceBuffer();
	mNextId++;
	return id;
}

void LCubeManager::setTransform(size_t id, glm::mat4 transform){
	mInstanceData[id] = transform;
	updateInstanceBuffer();
}

void LCubeManager::removeCube(size_t id){
	mInstanceData.erase(id);
	updateInstanceBuffer();
}

void LCubeManager::render(){
	if(mInstanceData.size() == 0 or mCubeInstances.data != nullptr) return;
	bgfx::setVertexBuffer(0, mCubeVbh);
	bgfx::setIndexBuffer(mCubeIbh);

	bgfx::setInstanceDataBuffer(&mCubeInstances);	
	
	bgfx::setState( BGFX_STATE_DEFAULT );
	bgfx::submit(0, mCubeShader);
}

LCubeManager::~LCubeManager(){ 
	bgfx::destroy(mCubeShader);
	bgfx::destroy(mCubeVbh);
	bgfx::destroy(mCubeIbh);
}

void LEditorScene::init(){

	Initialized = true;
	mCubeManager.init();
	glm::mat4 cube1 = glm::identity<glm::mat4>();
	printf("%d", mCubeManager.addCube(cube1));
}

LEditorScene::~LEditorScene(){

}

void LEditorScene::RenderSubmit(uint32_t m_width, uint32_t m_height){
    
	glm::mat4 view = glm::lookAt( glm::vec3( 0.0f, 0.0f, -35.0f ), glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3( 0.0f, 1.0f, 0.0f ) );
	glm::mat4 proj = glm::perspective( glm::radians( 60.0f ), float(m_width) / m_height, 0.1f, 100.0f );

	bgfx::setViewTransform(0, &view[0][0], &proj[0][0]);
    bgfx::setViewRect(0, 0, 0, uint16_t(m_width), uint16_t(m_height));
    bgfx::touch(0);
    
	mCubeManager.render();
}