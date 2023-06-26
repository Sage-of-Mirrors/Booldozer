#pragma once

#include <map>
#include <string>
#include <memory>
#include <vector>
#include "DOM/RoomDOMNode.hpp"
#include "DOM/RoomDataDOMNode.hpp"
#include "DOM/FurnitureDOMNode.hpp"
#include "DOM/ObjectDOMNode.hpp"
#include "DOM/ObserverDOMNode.hpp"
#include "DOM/MapDOMNode.hpp"
#include "DOM/DoorDOMNode.hpp"
#include "io/BinIO.hpp"
#include "ResUtil.hpp"

#include "scene/Camera.hpp"
#include <J3D/J3DModelLoader.hpp>
#include <J3D/J3DModelData.hpp>
#include <J3D/J3DUniformBufferObject.hpp>
#include <J3D/J3DLight.hpp>
#include <J3D/J3DModelInstance.hpp>

#include "UPathRenderer.hpp"
#include "UPointSpriteManager.hpp"

class LCubeManager {
private:
    uint32_t mVao, mVbo, mIbo, mCubeProgram, mCubeTex;

public:

    void init();
    void render(glm::mat4* transform, bool wireframe);

    LCubeManager();
    ~LCubeManager();

};


class LEditorScene {
    bool Initialized;
    glm::mat4 gridMatrix;

    LCubeManager mCubeManager;
    CPointSpriteManager mPointManager;
    CPathRenderer mPathRenderer;
    
    std::vector<std::weak_ptr<LDoorDOMNode>> mRoomDoors;
    std::vector<std::weak_ptr<LRoomDOMNode>> mCurrentRooms;
    
    //TODO: Fill door models and figure out how to handle drawing them
    std::vector<std::shared_ptr<BinModel>> mDoorModels;
    std::vector<std::shared_ptr<BinModel>> mRoomModels;
    std::map<std::string, std::shared_ptr<BinModel>> mRoomFurniture;

    std::shared_ptr<J3DModelData> mSkyboxModel, mCoinModel;
    std::shared_ptr<J3DModelInstance> mSkyBox, mCoin;

public:
    LSceneCamera Camera;
    std::vector<LSceneCamera*> mViewports;
    
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