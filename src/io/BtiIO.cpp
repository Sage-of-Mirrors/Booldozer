#include "io/BtiIO.hpp"
#include <cmath>

namespace ColorFormat {

// a few of these are based off of wwlib texture utils by LagoLunatic
uint16_t toGreyscale(uint32_t color){
    uint32_t r = color & 0xFF;
    uint32_t g = color & 0x00FF;
    uint32_t b = color & 0x0000FF;
    return std::round(((r * 30) + (g * 59) + (b * 11)) / 100);
}

uint8_t RGBA8toI4(uint32_t color){
    uint16_t greyscale = toGreyscale(color);
    return (greyscale >> 4) & 0xF;
}

uint8_t RGBA8toIA4(uint32_t color){
    uint16_t greyscale = toGreyscale(color);
    uint8_t output = 0;
    output |= (greyscale >> 4) & 0xF;
    output |= ((color << 24) & 0xFF) & 0xF0;
    return output;
}

uint8_t RGBA8toI8(uint32_t color){
    uint16_t greyscale = toGreyscale(color);
    return greyscale & 0xFF;
}

uint16_t RGBA8toIA8(uint32_t color){
    uint16_t greyscale = toGreyscale(color);
    uint16_t output = 0x0000;
    output |= greyscale & 0x00FF;
    output |= ((color & 0x000000FF) << 8) & 0xFF00;
    return output;
}

uint32_t RGB565toRGBA8(uint16_t data) {
	uint8_t r = (data & 0xF100) >> 11;
	uint8_t g = (data & 0x07E0) >> 5;
	uint8_t b = (data & 0x001F);

	uint32_t output = 0x000000FF;
	output |= (r << 3) << 24;
	output |= (g << 2) << 16;
	output |= (b << 3) << 8;

	return output;
}


uint32_t RGB5A3toRGBA8(uint16_t data) {
	uint8_t r, g, b, a;

	// No alpha bits to extract.
	if (data & 0x8000) {
		a = 0xFF;

		r = (data & 0x7C00) >> 10;
		g = (data & 0x03E0) >> 5;
		b = (data & 0x001F);

		r = (r << (8 - 5)) | (r >> (10 - 8));
		g = (g << (8 - 5)) | (g >> (10 - 8));
		b = (b << (8 - 5)) | (b >> (10 - 8));
	}
	// Alpha bits present.
	else {
		a = (data & 0x7000) >> 12;
		r = (data & 0x0F00) >> 8;
		g = (data & 0x00F0) >> 4;
		b = (data & 0x000F);

		a = (a << (8 - 3)) | (a << (8 - 6)) | (a >> (9 - 8));
		r = (r << (8 - 4)) | r;
		g = (g << (8 - 4)) | g;
		b = (b << (8 - 4)) | b;
	}

	uint32_t output = a;
	output |= r << 24;
	output |= g << 16;
	output |= b << 8;

	return output;
}

uint8_t* DecodeCMPRSubBlock(bStream::CStream* stream) {
	uint8_t* data = new uint8_t[4 * 4 * 4]{};

	uint16_t color0 = stream->readUInt16();
	uint16_t color1 = stream->readUInt16();
	uint32_t bits = stream->readUInt32();

	uint32_t colorTable[4]{};
	colorTable[0] = ColorFormat::RGB565toRGBA8(color0);
	colorTable[1] = ColorFormat::RGB565toRGBA8(color1);

	uint8_t r0, g0, b0, a0, r1, g1, b1, a1;
	r0 = (colorTable[0] & 0xFF000000) >> 24;
	g0 = (colorTable[0] & 0x00FF0000) >> 16;
	b0 = (colorTable[0] & 0x0000FF00) >> 8;
	a0 = (colorTable[0] & 0x000000FF);

	r1 = (colorTable[1] & 0xFF000000) >> 24;
	g1 = (colorTable[1] & 0x00FF0000) >> 16;
	b1 = (colorTable[1] & 0x0000FF00) >> 8;
	a1 = (colorTable[1] & 0x000000FF);

	if (color0 > color1) {
		colorTable[2] |= ((2 * r0 + r1) / 3) << 24;
		colorTable[2] |= ((2 * g0 + g1) / 3) << 16;
		colorTable[2] |= ((2 * b0 + b1) / 3) << 8;
		colorTable[2] |= 0xFF;

		colorTable[3] |= ((r0 + 2 * r1) / 3) << 24;
		colorTable[3] |= ((g0 + 2 * g1) / 3) << 16;
		colorTable[3] |= ((b0 + 2 * b1) / 3) << 8;
		colorTable[3] |= 0xFF;
	}
	else {
		colorTable[2] |= ((r0 + r1) / 2) << 24;
		colorTable[2] |= ((g0 + g1) / 2) << 16;
		colorTable[2] |= ((b0 + b1) / 2) << 8;
		colorTable[2] |= 0xFF;

		colorTable[3] |= ((r0 + 2 * r1) / 3) << 24;
		colorTable[3] |= ((g0 + 2 * g1) / 3) << 16;
		colorTable[3] |= ((b0 + 2 * b1) / 3) << 8;
		colorTable[3] |= 0x00;
	}

	for (int pixelY = 0; pixelY < 4; pixelY++) {
		for (int pixelX = 0; pixelX < 4; pixelX++) {
			uint32_t i = pixelY * 4 + pixelX;
			uint32_t bitOffset = (15 - i) * 2;
			uint32_t di = i * 4;
			uint32_t si = (bits >> bitOffset) & 3;

			data[di + 0] = (colorTable[si] & 0xFF000000) >> 24;
			data[di + 1] = (colorTable[si] & 0x00FF0000) >> 16;
			data[di + 2] = (colorTable[si] & 0x0000FF00) >> 8;
			data[di + 3] = (colorTable[si] & 0x000000FF);
		}
	}

	return data;
}

}

