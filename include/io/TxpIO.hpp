#pragma once
#include <vector>
#include "../lib/bStream/bstream.h"

namespace TXP {
    struct AnimationGroup {
        uint16_t KeyBufferSize;
        uint16_t MaterialIndex;
        uint16_t Padding;
        uint16_t KeyCount;
        uint32_t KeyDataOffset;

        std::vector<uint16_t> Frames;

        void Read(bStream::CStream* stream);
    };

    struct Header {
        uint16_t Version;
        uint16_t Unknown;
        uint16_t GroupCount;
        uint16_t FrameCount;
        uint32_t KeyDataOffset;
    
        void Read(bStream::CStream* stream); 
    };

    class Animation {
        uint32_t CurrentFrame { 0 };
        Header TxpHeader;
        std::vector<AnimationGroup> Groups;

    public:

        void SetFrame(uint32_t frame) { CurrentFrame = frame; }
        uint32_t GetFrame() { return CurrentFrame; }
        uint32_t GetFrameCount() { return TxpHeader.FrameCount; }
        
        uint32_t GetSamplerIndex(uint32_t materialIndex);
        void Load(bStream::CStream* stream);

        void Update(float dt);

        Animation(){}
        ~Animation(){}
    };
}
