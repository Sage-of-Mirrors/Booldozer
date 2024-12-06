#pragma once

#include "../lib/bStream/bstream.h"
#include "GenUtil.hpp"
#include "DOM/MapDOMNode.hpp"
#include "DOM/ItemAppearDOMNode.hpp"
#include "DOM/ItemFishingDOMNode.hpp"
#include <memory>
#include <map>

enum EAttackPattern {
    Punch,
    Punch2,
    Uppercut,
    Crush,
    Special,
    Bowling,
    SpearStab,
    SpearTwirl,
    BoneThrow,
    Grab,
    Banana,
    Bomb,
    Dance,
    CeilingDrop,
    Slam,
    Run
};

enum EFleePattern {
    Default,
    UpDown,
    Circle,
    Weak,
    CircleVertical,
    None
};

enum EShieldType {
    NoShield,
    Fire,
    Water,
    Ice
};

struct LCTPrm {
    uint32_t mLife;
    uint32_t mHitDamage;
    uint32_t mSpeed;
    uint32_t mSpeedUnseen;
    uint32_t mSpeedFight;
    uint32_t mEyesight;
    uint32_t mLightBindFrame;
    float mMinLightBindRange;
    float mMaxLightBindRange;
    uint32_t mNumAtkKarakai;
    uint32_t mNumAtkOrooro;
    float mHikiPower;
    float mEffectiveDegree;
    float mTsuriHeight;
    uint32_t mDissapearFrame;
    EAttackPattern mAttackPattern;
    EFleePattern mTsuriType;
    bool mActAfterSu;
    bool mActAfterFa;
    EShieldType mElement;
    bool mCheckbox;
    std::weak_ptr<LItemFishingDOMNode>  mTsuriItemTblId;
    std::weak_ptr<LItemAppearDOMNode>  mNormalItemTblId;
    float mPointerRange;
    glm::vec4 mBrightColor;
    glm::vec4 mAmbColor;
    uint32_t mFrightenLevel;
    uint32_t mAttackPattern2;
    uint8_t mHasCoin;
    float mKiryuCount;
    float mNumGround;
    uint8_t mFlag;
    std::vector<uint16_t> mEnabledProperties;
};

class LPrmIO {
    bool mConfigsLoaded { false };
    int mSelectedConfig;
    std::weak_ptr<LMapDOMNode> mMap;
    std::vector<std::string> mLoadedConfigs;
    std::map<std::string, std::shared_ptr<LCTPrm>> mCtpParams;
public:
    bool mParamToolOpen { false };
    void LoadConfigs(std::shared_ptr<LMapDOMNode>& map);
    void SaveConfigsToFile();
    void Load(std::string name, bStream::CStream* stream);
    void Save(std::string name, bStream::CStream* stream);
    bool RenderUI();

    LPrmIO();
    ~LPrmIO();
};