namespace ImageFormat {

namespace Decode {
    void CMPR(bStream::CStream* stream, uint16_t width, uint16_t height, uint8_t* imageData){
        if (imageData == nullptr)
            return;

        uint32_t numBlocksW = (width + 7) / 8;
        uint32_t numBlocksH = (height + 7) / 8;

        // Iterate the blocks in the image
        for (int blockY = 0; blockY < numBlocksH; blockY++) {
            for (int blockX = 0; blockX < numBlocksW; blockX++) {
                // Each block has a set of 2x2 sub-blocks.
                for (int subBlockY = 0; subBlockY < 2; subBlockY++) {
                    for (int subBlockX = 0; subBlockX < 2; subBlockX++) {
                        uint32_t subBlockWidth = std::max(0, std::min(4, width - (subBlockX * 4 + blockX * 8)));
                        uint32_t subBlockHeight = std::max(0, std::min(4, height - (subBlockY * 4 + blockY * 8)));

                        uint8_t* subBlockData = ColorFormat::DecodeCMPRSubBlock(stream);

                        for (int pixelY = 0; pixelY < subBlockHeight; pixelY++) {
                            uint32_t destX = blockX * 8 + subBlockX * 4;
                            uint32_t destY = blockY * 8 + (subBlockY * 4) + pixelY;

                            if (destX >= width || destY >= height)
                                continue;

                            uint32_t destOffset = (destY * width + destX) * 4;
                            memcpy(imageData + destOffset, subBlockData + (pixelY * 4 * 4), subBlockWidth * 4);
                        }

                        delete[] subBlockData;
                    }
                }
            }
        }
    }

    void RGB5A3(bStream::CStream* stream, uint16_t width, uint16_t height, uint8_t* imageData){
        if (imageData == nullptr)
            return;

        uint32_t numBlocksW = width / 4;
        uint32_t numBlocksH = height / 4;

        // Iterate the blocks in the image
        for (int blockY = 0; blockY < numBlocksH; blockY++) {
            for (int blockX = 0; blockX < numBlocksW; blockX++) {
                // Iterate the pixels in the current block
                for (int pixelY = 0; pixelY < 4; pixelY++) {
                    for (int pixelX = 0; pixelX < 4; pixelX++) {
                        // Bounds check to ensure the pixel is within the image.
                        if ((blockX * 4 + pixelX >= width) || (blockY * 4 + pixelY >= height))
                            continue;

                        // RGB values for this pixel are stored in a 16-bit integer.
                        uint16_t data = stream->readUInt16();
                        uint32_t rgba8 = ColorFormat::RGB5A3toRGBA8(data);

                        uint32_t destIndex = (width * ((blockY * 4) + pixelY) + (blockX * 4) + pixelX) * 4;

                        imageData[destIndex] = (rgba8 & 0xFF000000) >> 24;
                        imageData[destIndex + 1] = (rgba8 & 0x00FF0000) >> 16;
                        imageData[destIndex + 2] = (rgba8 & 0x0000FF00) >> 8;
                        imageData[destIndex + 3] = rgba8 & 0x000000FF;
                    }
                }
            }
        }
    }

