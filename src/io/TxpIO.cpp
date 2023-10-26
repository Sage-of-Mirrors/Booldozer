#include "io/TxpIO.hpp"
#include <cmath>

namespace TXP {

    void AnimationGroup::Read(bStream::CStream* stream){
        KeyBufferSize = stream->readUInt16();
        MaterialIndex = stream->readUInt16();
        Padding = stream->readUInt16();
        KeyCount = stream->readUInt16();
        KeyDataOffset = stream->readUInt32();

        uint32_t r = stream->tell();
        stream->seek(KeyDataOffset);
        for (size_t i = 0; i < KeyBufferSize; i++){
            Frames.push_back(stream->readUInt16());
        }
        stream->seek(r);
    }

    void Header::Read(bStream::CStream* stream){
        Version = stream->readUInt16();
        Unknown = stream->readUInt16();
        GroupCount = stream->readUInt16();
        FrameCount = stream->readUInt16();
        KeyDataOffset = stream->readUInt32();
    }

    void Animation::Load(bStream::CStream* stream){
        TxpHeader.Read(stream);
        
        for (size_t i = 0; i < TxpHeader.GroupCount; i++){
            AnimationGroup group;
            group.Read(stream);
            Groups.push_back(group);
        }
    }

    void Animation::Update(float dt){
        if(CurrentFrame < TxpHeader.FrameCount-1) CurrentFrame++;
    }

    uint32_t Animation::GetSamplerIndex(uint32_t materialIndex){
        for (AnimationGroup group : Groups){
            if(group.MaterialIndex == materialIndex && group.Frames.size() >= 1){
                return group.Frames[CurrentFrame]; //todo actually animate this?
            }
        }
        return UINT32_MAX;
    }
}