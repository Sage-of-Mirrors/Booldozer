#include "io/KeyframeIO.hpp"

namespace KeyframeIO { // TODO: Reorg so all classes and functions are in animation NS
    inline float ReadValue(bStream::CStream* stream, uint32_t valueSize){
        if(valueSize == 4){
            return stream->readFloat();
        } else {
            return static_cast<float>(stream->readInt16()) * 0.001533981f;
        }
    }
}

void LTrackCommon::LoadTrackEx(bStream::CStream* stream, uint32_t keyframeDataOffset, uint32_t beginIndex, uint8_t count, bool hasSlopeIn, bool hasSlopeOut, uint32_t valueSize){
    std::size_t streamPosition = stream->tell();
    stream->seek(keyframeDataOffset + (valueSize * beginIndex));
    for (std::size_t frame = 0; frame < count; frame++){
        
        LKeyframeCommon keyframe;

        if(valueSize == 2){
            keyframe.frame = stream->readUInt16();
        } else {
            keyframe.frame = stream->readFloat();
        }

        if(count == 1) {
            keyframe.value = keyframe.frame;
            keyframe.frame = 0;
        } else {
            keyframe.value = KeyframeIO::ReadValue(stream, valueSize);
        }
        
        if(hasSlopeIn && count > 1){
            keyframe.inslope = KeyframeIO::ReadValue(stream, valueSize) * 0.001533981f; /// huh????
            keyframe.outslope = keyframe.inslope;
            if(hasSlopeOut){
                keyframe.outslope = KeyframeIO::ReadValue(stream, valueSize) * 0.001533981f;
            }
        }            
        mFrames.insert(std::make_pair((uint32_t)keyframe.frame, keyframe));
    }
    
    for (auto& frame : mFrames)
    {
        mKeys.push_back(frame.first);
    }

    stream->seek(streamPosition);
}

void LTrackCommon::LoadTrack(bStream::CStream* stream, uint32_t keyframeDataOffset, ETrackType type)
{
    mType = type;
    uint16_t keyCount = stream->readUInt16();
    uint16_t beginIndex = stream->readUInt16();
    uint16_t elementCountFlags = stream->readUInt16();

    bool inSlope = false;
    bool outSlope = false;
    if(mType == ETrackType::CMN || mType == ETrackType::PTH)
    {
        inSlope = (elementCountFlags == 3);
        outSlope = (elementCountFlags == 4);
    } else if(type == ETrackType::ANM) {
        inSlope = true;
        outSlope = (elementCountFlags == 0x80);
    }
    
    // As far as I know only KEY anims use int16s for any value type, for the util function just pass size 4, can edit later if needed
    LoadTrackEx(stream, keyframeDataOffset, beginIndex, keyCount, inSlope, outSlope, 4);
}