    void RGB565(bStream::CStream* stream, uint16_t width, uint16_t height, uint8_t* imageData){
        if (imageData == nullptr)
            return;

        uint32_t numBlocksW = width / 4;
        uint32_t numBlocksH = height / 4;

        // Iterate the blocks in the image
        for (int blockY = 0; blockY < numBlocksH; blockY++) {
            for (int blockX = 0; blockX < numBlocksW; blockX++) {
                // Iterate the pixels in the current block
                for (int pixelY = 0; pixelY < 4; pixelY++) {
                    for (int pixelX = 0; pixelX < 4; pixelX++) {
                        // Bounds check to ensure the pixel is within the image.
                        if ((blockX * 4 + pixelX >= width) || (blockY * 4 + pixelY >= height))
                            continue;

                        // RGB values for this pixel are stored in a 16-bit integer.
                        uint16_t data = stream->readUInt16();
                        uint32_t rgba8 = ColorFormat::RGB565toRGBA8(data);

                        uint32_t destIndex = (width * ((blockY * 4) + pixelY) + (blockX * 4) + pixelX) * 4;

                        imageData[destIndex] = (rgba8 & 0xFF000000) >> 24;
                        imageData[destIndex + 1] = (rgba8 & 0x00FF0000) >> 16;
                        imageData[destIndex + 2] = (rgba8 & 0x0000FF00) >> 8;
                        imageData[destIndex + 3] = rgba8 & 0x000000FF;
                    }
                }
            }
        }
    }

    void I4(bStream::CStream* stream, uint16_t width, uint16_t height, uint8_t* imageData){
        if(imageData == nullptr) return;

        uint32_t numBlocksW = width / 8;
        uint32_t numBlocksH = height / 8;
                
        // Iterate the blocks in the image
        for (int blockY = 0; blockY < numBlocksH; blockY++) {
            for (int blockX = 0; blockX < numBlocksW; blockX++) {
                // Iterate the pixels in the current block
                for (int pixelY = 0; pixelY < 8; pixelY++) {
                    for (int pixelX = 0; pixelX < 8; pixelX += 2) {
                        // Bounds check to ensure the pixel is within the image.
                        if ((blockX * 8 + pixelX >= width) || (blockY * 8 + pixelY >= height))
                            continue;

                        uint8_t data = stream->readUInt8();

                        // Each byte represents two pixels.
                        uint8_t pixel0 = (data & 0xF0) >> 4;
                        uint8_t pixel1 = (data & 0x0F);

                        uint32_t destIndex = (width * ((blockY * 8) + pixelY) + (blockX * 8) + pixelX) * 4;

                        imageData[destIndex] = pixel0 * 0x11;
                        imageData[destIndex + 1] = pixel0 * 0x11;
                        imageData[destIndex + 2] = pixel0 * 0x11;
                        imageData[destIndex + 3] = pixel0 * 0x11;

                        imageData[destIndex + 4] = pixel1 * 0x11;
                        imageData[destIndex + 5] = pixel1 * 0x11;
                        imageData[destIndex + 6] = pixel1 * 0x11;
                        imageData[destIndex + 7] = pixel1 * 0x11;
                    }
                }
            }
        }
    }

