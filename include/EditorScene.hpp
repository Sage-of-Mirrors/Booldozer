#include <map>
#include <string>
#include "../lib/bigg/include/bigg.hpp"
#include <bx/math.h>
#include <bgfx/bgfx.h>

class LSceneModel {
    //TODO: set this up properly
};

class LCubeManager {
private:
    const static uint32_t mCubeInstanceStride = 64;
    bgfx::ProgramHandle mCubeShader;
    bgfx::InstanceDataBuffer mCubeInstances;
    bgfx::VertexBufferHandle mCubeVbh;
    bgfx::IndexBufferHandle mCubeIbh;
    bgfx::TextureHandle mCubeTexture;
    bgfx::UniformHandle mCubeTexUniform;
    std::map<size_t, glm::mat4> mInstanceData;
    size_t mNextId;

    void updateInstanceBuffer();

public:
    void init();
    void setTransform(size_t id, glm::mat4 transform);
    size_t addCube(glm::mat4 transform);
    void removeCube(size_t id);
    void render();

    LCubeManager();
    ~LCubeManager();

};


class LEditorScene {
private:
public:
    bool Initialized;
    LCubeManager mCubeManager;


    std::map<std::string, LSceneModel> models;
    
	bx::Vec3 at  = { 0.0f, 0.0f,  0.0f };
	bx::Vec3 eye = { 0.0f, 0.0f, -35.0f };
    
    void RenderSubmit(uint32_t, uint32_t);
    void init();

    LEditorScene();
    ~LEditorScene();
};