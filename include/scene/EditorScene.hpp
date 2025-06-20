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
#include "io/MdlIO.hpp"
#include "io/TxpIO.hpp"
#include "ResUtil.hpp"

#include "scene/Camera.hpp"
#include <J3D/J3DModelLoader.hpp>
#include <J3D/Data/J3DModelData.hpp>
#include <J3D/Material/J3DUniformBufferObject.hpp>
#include <J3D/Rendering/J3DLight.hpp>
#include <J3D/Data/J3DModelInstance.hpp>

#include "UPathRenderer.hpp"
#include "UPointSpriteManager.hpp"
#include "UPlaneRenderer.hpp"

class LEditorScene {
    bool Initialized;
    glm::mat4 gridMatrix;

    uint32_t mFbo, mRbo, mViewTex, mPickTex;
    uint32_t mPrevWidth, mPrevHeight;

    CPlaneRenderer mMirrorRenderer;
    CPointSpriteManager mPointManager;
    CPathRenderer mPathRenderer;
    
    std::vector<std::weak_ptr<LDoorDOMNode>> mRoomDoors;
    std::vector<std::weak_ptr<LRoomDOMNode>> mCurrentRooms;
    
    std::vector<std::shared_ptr<BIN::Model>> mDoorModels;
    std::map<std::string, std::shared_ptr<BIN::Model>> mRoomModels;


    std::map<std::string, std::unique_ptr<MDL::Model>> mActorModels;
    std::map<std::string, std::unique_ptr<TXP::Animation>> mMaterialAnimations;

    std::shared_ptr<J3DModelData> mSkyboxModel, mCoinModel;
    std::shared_ptr<J3DModelInstance> mSkyBox, mCoin;

    uint32_t mSelectedRoomNumber;


    bool mActive { false };

public:
    LSceneCamera Camera;
    std::map<std::string, std::shared_ptr<BIN::Model>> mRoomFurniture;
    
    glm::mat4 getCameraView();
    glm::mat4 getCameraProj();

    static void SetDirty();
    void SetActive(bool active) { mActive = active; }

    void SetRoom(std::shared_ptr<LRoomDOMNode> room);
    bool HasRoomLoaded(int32_t roomNumber);
    void LoadActor(std::string name, bool log = true);

    void UpdateRenderers();
    void RenderSubmit(uint32_t m_width, uint32_t m_height);

    void Init();
    void Clear();
    void Update(GLFWwindow* window, float dt, LEditorSelection* selection);
    void LoadResFromRoot();

    static LEditorScene* GetEditorScene();

    LEditorScene();
    ~LEditorScene();
};