    void I8(bStream::CStream* stream, uint16_t width, uint16_t height, uint8_t* imageData){
        if(imageData == nullptr) return;

        uint32_t numBlocksW = width / 8;
        uint32_t numBlocksH = height / 4;

        // Iterate the blocks in the image
        for (int blockY = 0; blockY < numBlocksH; blockY++) {
            for (int blockX = 0; blockX < numBlocksW; blockX++) {
                // Iterate the pixels in the current block
                for (int pixelY = 0; pixelY < 4; pixelY++) {
                    for (int pixelX = 0; pixelX < 8; pixelX++) {
                        // Bounds check to ensure the pixel is within the image.
                        if ((blockX * 8 + pixelX >= width) || (blockY * 4 + pixelY >= height))
                            continue;

                        uint8_t data = stream->readUInt8();

                        uint32_t destIndex = (width * ((blockY * 4) + pixelY) + (blockX * 8) + pixelX) * 4;

                        imageData[destIndex] = data;
                        imageData[destIndex + 1] = data;
                        imageData[destIndex + 2] = data;
                        imageData[destIndex + 3] = data;
                    }
                }
            }
        }
    }

    void IA4(bStream::CStream* stream, uint16_t width, uint16_t height, uint8_t* imageData){
        if(imageData == nullptr) return;

        uint32_t numBlocksW = width / 8;
        uint32_t numBlocksH = height / 4;

        // Iterate the blocks in the image
        for (int blockY = 0; blockY < numBlocksH; blockY++) {
            for (int blockX = 0; blockX < numBlocksW; blockX++) {
                // Iterate the pixels in the current block
                for (int pixelY = 0; pixelY < 4; pixelY++) {
                    for (int pixelX = 0; pixelX < 8; pixelX++) {
                        // Bounds check to ensure the pixel is within the image.
                        if ((blockX * 8 + pixelX >= width) || (blockY * 4 + pixelY >= height))
                            continue;

                        uint8_t data = stream->readUInt8();

                        uint32_t destIndex = (width * ((blockY * 4) + pixelY) + (blockX * 8) + pixelX) * 4;

                        imageData[destIndex] = data & 0xF;
                        imageData[destIndex + 1] = data & 0xF;
                        imageData[destIndex + 2] = data & 0xF;
                        imageData[destIndex + 3] = (data >> 4) & 0xF;
                    }
                }
            }
        }
    }

    void IA8(bStream::CStream* stream, uint16_t width, uint16_t height, uint8_t* imageData){
        if(imageData == nullptr) return;

        uint32_t numBlocksW = width / 8;
        uint32_t numBlocksH = height / 4;

        // Iterate the blocks in the image
        for (int blockY = 0; blockY < numBlocksH; blockY++) {
            for (int blockX = 0; blockX < numBlocksW; blockX++) {
                // Iterate the pixels in the current block
                for (int pixelY = 0; pixelY < 4; pixelY++) {
                    for (int pixelX = 0; pixelX < 8; pixelX++) {
                        // Bounds check to ensure the pixel is within the image.
                        if ((blockX * 8 + pixelX >= width) || (blockY * 4 + pixelY >= height))
                            continue;

                        uint16_t data = stream->readUInt16();

                        uint32_t destIndex = (width * ((blockY * 4) + pixelY) + (blockX * 8) + pixelX) * 4;

                        imageData[destIndex] = data & 0x00FF;
                        imageData[destIndex + 1] = data & 0x00FF;
                        imageData[destIndex + 2] = data & 0x00FF;
                        imageData[destIndex + 3] = data & 0xFF00;
                    }
                }
            }
        }
    }
}

namespace Encode {
    void CMPR(bStream::CStream* stream, uint16_t width, uint16_t height, uint8_t* imageData){}
    void RGB5A3(bStream::CStream* stream, uint16_t width, uint16_t height, uint8_t* imageData){}
    void RGB565(bStream::CStream* stream, uint16_t width, uint16_t height, uint8_t* imageData){}

