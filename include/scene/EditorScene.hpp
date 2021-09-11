#pragma once

#include <map>
#include <string>
#include <memory>
#include <vector>
#include "../../lib/bigg/include/bigg.hpp"
#include "DOM/RoomDOMNode.hpp"
#include "DOM/RoomDataDOMNode.hpp"
#include "DOM/FurnitureDOMNode.hpp"
#include "DOM/ObjectDOMNode.hpp"
#include "DOM/ObserverDOMNode.hpp"
#include "DOM/MapDOMNode.hpp"
#include "DOM/DoorDOMNode.hpp"
#include <bx/math.h>
#include <bgfx/bgfx.h>
#include "io/BinIO.hpp"
#include "ResUtil.hpp"

#include "scene/Camera.hpp"

class LCubeManager {
private:

    bgfx::VertexBufferHandle mCubeVbh;
    bgfx::IndexBufferHandle mCubeIbh;
    bgfx::TextureHandle mCubeTexture;

public:

    bgfx::ProgramHandle mCubeShader;
    bgfx::UniformHandle mCubeTexUniform;
    void init();
    void render(glm::mat4* transform);
    void renderAltTex(glm::mat4* transform, bgfx::TextureHandle& tex);

    LCubeManager();
    ~LCubeManager();

};


class LEditorScene {
    bool Initialized;
    glm::mat4 gridMatrix;

    LCubeManager mCubeManager;
    std::vector<std::weak_ptr<LDoorDOMNode>> mRoomDoors;
    std::vector<std::weak_ptr<LRoomDOMNode>> mCurrentRooms;
    
    //TODO: Fill door models and figure out how to handle drawing them
    std::vector<std::shared_ptr<BGFXBin>> mDoorModels;
    std::vector<std::shared_ptr<BGFXBin>> mRoomModels;
    std::map<std::string, std::shared_ptr<BGFXBin>> mRoomFurniture;
    
    bgfx::ProgramHandle mShader;
    bgfx::UniformHandle mTexUniform;
    bgfx::TextureHandle mBorderTex;
    bgfx::TextureHandle mObserverTex;

public:
    LSceneCamera Camera;
    glm::mat4 getCameraView();
    glm::mat4 getCameraProj();

    void SetRoom(std::shared_ptr<LRoomDOMNode> room);
    bool HasRoomLoaded(int32_t roomNumber);
    void RenderSubmit(uint32_t m_width, uint32_t m_height);

    void init();
    void update(GLFWwindow* window, float dt, LEditorSelection* selection);

    LEditorScene();
    ~LEditorScene();
};