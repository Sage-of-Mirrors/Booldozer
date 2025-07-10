#pragma once
#include "../lib/bStream/bstream.h"
#include <map>
#include <vector>

enum class ETrackType
{
	CMN,
	PTH,
    ANM,
    KEY
};

struct LKeyframeCommon
{
    float frame;
    float value;
    float inslope { 0.0f };
    float outslope { 0.0f };
};

class LTrackCommon
{
    ETrackType mType;
public:
    std::vector<int32_t> mKeys;
    std::map<int32_t, LKeyframeCommon> mFrames;

    void LoadTrack(bStream::CStream* stream, uint32_t keyframeDataOffset, ETrackType type);
    void LoadTrackEx(bStream::CStream* stream, uint32_t keyframeDataOffset, uint32_t beginIndex, uint8_t count, bool hasSlopeIn, bool hasSlopeOut, uint32_t valueSize=4);

    LTrackCommon(){}
    ~LTrackCommon(){}
};