    void I4(bStream::CStream* stream, uint16_t width, uint16_t height, uint8_t* imageData){
        if(imageData == nullptr) return;

        uint32_t numBlocksW = width / 8;
        uint32_t numBlocksH = height / 8;
                
        // Iterate the blocks in the image
        for (int blockY = 0; blockY < numBlocksH; blockY++) {
            for (int blockX = 0; blockX < numBlocksW; blockX++) {
                // Iterate the pixels in the current block
                for (int pixelY = 0; pixelY < 8; pixelY++) {
                    for (int pixelX = 0; pixelX < 8; pixelX += 2) {
                        // Bounds check to ensure the pixel is within the image.
                        if ((blockX * 8 + pixelX >= width) || (blockY * 8 + pixelY >= height))
                            continue;

                        uint32_t srcIndex = (width * ((blockY * 4) + pixelY) + (blockX * 8) + pixelX) * 4;
                        uint8_t color1 = ColorFormat::RGBA8toI4(*reinterpret_cast<uint32_t*>(&imageData[srcIndex]));
                        uint8_t color2 = ColorFormat::RGBA8toI4(*(reinterpret_cast<uint32_t*>(&imageData[srcIndex]) + 1));

                        stream->writeUInt8((color1 & 0xF) << 4 | (color2 & 0xF));
                    }
                }
            }
        }
    }

    void IA4(bStream::CStream* stream, uint16_t width, uint16_t height, uint8_t* imageData){
        if(imageData == nullptr) return;

        uint32_t numBlocksW = width / 8;
        uint32_t numBlocksH = height / 4;

        // Iterate the blocks in the image
        for (int blockY = 0; blockY < numBlocksH; blockY++) {
            for (int blockX = 0; blockX < numBlocksW; blockX++) {
                // Iterate the pixels in the current block
                for (int pixelY = 0; pixelY < 4; pixelY++) {
                    for (int pixelX = 0; pixelX < 8; pixelX++) {
                        // Bounds check to ensure the pixel is within the image.
                        if ((blockX * 8 + pixelX >= width) || (blockY * 4 + pixelY >= height))
                            continue;

                        uint32_t srcIndex = (width * ((blockY * 4) + pixelY) + (blockX * 8) + pixelX) * 4;
                        uint32_t data = *reinterpret_cast<uint32_t*>(&imageData[srcIndex]);

                        stream->writeUInt8(ColorFormat::RGBA8toIA4(data));
                    }
                }
            }
        }
    }

    void I8(bStream::CStream* stream, uint16_t width, uint16_t height, uint8_t* imageData){
        if(imageData == nullptr) return;

        uint32_t numBlocksW = width / 8;
        uint32_t numBlocksH = height / 4;

        // Iterate the blocks in the image
        for (int blockY = 0; blockY < numBlocksH; blockY++) {
            for (int blockX = 0; blockX < numBlocksW; blockX++) {
                // Iterate the pixels in the current block
                for (int pixelY = 0; pixelY < 4; pixelY++) {
                    for (int pixelX = 0; pixelX < 8; pixelX++) {
                        // Bounds check to ensure the pixel is within the image.
                        if ((blockX * 8 + pixelX >= width) || (blockY * 4 + pixelY >= height))
                            continue;

                        uint32_t srcIndex = (width * ((blockY * 4) + pixelY) + (blockX * 8) + pixelX) * 4;
                        uint32_t data = *reinterpret_cast<uint32_t*>(&imageData[srcIndex]);

                        stream->writeUInt8(ColorFormat::RGBA8toI8(data));
                    }
                }
            }
        }
    }

    void IA8(bStream::CStream* stream, uint16_t width, uint16_t height, uint8_t* imageData){
        if(imageData == nullptr) return;

        uint32_t numBlocksW = width / 8;
        uint32_t numBlocksH = height / 4;

        // Iterate the blocks in the image
        for (int blockY = 0; blockY < numBlocksH; blockY++) {
            for (int blockX = 0; blockX < numBlocksW; blockX++) {
                // Iterate the pixels in the current block
                for (int pixelY = 0; pixelY < 4; pixelY++) {
                    for (int pixelX = 0; pixelX < 8; pixelX++) {
                        // Bounds check to ensure the pixel is within the image.
                        if ((blockX * 8 + pixelX >= width) || (blockY * 4 + pixelY >= height))
                            continue;

                        uint32_t srcIndex = (width * ((blockY * 4) + pixelY) + (blockX * 8) + pixelX) * 4;
                        uint32_t data = *reinterpret_cast<uint32_t*>(&imageData[srcIndex]);

                        stream->writeUInt16(ColorFormat::RGBA8toIA8(data));
                    }
                }
            }
        }
    }
}
};

