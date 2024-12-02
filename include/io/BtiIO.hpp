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

namespace Bti {
    uint8_t* DecodeImage(bStream::CStream* stream, uint32_t& w, uint32_t& h);
}