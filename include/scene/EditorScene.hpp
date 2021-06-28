#pragma once

#include <map>
#include <string>
#include <memory>
#include <vector>
#include "../../lib/bigg/include/bigg.hpp"
#include <bx/math.h>
#include <bgfx/bgfx.h>

#include "scene/Camera.hpp"

class LModelManager {
private:
    const static uint32_t mInstanceStride = 64;

protected:
    //std::map<size_t, glm::mat4> mInstanceData;
    std::vector<std::shared_ptr<glm::mat4>> mInstanceData;
    bgfx::InstanceDataBuffer mModelInstances;
    void generateInstanceBuffer();

public:

    std::shared_ptr<glm::mat4> addInstance(glm::mat4 transform);
    virtual void render();

    LModelManager();
    ~LModelManager();
};

class LCubeManager : public LModelManager {
private:
    bgfx::ProgramHandle mCubeShader;
    bgfx::VertexBufferHandle mCubeVbh;
    bgfx::IndexBufferHandle mCubeIbh;
    bgfx::TextureHandle mCubeTexture;
    bgfx::UniformHandle mCubeTexUniform;


public:
    void init();
    void render() override;

    LCubeManager();
    ~LCubeManager();

};


class LEditorScene {
private:
    LSceneCamera mCamera;

public:
    bool Initialized;

    LCubeManager mCubeManager;
    std::map<std::string, LModelManager> mSceneModels;
    
    glm::mat4 getCameraView();
    glm::mat4 getCameraProj();

    std::shared_ptr<glm::mat4> InstanceModel(std::string name, glm::mat4 transform);
    void RenderSubmit(uint32_t m_width, uint32_t m_height);
    
    void init();
    void update(GLFWwindow* window, float dt);

    LEditorScene();
    ~LEditorScene();
};