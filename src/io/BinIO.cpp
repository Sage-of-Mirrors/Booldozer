#include <vector>
#include "glm/glm.hpp"
#include "glm/gtx/quaternion.hpp"
#include "io/BinIO.hpp"
#include <glad/glad.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <J3D/J3DTexture.hpp>
#include <geometry/GXGeometryEnums.hpp>

/*
*
* This file is full of hacks and duplicated code. I would like to do it properly later but for the time being, this works fine.
*
*/

uint32_t mProgramID = -1; // on setup handle this

uint32_t BinMaterial::RGB565toRGBA8(uint16_t data) {
	uint8_t r = (data & 0xF100) >> 11;
	uint8_t g = (data & 0x07E0) >> 5;
	uint8_t b = (data & 0x001F);

	uint32_t output = 0x000000FF;
	output |= (r << 3) << 24;
	output |= (g << 2) << 16;
	output |= (b << 3) << 8;

	return output;
}

uint32_t BinMaterial::RGB5A3toRGBA8(uint16_t data) {
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

void BinMaterial::DecodeCMPR(bStream::CStream* stream, uint16_t width, uint16_t height, uint8_t* imageData) {
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

					uint8_t* subBlockData = BinMaterial::DecodeCMPRSubBlock(stream);

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

uint8_t* BinMaterial::DecodeCMPRSubBlock(bStream::CStream* stream) {
	uint8_t* data = new uint8_t[4 * 4 * 4]{};

	uint16_t color0 = stream->readUInt16();
	uint16_t color1 = stream->readUInt16();
	uint32_t bits = stream->readUInt32();

	uint32_t colorTable[4]{};
	colorTable[0] = BinMaterial::RGB565toRGBA8(color0);
	colorTable[1] = BinMaterial::RGB565toRGBA8(color1);

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

void BinMaterial::DecodeRGB565(bStream::CStream* stream, uint16_t width, uint16_t height, uint8_t* imageData) {
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
					uint32_t rgba8 = BinMaterial::RGB565toRGBA8(data);

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

void BinMaterial::DecodeRGB5A3(bStream::CStream* stream, uint16_t width, uint16_t height, uint8_t* imageData) {
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
					uint32_t rgba8 = BinMaterial::RGB5A3toRGBA8(data);

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

const char* default_vtx_shader_source = "#version 460\n\
    #extension GL_ARB_separate_shader_objects : enable\n\
	struct GXLight {\n\
		vec4 Position;\n\
		vec4 Direction;\n\
		vec4 Color;\n\
		vec4 AngleAtten;\n\
		vec4 DistAtten;\n\
	};\n\
    layout (std140, binding=0) uniform uSharedData {\n\
		mat4 Proj;\n\
		mat4 View;\n\
		mat4 Model;\n\
		vec4 TevColor[4];\n\
		vec4 KonstColor[4];\n\
		GXLight Lights[8];\n\
		mat4 Envelopes[512];\n\
		mat4 TexMatrices[10];\n\
    };\n\
    uniform mat4 transform;\n\
    \
    layout(location = 0) in vec3 inPosition;\n\
    layout(location = 1) in vec2 inTexCoord;\n\
    \
    layout(location = 0) out vec2 fragTexCoord;\n\
    \
    void main()\n\
    {\
        gl_Position = Proj * View * transform * vec4(inPosition, 1.0);\n\
        fragTexCoord = inTexCoord;\n\
    }\
";

const char* default_frg_shader_source = "#version 460\n\
    #extension GL_ARB_separate_shader_objects : enable\n\
    \
    uniform sampler2D texSampler;\n\
    uniform vec4 binMatColor;\n\
    layout(location = 0) in vec2 fragTexCoord;\n\
    \
    layout(location = 0) out vec4 outColor;\n\
    \
    void main()\n\
    {\n\
        vec4 baseColor = texture(texSampler, vec2(fragTexCoord.y, fragTexCoord.x));\n\
        outColor = baseColor * binMatColor;//vec4(1.0, 1.0, 1.0, 1.0);\n\
        if(baseColor.a < 1.0 / 255.0) discard;\n\
    }\
";

struct BinVertex
{
	float x;
	float y;
	float z;
	float u;
	float v;
};

std::vector<BinVertex> ReadGXPrimitives(bStream::CStream* stream, std::vector<glm::vec3>& vertices, std::vector<glm::vec2>& texcoords, std::vector<GXAttribute>& attributes, bool nbt, uint32_t listSize){
    std::vector<BinVertex> vd_out;

    uint8_t primitiveType = stream->readUInt8(); 
    while (stream->tell() < listSize && (EGXPrimitiveType)primitiveType != EGXPrimitiveType::None)
    {
        uint16_t count = stream->readUInt16();
        std::vector<std::pair<uint16_t, uint16_t>> primVertices(count);

        //Read Primitives
        for (size_t v = 0; v < count; v++)
        {
            for(auto& attribute : attributes)
            {
                switch (attribute)
                {
                case GXAttribute::Position:
                    primVertices[v].first = stream->readUInt16();
                    break;
                case GXAttribute::Tex0:
                    primVertices[v].second = stream->readUInt16();
                    break;
                case GXAttribute::Normal:
                    stream->readUInt16();
                    if(nbt){
                        stream->readUInt16();
                        stream->readUInt16();
                    }
                    break;
                default:
                    stream->readUInt16(); //TODO: NBT Fix
                    break;
                }
            }
        }

        for (auto& vtx : primVertices)
        {
            if(vtx.first > vertices.size() || vtx.second > texcoords.size()){
                std::cout << "Error Loading Model! Primitives are wrong? Vertex " << vtx.first << " out of range " << vertices.size() << " or TexCoord " << vtx.second << " out of range " << texcoords.size() << std::endl;
                return vd_out;
            }
        }
        

        //Triangulate
        BinVertex vtx;
        glm::vec3 pos;
        glm::vec2 texcoord;
        switch (primitiveType)
        {
        case Triangles:

            for (size_t v = 0; v < count; v++)
            {
                pos = vertices.at(primVertices.at(v).first);
                texcoord = texcoords.at(primVertices.at(v).second);
                vtx.x = pos.x;
                vtx.y = pos.y;
                vtx.z = pos.z;
                vtx.u = texcoord.y;
                vtx.v = texcoord.x;
                vd_out.push_back(vtx);
            }
            break;
        
        case TriangleStrip:
            for (size_t v = 2; v < count; v++)
            {

                pos = vertices.at(primVertices.at(v - 2).first);
                texcoord = texcoords.at(primVertices.at(v - 2).second);
                vtx.x = pos.x;
                vtx.y = pos.y;
                vtx.z = pos.z;
                vtx.u = texcoord.x;
                vtx.v = texcoord.y;
                vd_out.push_back(vtx);

                pos = vertices.at((v % 2 != 0 ? primVertices.at(v) : primVertices.at(v - 1)).first);
                texcoord = texcoords.at((v % 2 != 0 ? primVertices.at(v) : primVertices.at(v - 1)).second);
                vtx.x = pos.x;
                vtx.y = pos.y;
                vtx.z = pos.z;
                vtx.u = texcoord.x;
                vtx.v = texcoord.y;
                vd_out.push_back(vtx);

                pos = vertices.at((v % 2 != 0 ? primVertices.at(v - 1) : primVertices.at(v)).first);
                texcoord = texcoords.at((v % 2 != 0 ? primVertices.at(v - 1) : primVertices.at(v)).second);
                vtx.x = pos.x;
                vtx.y = pos.y;
                vtx.z = pos.z;
                vtx.u = texcoord.x;
                vtx.v = texcoord.y;
                vd_out.push_back(vtx);

            }
            break;

        default:
            break;
        }

        primitiveType = stream->readUInt8();
    }

    return vd_out;  
}


BinMesh::BinMesh(bStream::CStream* stream, uint32_t offset, std::vector<glm::vec3>& vertexData, std::vector<glm::vec2>& texcoordData){
    
    stream->skip(2);
    uint32_t listSize = stream->readUInt16() << 5;
    uint32_t attr = stream->readUInt32();

    std::vector<GXAttribute> attributes;
    uint32_t mask = 1;

    for (size_t i = 0; i < 26; i++)
    {
        if((attr & mask) >> i)
        {
            attributes.push_back((GXAttribute)i);
        }

        mask <<= 1;
    }
    
    stream->skip(3);
    
    uint8_t useNbt = stream->readUInt8();
    uint32_t primitiveOffset = stream->readUInt32();
    uint32_t ret = stream->tell();

    stream->seek(offset + primitiveOffset);
    ////std::cout << "Reading Primitives at " << std::hex << stream->tell() << std::endl;
    
    std::vector<BinVertex> buffer = ReadGXPrimitives(stream, vertexData, texcoordData, attributes, useNbt, listSize + offset + primitiveOffset);

    mVertexCount = buffer.size();

    glGenVertexArrays(1, &mVao);
    glBindVertexArray(mVao);

    glGenBuffers(1, &mVbo);
    glBindBuffer(GL_ARRAY_BUFFER, mVbo);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(BinVertex), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(BinVertex), (void*)12);

    glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(BinVertex), buffer.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    stream->seek(ret);
}

void BinMesh::Bind(){
    glBindVertexArray(mVao);
}

BinMesh::~BinMesh(){
    glDeleteVertexArrays(1, &mVao);
    glDeleteBuffers(1, &mVbo);
}

///
/// Materials and Textures
///



void BinMaterial::Bind(){
    //TODO: wrap modes
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mTexture);
}

BinMaterial::~BinMaterial(){
    glDeleteTextures(1, &mTexture);
}

BinMaterial::BinMaterial(bStream::CStream* stream, uint32_t textureOffset){
    ////std::cout << "Reading Material at " << std::hex << stream->tell() << std::endl;
    int16_t textureID = stream->readInt16();
    if(textureID == -1) return;

    stream->skip(2);
    uint8_t wu = stream->readUInt8();
    uint8_t wv = stream->readUInt8();

    stream->seek(textureOffset + (textureID * 0xC));
    uint16_t w = stream->readUInt16();
    uint16_t h = stream->readUInt16();

    uint8_t format = stream->readUInt8();
    stream->skip(3);
    uint32_t dataOffset = stream->readUInt32() + textureOffset;
    stream->seek(dataOffset);


    uint8_t* textureData = new uint8_t[w*h*4]{};

	switch ((EGXTextureFormat)format) {
		case EGXTextureFormat::RGB565:
			//DecodeRGB565(stream, w, h, textureData);
            BinMaterial::DecodeRGB565(stream, w, h, textureData);
			break;
		case EGXTextureFormat::RGB5A3:
			//DecodeRGB5A3(stream, w, h, textureData);
            BinMaterial::DecodeRGB5A3(stream, w, h, textureData);
			break;
		case EGXTextureFormat::CMPR:
			BinMaterial::DecodeCMPR(stream, w, h, textureData);
			break;
	}

    glGenTextures(1, &mTexture);
    glBindTexture(GL_TEXTURE_2D, mTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (wu == 2 ? GL_MIRRORED_REPEAT : GL_REPEAT));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (wu == 2 ? GL_MIRRORED_REPEAT : GL_REPEAT));

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
    
    glBindTexture(GL_TEXTURE_2D, 0);

    delete textureData;

}

BinSampler::BinSampler(bStream::CStream* stream){
    stream->readInt16();
    stream->readUInt8();
    uint32_t color = stream->readUInt32();
    mAmbientColor = glm::vec4((color & 0x000000FF) / 255.0f, (color & 0x0000FF) / 255.0f, (color & 0x00FF) / 255.0f, (color & 0xFF) / 255.0f);
    stream->readUInt8();
    mTextureID = stream->readUInt16();
}

///
/// Scenegraph Node
///

BinScenegraphNode::BinScenegraphNode(){}
BinScenegraphNode::~BinScenegraphNode(){}


void BinScenegraphNode::AddMesh(int16_t material, int16_t mesh){
    meshes.push_back(std::pair(material, mesh));
}

void BinScenegraphNode::Draw(glm::mat4 localTransform, glm::mat4* instance, BinModel* bin, bool bIgnoreTransforms){
    if(!bIgnoreTransforms){
        glUniformMatrix4fv(glGetUniformLocation(mProgramID, "transform"), 1, 0, &(*instance * localTransform * transform)[0][0]);
    } else {
        glUniformMatrix4fv(glGetUniformLocation(mProgramID, "transform"), 1, 0, &(*instance)[0][0]);
    }
    for (auto& mesh : meshes)
    {
        bin->BindMesh(mesh.first);
        bin->BindMaterial(mesh.second);

        glUniform4fv(glGetUniformLocation(mProgramID, "binMatColor"), 1, &bin->GetSampler(mesh.second)->mAmbientColor[0]);

        glDrawArrays(GL_TRIANGLES, 0, bin->GetMesh(mesh.first)->mVertexCount);

        glBindVertexArray(0);
    }
    
    if(child != nullptr){
        child->Draw(localTransform * transform, instance, bin, bIgnoreTransforms);
    }

    if(next != nullptr){
        next->Draw(localTransform, instance, bin, bIgnoreTransforms);
    }
}

bool BinModel::BindMesh(uint16_t id){
    if(mMeshes.count(id) != 0){
        mMeshes[id]->Bind();
        return true;
    } else {
        return false;
    }
}

bool BinModel::BindMaterial(uint16_t id){
    if(mMaterials.size() > mSamplers[id]->mTextureID){
        mMaterials[mSamplers[id]->mTextureID]->Bind();
        return true;
    } else {
        //std::cout << "Couldn't bind material " << mSamplers[id]->mTextureID << std::endl;
        return false;
    }
}

BinModel::BinModel(bStream::CStream* stream){
    
    uint32_t chunkOffsets[21];
    stream->seek(12);

    for (size_t o = 0; o < 21; o++)
    {
        chunkOffsets[o] = stream->readUInt32();
    }


    uint32_t vertexCount = 0;

    for(size_t o = 3; o < 21; o++)
    {
        vertexCount = (uint32_t)((chunkOffsets[o] - chunkOffsets[2]) / 6) + 5;
        if(chunkOffsets[o] != 0) break;
    }

    uint32_t texcoordCount = 0; //(uint32_t)((chunkOffsets[10] - chunkOffsets[6]) / 8);
    for(size_t o = 7; o < 21; o++)
    {
        texcoordCount = (uint32_t)((chunkOffsets[o] - chunkOffsets[6]) / 8);
        if(chunkOffsets[o] != 0) break;
    }

    uint32_t material_count = (uint32_t)((chunkOffsets[2] - chunkOffsets[1]) / 0x14);
    
    if(chunkOffsets[1] == 0) material_count = 0; //?

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> texcoords;
    
    stream->seek(chunkOffsets[2]);
    for (size_t v = 0; v < vertexCount; v++)
    {
        vertices.push_back(glm::vec3(stream->readInt16(), stream->readInt16(), stream->readInt16()));
    }
    
    stream->seek(chunkOffsets[6]);
    for (size_t tc = 0; tc < texcoordCount; tc++)
    {
        texcoords.push_back(glm::vec2(stream->readFloat(), stream->readFloat()));
    }

    for (size_t m = 0; m < material_count; m++)
    {
        stream->seek(chunkOffsets[1] + (0x14 * m));
        mMaterials.push_back(std::make_shared<BinMaterial>(stream, chunkOffsets[0]));
    }
    
    mRoot = ParseSceneraph(stream, chunkOffsets, 0, vertices, texcoords);
}

std::shared_ptr<BinScenegraphNode> BinModel::ParseSceneraph(bStream::CStream* stream, uint32_t* offsets, uint16_t index, std::vector<glm::vec3>& vertexData, std::vector<glm::vec2>& texcoordData, std::shared_ptr<BinScenegraphNode> parent, std::shared_ptr<BinScenegraphNode> previous){
    stream->seek(offsets[12] + (0x8C * index));
    std::shared_ptr<BinScenegraphNode> current = std::make_shared<BinScenegraphNode>();
    //skip 4 uint16 indices, 1 byte padding, 1 byte render flags, 1 uint16 padding
    int16_t parentIndex = stream->readInt16();
    int16_t childIndex = stream->readInt16();
    int16_t nextIndex = stream->readInt16();
    int16_t prevIndex = stream->readInt16();

    if(previous != nullptr){
        current->prev = previous;
    }

    if(parent != nullptr){
        current->parent = parent;
    }

    stream->skip(4);
    current->transform = glm::identity<glm::mat4>();
    current->transform = glm::scale(current->transform, glm::vec3(stream->readFloat(), stream->readFloat(), stream->readFloat())); 
    glm::vec3 rotation(stream->readFloat(), stream->readFloat(), stream->readFloat());
    glm::quat rotationQuat = glm::angleAxis(glm::radians(rotation.x), glm::vec3(1.0,0.0,0.0)) * glm::angleAxis(-glm::radians(rotation.y), glm::vec3(0.0,1.0,0.0)) * glm::angleAxis(glm::radians(rotation.z), glm::vec3(0.0,0.0,1.0));
    current->transform *= glm::toMat4(rotationQuat);
    current->transform = glm::translate(current->transform, glm::vec3(stream->readFloat(), stream->readFloat(), stream->readFloat()));

    stream->skip(4*7);

    uint16_t meshCount = stream->readUInt16();
    stream->skip(2); // Skip padding
    uint32_t meshOffset = stream->readUInt32();
    stream->seek(offsets[12] + meshOffset);

    for (size_t m = 0; m < meshCount; m++)
    {
        int16_t matIndex = stream->readInt16();
        int16_t meshIndex = stream->readInt16();
        
        if(mMeshes.count(meshIndex) == 0){
            size_t r = stream->tell();
            stream->seek(offsets[11] + 0x18 * meshIndex);

            mMeshes[meshIndex] = std::make_shared<BinMesh>(stream, offsets[11], vertexData, texcoordData);
            
            stream->seek(r);
        }

        if(mSamplers.count(matIndex) == 0){
            size_t r = stream->tell();

            stream->seek(offsets[10] + 0x28 * matIndex);

            mSamplers[matIndex] = std::make_shared<BinSampler>(stream);
            
            stream->seek(r);
        }

        current->AddMesh(meshIndex, matIndex);
    }

    if(childIndex != -1){
        current->child = ParseSceneraph(stream, offsets, childIndex, vertexData, texcoordData, current);
    }
    
    if(nextIndex != -1){
        current->next = ParseSceneraph(stream, offsets, nextIndex, vertexData, texcoordData, nullptr, current);
    }

    return current;
    
}

void BinModel::TranslateRoot(glm::vec3 translation){
    mRoot->transform = glm::translate(mRoot->transform, translation);
}

void BinModel::Draw(glm::mat4* transform, bool bIgnoreTransforms){
    glFrontFace(GL_CW);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glUseProgram(mProgramID);
    mRoot->Draw(glm::identity<glm::mat4>(), transform, this, bIgnoreTransforms);
}

BinModel::~BinModel(){

}

void BinModel::DestroyShaders(){
    glDeleteProgram(mProgramID);
}


void BinModel::InitShaders(){
    std::cout << "Compiling shaders for bin?" << std::endl; 
    	//Compile Shaders
    char glErrorLogBuffer[4096];
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vs, 1, &default_vtx_shader_source, NULL);
    glShaderSource(fs, 1, &default_frg_shader_source, NULL);

    glCompileShader(vs);

    GLint status;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
    if(status == GL_FALSE){
        GLint infoLogLength;
        glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &infoLogLength);

        glGetShaderInfoLog(vs, infoLogLength, NULL, glErrorLogBuffer);

        printf("Compile failure in vertex shader:\n%s\n", glErrorLogBuffer);
    }

    glCompileShader(fs);

    glGetShaderiv(fs, GL_COMPILE_STATUS, &status);
    if(status == GL_FALSE){
        GLint infoLogLength;
        glGetShaderiv(fs, GL_INFO_LOG_LENGTH, &infoLogLength);

        glGetShaderInfoLog(fs, infoLogLength, NULL, glErrorLogBuffer);

        printf("Compile failure in fragment shader:\n%s\n", glErrorLogBuffer);
    }

    mProgramID = glCreateProgram();

    glAttachShader(mProgramID, vs);
    glAttachShader(mProgramID, fs);

    glLinkProgram(mProgramID);

    glGetProgramiv(mProgramID, GL_LINK_STATUS, &status); 
    if(GL_FALSE == status) {
        GLint logLen; 
        glGetProgramiv(mProgramID, GL_INFO_LOG_LENGTH, &logLen); 
        glGetProgramInfoLog(mProgramID, logLen, NULL, glErrorLogBuffer); 
        printf("Shader Program Linking Error:\n%s\n", glErrorLogBuffer);
    } 

    glDetachShader(mProgramID, vs);
    glDetachShader(mProgramID, fs);

    glDeleteShader(vs);
    glDeleteShader(fs);

}