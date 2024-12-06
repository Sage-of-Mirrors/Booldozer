#include "io/KeyframeIO.hpp"


void LTrackCommon::LoadTrack(bStream::CStream* stream, uint32_t keyframeDataOffset, ETrackType type)
{
    mType = type;
    uint16_t keyCount = stream->readUInt16();
    uint16_t beginIndex = stream->readUInt16();
    uint16_t elementCountFlags = stream->readUInt16();

    size_t group = stream->tell();

    //LGenUtility::Log << "Track Properties:\nKey Count: " << std::dec << keyCount << "\nBegin Index: " << beginIndex << "\nElement Count/Flags: " << elementCountFlags << std::endl;

    if(mType == ETrackType::CMN || mType == ETrackType::PTH)
    {
        mUnifiedSlope = (elementCountFlags == 3);
    } else if(type == ETrackType::ANM) {
        mUnifiedSlope = (elementCountFlags == 0x80);
    }

    //LGenUtility::Log << "Reading keyframe at " << std::hex << keyframeDataOffset + (4 * beginIndex) << std::endl;

    stream->seek(keyframeDataOffset + (4 * beginIndex));
    for (size_t frame = 0; frame < keyCount; frame++)
    {
        
        LKeyframeCommon keyframe;

        keyframe.frame = stream->readFloat();

        if(mType == ETrackType::CMN || mType == ETrackType::PTH)
        {
            //LGenUtility::Log << "uh" << std::endl;
            if(elementCountFlags == 1) {
                keyframe.frame = 0;
                keyframe.value = keyframe.frame;

                //LGenUtility::Log << "Single frame track value: " << keyframe.value << std::endl;
            } else {
                keyframe.value = stream->readFloat();

                //LGenUtility::Log << "Frame: " << keyframe.frame << "\nValue: " << keyframe.value << std::endl;
            }

            if(elementCountFlags == 3){
                float slope = stream->readFloat();

                keyframe.inslope = slope;
                keyframe.outslope = slope;

                //LGenUtility::Log << "Slope: " << slope << std::endl;
            } else if (elementCountFlags == 4) {
                keyframe.inslope = stream->readFloat();
                keyframe.outslope = stream->readFloat();

                //LGenUtility::Log << "In Slope: " << keyframe.inslope << std::endl;
                //LGenUtility::Log << "Out Slope: " << keyframe.outslope << std::endl;
            }
        } else if(mType == ETrackType::ANM && keyCount > 1) {
            keyframe.value = stream->readFloat();
            float slope = stream->readFloat();

            keyframe.inslope = slope;
            keyframe.outslope = slope;

            if(elementCountFlags == 0x80)
            {
                keyframe.outslope = stream->readFloat();
            }
        }
        
        mFrames.insert(std::make_pair((uint32_t)keyframe.frame, keyframe));
    }

    stream->seek(group);

    for (auto& frame : mFrames)
    {
        mKeys.push_back(frame.first);
    }

}