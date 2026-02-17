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

    void AnimationGroup::WriteHeader(bStream::CStream* stream){
        stream->writeUInt16(KeyBufferSize);
        stream->writeUInt16(MaterialIndex);
        stream->writeUInt16(Padding);
        stream->writeUInt16(KeyCount);
        stream->writeUInt32(0);
    }

    void AnimationGroup::WriteData(bStream::CStream* stream){
        for (size_t i = 0; i < KeyBufferSize; i++){
            stream->writeUInt16(Frames[i]);
        }
    }

    void Header::Read(bStream::CStream* stream){
        Version = stream->readUInt16();
        Unknown = stream->readUInt16();
        GroupCount = stream->readUInt16();
        FrameCount = stream->readUInt16();
        KeyDataOffset = stream->readUInt32();
    }

    void Header::Write(bStream::CStream* stream){
        stream->writeUInt16(Version);
        stream->writeUInt16(Unknown);
        stream->writeUInt16(GroupCount);
        stream->writeUInt16(FrameCount);
        stream->writeUInt32(KeyDataOffset);
    }

    void Animation::Load(bStream::CStream* stream){
        TxpHeader.Read(stream);

        for (size_t i = 0; i < TxpHeader.GroupCount; i++){
            AnimationGroup group;
            group.Read(stream);
            Groups.push_back(group);
        }
    }

    void Animation::Save(bStream::CStream* stream){
        TxpHeader.GroupCount = Groups.size();
        TxpHeader.Write(stream);
        std::size_t groupsOffset = stream->tell();
        for (int i = 0; i < Groups.size(); i++){
            Groups[i].WriteHeader(stream);
        }
        for (int i = 0; i < Groups.size(); i++){
            stream->writeOffsetAt32(groupsOffset + (0x0C*Groups.size())+8);
            Groups[i].WriteData(stream);
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