void Bti::EncodeImage(bStream::CStream* stream, uint8_t* imageData){
    stream->writeUInt8(mFormat);
    stream->writeUInt8(mEnableAlpha);
    stream->writeUInt16(mWidth);
    stream->writeUInt16(mHeight);
    stream->writeUInt8(mWrapS);
    stream->writeUInt8(mWrapT);
    stream->writeUInt8(mPaletteFormat);
    stream->writeUInt8(mNumPaletteEntries);
    stream->writeUInt32(mPaletteOffsetData);
    stream->writeUInt8(mMipMapEnabled);
    stream->writeUInt8(mMipMapEnabled);
    stream->writeUInt8(mEdgeLODEnabled);
    stream->writeUInt8(mClampLODBias);
    stream->writeUInt8(mMaxAnisotropy);
    stream->writeUInt8(mMinFilterType);
    stream->writeUInt8(mMagFilterType);
    stream->writeUInt8(mMinLOD);
    stream->writeUInt8(mMaxLOD);
    stream->writeUInt8(mNumImages); // should be 1?
    stream->writeUInt8(0);
    stream->writeUInt16(mLODBias);

    stream->writeUInt32(stream->tell() + sizeof(uint32_t));

    switch (mFormat){
    case 0x00:
        ImageFormat::Encode::I4(stream, mWidth, mHeight, imageData);
        break;
    case 0x01:
        ImageFormat::Encode::I8(stream, mWidth, mHeight, imageData);
        break;
    case 0x02:
        ImageFormat::Encode::IA4(stream, mWidth, mHeight, imageData);
        break;
    case 0x03:
        ImageFormat::Encode::IA8(stream, mWidth, mHeight, imageData);
        break;
    case 0x04:
        //ImageFormat::Encode::RGB565(stream, mWidth, mHeight, imageData);
        break;
    case 0x05:
        //ImageFormat::Encode::RGB5A3(stream, mWidth, mHeight, imageData);
        break;
    case 0x0E:
        //ImageFormat::Encode::CMPR(stream, mWidth, mHeight, imageData);
        break;
    default:
        break;
    }
}

uint8_t* Bti::DecodeImage(bStream::CStream* stream){
    mFormat = stream->readUInt8();
    mEnableAlpha = stream->readUInt8();

    mWidth = stream->readUInt16();
    mHeight = stream->readUInt16();

    mWrapS = stream->readUInt8();
    mWrapT = stream->readUInt8();
    mPaletteFormat = stream->readUInt8();
    mNumPaletteEntries = stream->readUInt8();
    mPaletteOffsetData = stream->readUInt32();
    mMipMapEnabled = stream->readUInt8();
    mEdgeLODEnabled = stream->readUInt8();
    mClampLODBias = stream->readUInt8();
    mMaxAnisotropy = stream->readUInt8();
    mMinFilterType = stream->readUInt8();
    mMagFilterType = stream->readUInt8();
    mMinLOD = stream->readUInt8();
    mMaxLOD = stream->readUInt8();
    mNumImages = stream->readUInt8();

    stream->skip(1);

    mLODBias = stream->readUInt16();
    uint32_t imageDataOffset = stream->readUInt32();


    stream->seek(imageDataOffset);

    uint8_t* imageData = new uint8_t[mWidth * mHeight * 4](0);

    switch (mFormat){
    case 0x00:
        ImageFormat::Decode::I4(stream, mWidth, mHeight, imageData);
        break;
    case 0x01:
        ImageFormat::Decode::I8(stream, mWidth, mHeight, imageData);
        break;
    case 0x02:
        ImageFormat::Decode::IA4(stream, mWidth, mHeight, imageData);
        break;
    case 0x03:
        ImageFormat::Decode::IA8(stream, mWidth, mHeight, imageData);
        break;
    case 0x04:
        ImageFormat::Decode::RGB565(stream, mWidth, mHeight, imageData);
        break;
    case 0x05:
        ImageFormat::Decode::RGB5A3(stream, mWidth, mHeight, imageData);
        break;
    case 0x0E:
        ImageFormat::Decode::CMPR(stream, mWidth, mHeight, imageData);
        break;
    default:
        break;
    }

    return imageData;
}
