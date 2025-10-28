#pragma once

#include "bstream.h"
#include "GenUtil.hpp"
#include "DOM/MapDOMNode.hpp"
#include <filesystem>
#include <memory>
#include <map>

struct GridCell {
    std::vector<uint16_t> mAll {};
    std::vector<uint16_t> mFloor {};
};

struct CollisionTriangle {
    uint16_t mVtx1;
    uint16_t mVtx2;
    uint16_t mVtx3;
    uint16_t mNormal;
    uint16_t mEdgeTan1;
    uint16_t mEdgeTan2;
    uint16_t mEdgeTan3;
    uint16_t mUnkIdx;
    float mDot;
    uint16_t mMask;
    uint16_t mFriction;

    uint32_t mSound;
    uint32_t mSoundEchoSwitch;
    uint32_t mLadder;
    uint32_t mIgnorePointer;
    uint32_t mSurfMaterial;

    bool mFloor;
    glm::vec3 v1, v2, v3;
    int16_t mTriIdx;
};

struct ColModel {
    glm::vec3 min, max;
    std::vector<glm::vec3> mVertices;
    std::vector<glm::vec3> mNormals;
    std::vector<CollisionTriangle> mTriangles;
};

class LCollisionIO {
public:
    void LoadMp(std::filesystem::path path, std::weak_ptr<LMapDOMNode> map);
    void LoadObj(std::filesystem::path path, std::weak_ptr<LMapDOMNode> map, std::map<std::string, std::string> propertyMap, bool bakeFurniture);
    void LoadFBX(std::filesystem::path path, std::weak_ptr<LMapDOMNode> map, std::map<std::string, std::string> propertyMap, bool bakeFurniture){}

    LCollisionIO(){}
    ~LCollisionIO(){}
};
