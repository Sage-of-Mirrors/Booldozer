#include "EditorScene.hpp"
#include <fstream>

LEditorScene::LEditorScene(){
	mCubeShader = bigg::loadProgram( "cube_shader_v.bin", "cube_shader_f.bin" );
}

LEditorScene::~LEditorScene(){
    bgfx::destroy(mCubeShader);

}

void LEditorScene::RenderSubmit(u_int32_t m_width, u_int32_t m_height){
    {
        float view[16];
        float proj[16];

        bx::mtxLookAt(view, this->eye, this->at);
		bx::mtxProj(proj, 60.0f, float(m_width) / float(m_height), 0.1f, 100.0f, bgfx::getCaps()->homogeneousDepth);
		bgfx::setViewTransform(0, view, proj);

    }
}