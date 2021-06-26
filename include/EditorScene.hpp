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
public:
    bool Initialized;
    bgfx::ProgramHandle mCubeShader;
    bgfx::InstanceDataBuffer mCubeInstaces;
    bgfx::VertexBufferHandle mCubeVbh;
    bgfx::IndexBufferHandle mCubeIbh;


    std::map<std::string, LSceneModel> models;
    
	bx::Vec3 at  = { 0.0f, 0.0f,  0.0f };
	bx::Vec3 eye = { 0.0f, 0.0f, -35.0f };

    void RenderSubmit(uint32_t, uint32_t);
    void init();

    LEditorScene();
    ~LEditorScene();
};