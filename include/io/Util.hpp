#pragma once
#include <vector>
#include <type_traits>
#include <GenUtil.hpp>
#include <bstream.h>

enum class GXAttribute : int {
	PositionMatrixIndex,
	Tex0MatrixIndex,
	Tex1MatrixIndex,
	Tex2MatrixIndex,
	Tex3MatrixIndex,
	Tex4MatrixIndex,
	Tex5MatrixIndex,
	Tex6MatrixIndex,
	Tex7MatrixIndex,
	Position,
	Normal,
	Color0,
	Color1,
	Tex0,
	Tex1,
	Tex2,
	Tex3,
	Tex4,
	Tex5,
	Tex6,
	Tex7,
	PositionMatrixArray,
	NormalMatrixArray,
	TextureMatrixArray,
	LitMatrixArray,
	NormalBinormalTangent,
	NullAttr = 0xFF
};

template<GXAttribute attr>
bool HasAttribute(uint32_t attributes){
    return (attributes & (1 << static_cast<uint32_t>(attr))) != 0;
}

enum GXPrimitiveType {
	Points = 0xB8,
	Lines = 0xA8,
	LineStrip = 0xB0,
	Triangles = 0x90,
	TriangleStrip = 0x98,
	TriangleFan = 0xA0,
	Quads = 0x80,
	PrimitiveNone = 0x00
};

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec3 Binormal;
    glm::vec3 Tangent;
    glm::vec4 Color { 1.0f, 1.0f, 1.0f, 1.0f };
    glm::vec2 Texcoord;
	glm::vec2 Texcoord1;
};

struct PrimitiveVertex {
    int8_t Matrix;
    int16_t Position;
    int16_t Normal;
    int16_t Color;
    int16_t Texcoord;
	int16_t Texcoord1;
};

struct Readable {
    virtual void Read(bStream::CStream* stream) = 0;
    virtual ~Readable(){}
};
