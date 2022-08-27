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
    float inslope;
    float outslope;
};

class LTrackCommon
{
    ETrackType mType;
    bool mUnifiedSlope;
    

public:
    std::vector<uint32_t> mKeys;
    std::map<uint32_t, LKeyframeCommon> mFrames;

    void LoadTrack(bStream::CStream* stream, uint32_t keyframeDataOffset, ETrackType type);

    LTrackCommon(){}
    ~LTrackCommon(){}
};