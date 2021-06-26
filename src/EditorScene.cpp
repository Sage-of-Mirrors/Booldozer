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

LEditorScene::LEditorScene(){}

void LEditorScene::init(){

    PosColorVertex::init();
	mCubeVbh = bgfx::createVertexBuffer( bgfx::makeRef(s_cubeVertices, sizeof(s_cubeVertices) ), PosColorVertex::ms_layout );
	mCubeIbh = bgfx::createIndexBuffer( bgfx::makeRef(s_cubeTriList, sizeof(s_cubeTriList) ) );
    mCubeShader = bigg::loadProgram( "shaders/vs_cubes.bin", "shaders/fs_cubes.bin" );
    
}

LEditorScene::~LEditorScene(){
    bgfx::destroy(mCubeShader);

}

void LEditorScene::RenderSubmit(u_int32_t m_width, u_int32_t m_height){
    
	glm::mat4 view = glm::lookAt( glm::vec3( 0.0f, 0.0f, -35.0f ), glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3( 0.0f, 1.0f, 0.0f ) );
	glm::mat4 proj = glm::perspective( glm::radians( 60.0f ), float(m_width) / m_height, 0.1f, 100.0f );

	bgfx::setViewTransform(0, &view[0][0], &proj[0][0]);
    bgfx::setViewRect(0, 0, 0, uint16_t(m_width), uint16_t(m_height));
    bgfx::touch(0);
    
	glm::mat4 mtx = glm::identity<glm::mat4>();
	bgfx::setTransform(&mtx[0][0]);
	bgfx::setVertexBuffer(0, mCubeVbh);
	bgfx::setIndexBuffer(mCubeIbh);
	bgfx::setState( BGFX_STATE_DEFAULT );
	bgfx::submit(0, mCubeShader);
}