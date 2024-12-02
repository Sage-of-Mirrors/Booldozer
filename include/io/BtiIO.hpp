#pragma once
#include "bstream.h"

namespace ImageFormat {
    namespace Decode {
        void CMPR(bStream::CStream* stream, uint16_t width, uint16_t height, uint8_t* imageData);
        
        void RGB5A3(bStream::CStream* stream, uint16_t width, uint16_t height, uint8_t* imageData);
        void RGB565(bStream::CStream* stream, uint16_t width, uint16_t height, uint8_t* imageData);
        
        void I4(bStream::CStream* stream, uint16_t width, uint16_t height, uint8_t* imageData);
        void I8(bStream::CStream* stream, uint16_t width, uint16_t height, uint8_t* imageData);

        void IA4(bStream::CStream* stream, uint16_t width, uint16_t height, uint8_t* imageData);
        void IA8(bStream::CStream* stream, uint16_t width, uint16_t height, uint8_t* imageData);
    }

    namespace Encode {
        void CMPR(bStream::CStream* stream, uint16_t width, uint16_t height, uint8_t* imageData);
        void RGB5A3(bStream::CStream* stream, uint16_t width, uint16_t height, uint8_t* imageData);
        void RGB565(bStream::CStream* stream, uint16_t width, uint16_t height, uint8_t* imageData);


        void I4(bStream::CStream* stream, uint16_t width, uint16_t height, uint8_t* imageData);
        void I8(bStream::CStream* stream, uint16_t width, uint16_t height, uint8_t* imageData);

        void IA4(bStream::CStream* stream, uint16_t width, uint16_t height, uint8_t* imageData);
        void IA8(bStream::CStream* stream, uint16_t width, uint16_t height, uint8_t* imageData);
    }
};

class Bti {
    uint8_t mFormat { 0 };
    uint8_t mEnableAlpha { 0 };
    uint8_t mWrapS { 0 };
    uint8_t mWrapT { 0 };
    uint16_t mPaletteFormat { 0 };
    uint16_t mNumPaletteEntries { 0 };
    uint32_t mPaletteOffsetData { 0 };
    uint8_t mMipMapEnabled { 0 };
    uint8_t mEdgeLODEnabled { 0 };
    uint8_t mClampLODBias { 0 };
    uint8_t mMaxAnisotropy { 0 };
    uint8_t mMinFilterType { 0 };
    uint8_t mMagFilterType { 0 };
    uint8_t mMinLOD { 0 };
    uint8_t mMaxLOD { 0 };
    uint8_t mNumImages { 0 };
    uint16_t mLODBias { 0 };

public:
    uint16_t mWidth { 0 };
    uint16_t mHeight { 0 };

    uint8_t* DecodeImage(bStream::CStream* stream);
    void EncodeImage(bStream::CStream* stream, uint8_t* imageData);

};