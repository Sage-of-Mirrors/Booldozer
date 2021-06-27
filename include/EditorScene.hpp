#include <map>
#include <string>
#include "../lib/bigg/include/bigg.hpp"
#include <bx/math.h>
#include <bgfx/bgfx.h>

/*

    TODO: Seriously clean up how scene model and cube manager works. Make parent LModelManager class?

*/

class LSceneModel {
private:
    std::map<size_t, glm::mat4> mInstanceData;
    size_t mNextID;
public:
    size_t addInstance(glm::mat4 transform);
    void setTransform(size_t id, glm::mat4 transform);

    LSceneModel();
    ~LSceneModel();
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


    std::map<std::string, LSceneModel> mSceneModels;
    
	bx::Vec3 at  = { 0.0f, 0.0f,  0.0f };
	bx::Vec3 eye = { 0.0f, 0.0f, -35.0f };
    
    size_t RegisterModel(std::string name, glm::mat4 transform);
    void UpdateModelPosition(std::string name, size_t id, glm::mat4 transform);
    void RenderSubmit(uint32_t m_width, uint32_t m_height);
    void init();

    LEditorScene();
    ~LEditorScene();
};