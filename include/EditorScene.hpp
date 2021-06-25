#include <map>
#include <string>
#include "../lib/bigg/include/bigg.hpp"
#include <bx/math.h>
#include <bgfx/bgfx.h>

class LSceneModel {
    //TODO: set this up properly
};

class LEditorScene {
private:
    
    bgfx::ProgramHandle mCubeShader;
    bgfx::InstanceDataBuffer mCubeInstaces;

    std::map<std::string, LSceneModel> models;
    
	bx::Vec3 at  = { 0.0f, 0.0f,  0.0f };
	bx::Vec3 eye = { 0.0f, 0.0f, -7.0f };

public:
    void RenderSubmit(u_int32_t, u_int32_t);
    
    LEditorScene();
    ~LEditorScene();
};