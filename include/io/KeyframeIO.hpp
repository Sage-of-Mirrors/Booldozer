#pragma once
#include "../lib/bStream/bstream.h"
#include <map>
#include <vector>

enum class ETrackType
{
	CMN,
	PTH,
    ANM
};

struct LKeyframeCommon
{
    float frame;
    float value;
    float inslope { 1.0f };
    float outslope { 1.0f };
};

class LTrackCommon
{
    ETrackType mType;
    bool mUnifiedSlope;
    

public:
    std::vector<int32_t> mKeys;
    std::map<int32_t, LKeyframeCommon> mFrames;

    void LoadTrack(bStream::CStream* stream, uint32_t keyframeDataOffset, ETrackType type);

    LTrackCommon(){}
    ~LTrackCommon(){}
};