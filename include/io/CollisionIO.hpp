#pragma once

#include "../lib/bStream/bstream.h"
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

    bool mFloor;
    glm::vec3 v1, v2, v3;
};

class LCollisionIO {
public:
    void LoadMp(std::filesystem::path path, std::weak_ptr<LMapDOMNode> map);
    void LoadObj(std::filesystem::path path, std::weak_ptr<LMapDOMNode> map);

    LCollisionIO(){}
    ~LCollisionIO(){}
};