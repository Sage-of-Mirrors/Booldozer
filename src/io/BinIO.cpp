#include <cstdint>
#include <filesystem>
#include <iterator>
#include <numeric>
#include <vector>
#include "GXVertexData.hpp"
#include "GenUtil.hpp"
#include "Util.hpp"
#include "bstream.h"
#include "constants.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/glm.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/trigonometric.hpp"
#include "io/Util.hpp"
#include "io/BinIO.hpp"
#include <glad/glad.h>
#include <J3D/Texture/J3DTexture.hpp>
#include <GXGeometryEnums.hpp>
#include <Bti.hpp>
#include <format>
#include "io/KeyframeIO.hpp"
#include "stb_image.h"

namespace BIN {
    // from https://github.com/Sage-of-Mirrors/libjstudio/blob/main/src/engine/value/interpolation.cpp
    float InterpolateHermite(float factor, float timeA, float valueA, float outTangent, float timeB, float valueB, float inTangent){
    	float a = factor - timeA;
    	float b = a * (1.0f / (timeB - timeA));
    	float c = b - 1.0f;
    	float d = (3.0f + -2.0f * b) * (b * b);

    	float cab = c * a * b;
    	float coeffx3 = cab * inTangent;
    	float cca = c * c * a;
    	float coeffc2 = cca * outTangent;

    	return b * c * a * inTangent + a * c * c * outTangent + (1.0f - d) * valueA + d * valueB;
    }

    float MixTrack(LTrackCommon* track, uint32_t frameCount, float curFrame, uint32_t& mNextKey){
        if(mNextKey < track->mKeys.size()){
            float v = InterpolateHermite(
                (uint32_t)(((uint32_t)curFrame - track->mFrames[track->mKeys[mNextKey - 1]].frame) / (track->mFrames[track->mKeys[mNextKey]].frame - track->mFrames[track->mKeys[mNextKey - 1]].frame)),
                track->mFrames[track->mKeys[mNextKey - 1]].frame,
                track->mFrames[track->mKeys[mNextKey - 1]].value,
                track->mFrames[track->mKeys[mNextKey - 1]].outslope,
                track->mFrames[track->mKeys[mNextKey]].frame,
                track->mFrames[track->mKeys[mNextKey]].value,
                track->mFrames[track->mKeys[mNextKey]].inslope
            );

            if(std::floor(curFrame) >= track->mFrames[track->mKeys[mNextKey]].frame){
                mNextKey += 1;
            }
            return v;
        } else {
            return track->mFrames[track->mKeys[mNextKey - 1]].value;
        }
    }

    void SceneGraphNode::Read(bStream::CStream* stream){
        ParentIndex = stream->readInt16();
        ChildIndex = stream->readInt16();
        NextSibIndex = stream->readInt16();
        PreviousSibIndex = stream->readInt16();

        stream->skip(1);
        RenderFlags = stream->readUInt8();
        stream->skip(2);

        Scale = { stream->readFloat(), stream->readFloat(), stream->readFloat() };
        Rotation = { glm::radians(stream->readFloat() * 0.0001533981f), glm::radians(stream->readFloat() * 0.0001533981f), glm::radians(stream->readFloat() * 0.0001533981f) };
        Position = { stream->readFloat(), stream->readFloat(), stream->readFloat() };

        BoundingBoxMin = { stream->readFloat(), stream->readFloat(), stream->readFloat() };
        BoundingBoxMax = { stream->readFloat(), stream->readFloat(), stream->readFloat() };
        Radius = stream->readFloat();

        ElementCount = stream->readUInt16();
        stream->skip(2);
        ElementOffset = stream->readUInt32();
    }

    void SceneGraphNode::Write(bStream::CStream* stream){
        stream->writeInt16(ParentIndex);
        stream->writeInt16(ChildIndex);
        stream->writeInt16(NextSibIndex);
        stream->writeInt16(PreviousSibIndex);
        stream->writeUInt8(0);
        stream->writeUInt8(RenderFlags);
        stream->writeUInt16(0);

        stream->writeFloat(Scale.x);
        stream->writeFloat(Scale.y);
        stream->writeFloat(Scale.z);

        stream->writeFloat(glm::degrees(Rotation.x) / 0.0001533981f);
        stream->writeFloat(glm::degrees(Rotation.y) / 0.0001533981f);
        stream->writeFloat(glm::degrees(Rotation.z) / 0.0001533981f);

        stream->writeFloat(Position.x);
        stream->writeFloat(Position.y);
        stream->writeFloat(Position.z);

        stream->writeFloat(BoundingBoxMin.x);
        stream->writeFloat(BoundingBoxMin.y);
        stream->writeFloat(BoundingBoxMin.z);

        stream->writeFloat(BoundingBoxMax.x);
        stream->writeFloat(BoundingBoxMax.y);
        stream->writeFloat(BoundingBoxMax.z);

        stream->writeFloat(Radius);

        stream->writeUInt16(mDrawElements.size());
        stream->writeUInt16(0);
        stream->writeUInt32(0);

        for(int i = 0; i < 14; i++) stream->writeUInt32(0);
    }

    void Sampler::Read(bStream::CStream* stream){
        TextureIndex = stream->readInt16();
        PaletteIndex = stream->readInt16();
        WrapU = stream->readUInt8();
        WrapV = stream->readUInt8();
        Unk = stream->readInt16();
    }

    void Sampler::Write(bStream::CStream* stream){
        stream->writeInt16(TextureIndex);
        stream->writeInt16(PaletteIndex);
        stream->writeUInt8(WrapU);
        stream->writeUInt8(WrapV);
        stream->writeInt16(Unk);
        for(int i = 0; i < 3; i++) stream->writeUInt32(0);
    }

    void Material::Read(bStream::CStream* stream){
        LightEnabled = stream->readUInt8();
        Unk0 = stream->readUInt8();
        Unk1 = stream->readUInt8();
        Color = { stream->readUInt8() / 0xFF, stream->readUInt8() / 0xFF, stream->readUInt8() / 0xFF, stream->readUInt8() / 0xFF };
        stream->skip(1);

        for(int i = 0; i < 8; i++){
            SamplerIndices[i] = stream->readInt16();
        }
    }

    void Material::Write(bStream::CStream* stream){
        stream->writeUInt8(LightEnabled);
        stream->writeUInt8(Unk0);
        stream->writeUInt8(Unk1);
        stream->writeUInt32(static_cast<uint8_t>(Color.a * 255) | (static_cast<uint8_t>(Color.b * 255) << 8) | (static_cast<uint8_t>(Color.g * 255) << 16) | (static_cast<uint8_t>(Color.r * 255) << 24));
        stream->writeUInt8(0);
        for(int i = 0; i < 8; i++){
            stream->writeInt16(SamplerIndices[i]);
        }
        for(int i = 0; i < 8; i++){
            stream->writeInt16(-1);
        }
    }

    void DrawElement::Read(bStream::CStream* stream){
        MaterialIndex = stream->readInt16();
        BatchIndex = stream->readInt16();
    }

    void Batch::Read(bStream::CStream* stream){
        TriangleCount = stream->readUInt16();
        DisplayListSize = stream->readUInt16();
        VertexAttributes = stream->readUInt32();
        NormalFlag = stream->readUInt8();
        PositionFlag = stream->readUInt8();
        TexCoordFlag = stream->readUInt8();
        NBTFlag = stream->readUInt8();
        PrimitiveOffset = stream->readUInt32();
    }

    void Batch::Write(bStream::CStream* stream){
        stream->writeUInt16(TriangleCount);
        stream->writeUInt16(DisplayListSize);
        stream->writeUInt32(VertexAttributes);
        stream->writeUInt8(NormalFlag);
        stream->writeUInt8(PositionFlag);
        stream->writeUInt8(TexCoordFlag);
        stream->writeUInt8(NBTFlag);
        stream->writeUInt32(0);
        stream->writeUInt32(0);
        stream->writeUInt32(0);
    }

    void TextureHeader::Read(bStream::CStream* stream){
        Width = stream->readUInt16();
        Height = stream->readUInt16();
        Format = stream->readUInt8();
        Unknown = stream->readUInt16();
        stream->readUInt8();

        ImageOffset = stream->readUInt32();
    }

    void TextureHeader::Write(bStream::CStream* stream){
        stream->writeUInt16(Width);
        stream->writeUInt16(Height);
        stream->writeUInt8(Format);
        stream->writeUInt16(Unknown);
        stream->writeUInt8(0);
        stream->writeUInt32(0);
    }

    void TextureHeader::Load(bStream::CStream* stream){
        ImageData = new uint8_t[Width*Height*4];
        switch (Format) {
            case 0x0E:
                ImageFormat::Decode::CMPR(stream, Width, Height, ImageData);
                break;
            case 0x05:
                ImageFormat::Decode::RGB5A3(stream, Width, Height, ImageData);
                break;
            case 0x04:
                ImageFormat::Decode::RGB565(stream, Width, Height, ImageData);
                break;
            case 0x03:
                ImageFormat::Decode::IA8(stream, Width, Height, ImageData);
                break;
            case 0x02:
                ImageFormat::Decode::IA4(stream, Width, Height, ImageData);
                break;
            case 0x01:
                ImageFormat::Decode::I8(stream, Width, Height, ImageData);
                break;
            case 0x00:
                ImageFormat::Decode::I4(stream, Width, Height, ImageData);
                break;
        }
        glGenTextures(1, &TextureID);
        glBindTexture(GL_TEXTURE_2D, TextureID);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, ImageData);

        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void TextureHeader::Save(bStream::CStream* stream){
        switch (Format) {
            case 0x0E:
                ImageFormat::Encode::CMPR(stream, Width, Height, ImageData);
                break;
            case 0x05:
                ImageFormat::Encode::RGB5A3(stream, Width, Height, ImageData);
                break;
            case 0x04:
                ImageFormat::Encode::RGB565(stream, Width, Height, ImageData);
                break;
            case 0x03:
                ImageFormat::Encode::IA8(stream, Width, Height, ImageData);
                break;
            case 0x02:
                ImageFormat::Encode::IA4(stream, Width, Height, ImageData);
                break;
            case 0x01:
                ImageFormat::Encode::I8(stream, Width, Height, ImageData);
                break;
            case 0x00:
                ImageFormat::Encode::I4(stream, Width, Height, ImageData);
                break;
        }
    }

    void TextureHeader::SetImage(uint8_t* data, std::size_t size, int w, int h){
        if(TextureID != UINT32_MAX) glDeleteTextures(1, &TextureID);
        if(ImageData != nullptr) delete[] ImageData;

        ImageData = new uint8_t[size];
        memcpy(ImageData, data, size);

        Width = w;
        Height = h;

        glGenTextures(1, &TextureID);
        glBindTexture(GL_TEXTURE_2D, TextureID);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, ImageData);

        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void TextureHeader::Destroy(){
        glDeleteTextures(1, &TextureID);
        delete[] ImageData;
    }

    void Batch::Destroy(){
        glDeleteVertexArrays(1, &Vao);
        glDeleteBuffers(1, &Vbo);
    }

    const char* default_bin_vtx_shader_source = "#version 460\n\
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
            mat4 IndTexMatrices[10];\n\
            uint BillboardType;\n\
            uint ModelId;\n\
            uint MaterialId;\n\
            vec4 HighlightColor;\n\
        };\n\
        uniform mat4 transform;\n\
        \
        layout(location = 0) in vec3 inPosition;\n\
        layout(location = 1) in vec3 inNormal;\n\
        layout(location = 2) in vec4 inColor;\n\
        layout(location = 3) in vec2 inTexCoord;\n\
        \
        layout(location = 0) out vec2 fragTexCoord;\n\
        layout(location = 1) out vec4 fragColor;\n\
        \
        void main()\n\
        {\
            gl_Position = Proj * View * transform * vec4(inPosition.z, inPosition.y, inPosition.x, 1.0);\n\
            fragTexCoord = inTexCoord;\n\
            fragColor = inColor;\n\
        }\
    ";

    const char* default_bin_frg_shader_source = "#version 460\n\
        #extension GL_ARB_separate_shader_objects : enable\n\
        \
        uniform sampler2D texSampler;\n\
        //uniform vec4 diffuseColor;\n\
        uniform int selected;\n\
        uniform int pickID;\n\
        \
        layout(location = 0) in vec2 fragTexCoord;\n\
        layout(location = 1) in vec4 fragColor;\n\
        \
        layout(location = 0) out vec4 outColor;\n\
        layout(location = 1) out int outPick;\n\
        \
        void main()\n\
        {\n\
            vec4 baseColor = texture(texSampler, vec2(fragTexCoord.x, fragTexCoord.y));\n\
            if(selected == 1){\n\
                outColor = baseColor * vec4(1.0, 1.0, 0.2, 1.0);\n\
            } else {\n\
                outColor = baseColor;\n\
            }\n\
            outPick = pickID;\n\
            if(baseColor.a < 1.0 / 255.0) discard;\n\
        }\
    ";

    void DestroyShaders(){
        glDeleteProgram(mProgram);
    }

    void InitShaders(){

        // Load Dev Tex
        if(std::filesystem::exists(RES_BASE_PATH / "img" / "missing.png")){
            int x, y, channels = 0;
            unsigned char* img = stbi_load((RES_BASE_PATH / "img" / "missing.png").string().c_str(), &x, &y, &channels, 4);
            glGenTextures(1, &MissingTexID);
            glBindTexture(GL_TEXTURE_2D, MissingTexID);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);
            glBindTexture(GL_TEXTURE_2D, 0);
            stbi_image_free(img);
        }

        char glErrorLogBuffer[4096];
        GLuint vs = glCreateShader(GL_VERTEX_SHADER);
        GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(vs, 1, &default_bin_vtx_shader_source, NULL);
        glShaderSource(fs, 1, &default_bin_frg_shader_source, NULL);
        glCompileShader(vs);
        GLint status;
        glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
        if(status == GL_FALSE){
            GLint infoLogLength;
            glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &infoLogLength);
            glGetShaderInfoLog(vs, infoLogLength, NULL, glErrorLogBuffer);
            printf("[MDL Loader]: Compile failure in mdl vertex shader:\n%s\n", glErrorLogBuffer);
        }
        glCompileShader(fs);
        glGetShaderiv(fs, GL_COMPILE_STATUS, &status);
        if(status == GL_FALSE){
            GLint infoLogLength;
            glGetShaderiv(fs, GL_INFO_LOG_LENGTH, &infoLogLength);
            glGetShaderInfoLog(fs, infoLogLength, NULL, glErrorLogBuffer);
            printf("[MDL Loader]: Compile failure in mdl fragment shader:\n%s\n", glErrorLogBuffer);
        }
        mProgram = glCreateProgram();
        glAttachShader(mProgram, vs);
        glAttachShader(mProgram, fs);
        glLinkProgram(mProgram);
        glGetProgramiv(mProgram, GL_LINK_STATUS, &status);
        if(GL_FALSE == status) {
            GLint logLen;
            glGetProgramiv(mProgram, GL_INFO_LOG_LENGTH, &logLen);
            glGetProgramInfoLog(mProgram, logLen, NULL, glErrorLogBuffer);
            printf("[MDL Loader]: Shader Program Linking Error:\n%s\n", glErrorLogBuffer);
        }
        glDetachShader(mProgram, vs);
        glDetachShader(mProgram, fs);
        glDeleteShader(vs);
        glDeleteShader(fs);
    }

    template<class T>
    T* Read(std::map<uint16_t, T>& list, bStream::CStream* stream, uint32_t offset, uint16_t index, std::size_t size){
        if(!list.contains(index)){
            std::size_t pos = offset + (index * size);
            stream->seek(pos, false);
            list[index] = {};
            list[index].Read(stream);
        }

        return &list[index];
    };

    void Model::ReadSceneGraphNode(bStream::CStream* stream, uint32_t index){
        SceneGraphNode* node = Read<SceneGraphNode>(mGraphNodes, stream, mHeader.SceneGraphOffset, index, 0x8C);

        node->Index = index;

        node->Transform = glm::mat4(1.0f);
        node->Transform = glm::scale(node->Transform, node->Scale);
        node->Transform = glm::rotate(node->Transform, glm::radians(node->Rotation.z), {1.0f, 0.0f, 0.0f});
        node->Transform = glm::rotate(node->Transform, glm::radians(node->Rotation.y), {0.0f, 1.0f, 0.0f});
        node->Transform = glm::rotate(node->Transform, glm::radians(node->Rotation.x), {0.0f, 0.0f, 1.0f});
        node->Transform = glm::translate(node->Transform, {node->Position.z, node->Position.y, node->Position.x});

        for(int i = 0; i < node->ElementCount; i++){
            stream->seek(mHeader.SceneGraphOffset + node->ElementOffset + (i * 4));

            DrawElement element;
            element.Read(stream);

            if(element.BatchIndex >= 0 && !mBatches.contains(element.BatchIndex)){
                Batch* batch = Read<Batch>(mBatches, stream, mHeader.BatchOffset, element.BatchIndex, 0x18);
                // read primitives

                std::vector<Vertex> triangulatedPrimitives;
                stream->seek(batch->PrimitiveOffset + mHeader.BatchOffset);
                while(stream->tell() < (mHeader.BatchOffset + batch->PrimitiveOffset) + (batch->DisplayListSize << 5)){
                    uint8_t opcode = stream->readUInt8();

                    if(opcode == 0 || (opcode != GXPrimitiveType::Triangles && opcode != GXPrimitiveType::Points && opcode != GXPrimitiveType::Lines && opcode != GXPrimitiveType::LineStrip && opcode != GXPrimitiveType::TriangleStrip && opcode != GXPrimitiveType::TriangleFan && opcode != GXPrimitiveType::Quads)) break;


                    uint16_t vertexCount = stream->readUInt16();
                    std::vector<PrimitiveVertex> primitiveVertices;
                    Primitive p;
                    p.Opcode = opcode;
                    for(int v = 0; v < vertexCount; v++){
                        PrimitiveVertex vtx = {0};
                        Vertex vtxData = {};

                        uint32_t mask = 1;
                        for(int a = 0; a < 26; a++){
                            if((batch->VertexAttributes & mask) >> a){
                                if((GXAttribute)a == GXAttribute::Position){
                                    vtx.Position = stream->readUInt16();
                                    vtxData.Position = mPositions[vtx.Position];
                                } else if((GXAttribute)a == GXAttribute::Normal){
                                    vtx.Normal = stream->readUInt16();
                                    vtxData.Normal = mNormals[vtx.Normal];
                                    if(batch->NBTFlag > 0){
                                        vtxData.Binormal = mNormals[stream->readUInt16()];
                                        vtxData.Tangent = mNormals[stream->readUInt16()];
                                    }
                                } else if((GXAttribute)a == GXAttribute::Tex0){ // this can be done cleaner, should make this all 13
                                    vtx.Texcoord = stream->readUInt16();
                                    vtxData.Texcoord = mTexCoords[vtx.Texcoord];
                                } else if((GXAttribute)a == GXAttribute::Tex1){
                                    vtxData.Texcoord1 = mTexCoords[stream->readUInt16()];
                                } else {
                                    stream->readInt16();
                                }
                            }
                            mask <<= 1;
                        }
                        p.Vertices.push_back(vtxData);
                        primitiveVertices.push_back(vtx);
                    }
                    batch->Primitives.push_back(p);

                    switch (opcode){
                    case GXPrimitiveType::Triangles: {
                            int8_t prevMtx = -1;
                            for(PrimitiveVertex vtxIdx : primitiveVertices){
                                Vertex vtx = {{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {1,1,1,1}, {0,0}, {0,0}};

                                vtx.Position = mPositions[vtxIdx.Position];
                                vtx.Normal = mNormals[vtxIdx.Normal];
                                vtx.Texcoord = mTexCoords[vtxIdx.Texcoord];

                                triangulatedPrimitives.push_back(vtx);
                            }
                        }
                        break;
                    case GXPrimitiveType::TriangleStrip: {
                            for (size_t v = 2; v < primitiveVertices.size(); v++){
                                Vertex vtx1 = {{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {1,1,1,1}, {0,0}, {0,0}},
                                       vtx2 = {{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {1,1,1,1}, {0,0}, {0,0}},
                                       vtx3 = {{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {1,1,1,1}, {0,0}, {0,0}};

                                vtx1.Position = mPositions[primitiveVertices[v-2].Position];
                                vtx1.Normal = mNormals[primitiveVertices[v-2].Normal];
                                vtx1.Texcoord = mTexCoords[primitiveVertices[v-2].Texcoord];

                                vtx2.Position = mPositions[primitiveVertices[(v % 2 != 0 ? v : v-1)].Position];
                                vtx2.Normal = mNormals[primitiveVertices[(v % 2 != 0 ? v : v-1)].Normal];
                                vtx2.Texcoord = mTexCoords[primitiveVertices[(v % 2 != 0 ? v : v-1)].Texcoord];

                                vtx3.Position = mPositions[primitiveVertices[(v % 2 != 0 ? v-1 : v)].Position];
                                vtx3.Normal = mNormals[primitiveVertices[(v % 2 != 0 ? v-1 : v)].Normal];
                                vtx3.Texcoord = mTexCoords[primitiveVertices[(v % 2 != 0 ? v-1 : v)].Texcoord];

                                triangulatedPrimitives.push_back(vtx1);
                                triangulatedPrimitives.push_back(vtx2);
                                triangulatedPrimitives.push_back(vtx3);

                            }
                        }
                        break;
                    case GXPrimitiveType::TriangleFan:{
                            for(size_t v = 0; v < 3; v++){
                                Vertex vtx = {{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {1,1,1,1}, {0,0}, {0,0}};

                                vtx.Position = mPositions[primitiveVertices[v].Position];
                                vtx.Normal = mNormals[primitiveVertices[v].Normal];
                                vtx.Texcoord = mTexCoords[primitiveVertices[v].Texcoord];

                                triangulatedPrimitives.push_back(vtx);
                            }

                            for (size_t v = 2; v < primitiveVertices.size(); v++){
                                Vertex vtx1 = {{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {1,1,1,1}, {0,0}, {0,0}},
                                       vtx2 = {{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {1,1,1,1}, {0,0}, {0,0}},
                                       vtx3 = {{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {1,1,1,1}, {0,0}, {0,0}};

                                if(primitiveVertices[v].Position == primitiveVertices[v-1].Position ||
                                   primitiveVertices[v-1].Position == primitiveVertices[0].Position ||
                                   primitiveVertices[v].Position == primitiveVertices[0].Position){
                                    continue;
                                }

                                vtx1.Position = mPositions[primitiveVertices[0].Position];
                                vtx1.Normal = mNormals[primitiveVertices[0].Normal];
                                vtx1.Texcoord = mTexCoords[primitiveVertices[0].Texcoord];

                                vtx2.Position = mPositions[primitiveVertices[v-1].Position];
                                vtx2.Normal = mNormals[primitiveVertices[v-1].Normal];
                                vtx2.Texcoord = mTexCoords[primitiveVertices[v-1].Texcoord];

                                vtx3.Position = mPositions[primitiveVertices[v].Position];
                                vtx3.Normal = mNormals[primitiveVertices[v].Normal];
                                vtx3.Texcoord = mTexCoords[primitiveVertices[v].Texcoord];

                                triangulatedPrimitives.push_back(vtx1);
                                triangulatedPrimitives.push_back(vtx2);
                                triangulatedPrimitives.push_back(vtx3);

                            }
                        }
                        break;
                    default:
                        std::cout << "[BIN Loader]: Unimplemented primitive " << std::format("{0}", opcode) << std::endl;
                        break;
                    }
                }

                glGenVertexArrays(1, &batch->Vao);
                glBindVertexArray(batch->Vao);

                glGenBuffers(1, &batch->Vbo);
                glBindBuffer(GL_ARRAY_BUFFER, batch->Vbo);

                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));

                glEnableVertexAttribArray(1);
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

                glEnableVertexAttribArray(2);
                glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Color));

                glEnableVertexAttribArray(3);
                glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Texcoord));

                batch->VertexCount = triangulatedPrimitives.size();
                glBufferData(GL_ARRAY_BUFFER, triangulatedPrimitives.size() * sizeof(Vertex), triangulatedPrimitives.data(), GL_STATIC_DRAW);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glBindVertexArray(0);
            }

            if(element.MaterialIndex >= 0 && !mMaterials.contains(element.MaterialIndex)){
                Material* material = Read<Material>(mMaterials, stream, mHeader.MaterialOffset, element.MaterialIndex, 0x28);
                // read materials

                for(int j = 0; j < 8; j++){
                    int16_t samplerIdx = material->SamplerIndices[j];
                    if(samplerIdx >= 0 && !mSamplers.contains(samplerIdx)){
                        Sampler* sampler = Read<Sampler>(mSamplers, stream, mHeader.SamplerOffset, samplerIdx, 0x14);
                        if(sampler->TextureIndex >= 0 && !mTexturesHeaders.contains(sampler->TextureIndex)){
                            TextureHeader* texture = Read<TextureHeader>(mTexturesHeaders, stream, mHeader.TextureOffset, sampler->TextureIndex, 0xC);

                            stream->seek(mHeader.TextureOffset + texture->ImageOffset);
                            texture->Load(stream);
                            //texture->Format = 0x04;
                        }
                    }
                }

            }

            node->mDrawElements.push_back(element);
        }

        if(node->ChildIndex != -1 && !mGraphNodes.contains(node->ChildIndex)){
            ReadSceneGraphNode(stream, node->ChildIndex);
        }

        if(node->NextSibIndex != -1 && !mGraphNodes.contains(node->NextSibIndex)){
            ReadSceneGraphNode(stream, node->NextSibIndex);
        }
    }

    void Model::Write(bStream::CStream* stream){
        bStream::CMemoryStream TextureStream((mTexturesHeaders.rbegin()->first * 0x0C), bStream::Endianess::Big, bStream::OpenMode::Out);
        bStream::CMemoryStream SamplerStream((mSamplers.rbegin()->first * 0x14), bStream::Endianess::Big, bStream::OpenMode::Out);
        bStream::CMemoryStream MaterialStream((mMaterials.rbegin()->first * 0x28), bStream::Endianess::Big, bStream::OpenMode::Out);
        bStream::CMemoryStream BatchStream((mBatches.rbegin()->first * 0x18), bStream::Endianess::Big, bStream::OpenMode::Out); // this doesnt account for primitive size
        bStream::CMemoryStream NodeStream((mGraphNodes.rbegin()->first * 0x8C) + (mGraphNodes.rbegin()->first * 0x4), bStream::Endianess::Big, bStream::OpenMode::Out);

        std::vector<glm::vec3> newPositions;
        std::vector<glm::vec3> newNormals;
        std::vector<glm::vec2> newTexCoords;

        for(auto [idx, texture] : mTexturesHeaders){
            texture.Write(&TextureStream);
        }
        TextureStream.alignTo(32);

        for(auto [idx, texture] : mTexturesHeaders){
            TextureStream.writeOffsetAt32((0xC * idx) + 8);
            texture.Save(&TextureStream);
            TextureStream.alignTo(32);
        }
        uint32_t TextureSectionSize = TextureStream.tell();

        for(auto [idx, sampler] : mSamplers){
            sampler.Write(&SamplerStream);
        }
        SamplerStream.alignTo(32);
        uint32_t SamplerSectionSize = SamplerStream.tell();

        for(auto [idx, material] : mMaterials){
            material.Write(&MaterialStream);
        }
        MaterialStream.alignTo(32);
        uint32_t MaterialSectionSize = MaterialStream.tell();

        for(auto [idx, batch] : mBatches){
            batch.Write(&BatchStream);
        }
        BatchStream.alignTo(32);

        // write primitives
        for(auto [idx, batch] : mBatches){
            BatchStream.writeOffsetAt32((idx * 0x18) + 12);

            std::size_t start = BatchStream.tell();
            for(auto primitive : batch.Primitives){
                BatchStream.writeUInt8(primitive.Opcode);
                BatchStream.writeUInt16(primitive.Vertices.size());
                for(auto vertex : primitive.Vertices){
                    if(HasAttribute<GXAttribute::Position>(batch.VertexAttributes)){
                        std::vector<glm::vec3>::iterator pos = std::find(newPositions.begin(), newPositions.end(), vertex.Position);
                        if(pos == newPositions.end()){
                            BatchStream.writeUInt16(newPositions.size());
                            newPositions.push_back(vertex.Position);
                        } else {
                            BatchStream.writeUInt16((pos - newPositions.begin()));
                        }
                    }
                    if(HasAttribute<GXAttribute::Normal>(batch.VertexAttributes)){
                        std::vector<glm::vec3>::iterator pos = std::find(newNormals.begin(), newNormals.end(), vertex.Normal);
                        if(pos == newNormals.end()){
                            BatchStream.writeUInt16(newNormals.size());
                            newNormals.push_back(vertex.Normal);
                        } else {
                            BatchStream.writeUInt16(pos - newNormals.begin());
                        }
                        if(batch.NBTFlag > 0){
                            pos = std::find(newNormals.begin(), newNormals.end(), vertex.Binormal);
                            if(pos == newNormals.end()){
                                BatchStream.writeUInt16(newNormals.size());
                                newNormals.push_back(vertex.Binormal);
                            } else {
                                BatchStream.writeUInt16(pos - newNormals.begin());
                            }
                            pos = std::find(newNormals.begin(), newNormals.end(), vertex.Tangent);
                            if(pos == newNormals.end()){
                                BatchStream.writeUInt16(newNormals.size());
                                newNormals.push_back(vertex.Tangent);
                            } else {
                                BatchStream.writeUInt16(pos - newNormals.begin());
                            }
                        }
                    }
                    if(HasAttribute<GXAttribute::Tex0>(batch.VertexAttributes)){
                        std::vector<glm::vec2>::iterator pos = std::find(newTexCoords.begin(), newTexCoords.end(), vertex.Texcoord);
                        if(pos == newTexCoords.end()){
                            BatchStream.writeUInt16(newTexCoords.size());
                            newTexCoords.push_back(vertex.Texcoord);
                        } else {
                            BatchStream.writeUInt16(pos - newTexCoords.begin());
                        }
                    }
                    if(HasAttribute<GXAttribute::Tex1>(batch.VertexAttributes)){
                        std::vector<glm::vec2>::iterator pos = std::find(newTexCoords.begin(), newTexCoords.end(), vertex.Texcoord1);
                        if(pos == newTexCoords.end()){
                            BatchStream.writeUInt16(newTexCoords.size());
                            newTexCoords.push_back(vertex.Texcoord1);
                        } else {
                            BatchStream.writeUInt16(pos - newTexCoords.begin());
                        }
                    }
                }
            }
            BatchStream.alignTo(32);

            std::size_t end = BatchStream.tell();
            BatchStream.seek((idx * 0x18) + 2);
            batch.DisplayListSize = (end - start) / 0x20;
            BatchStream.writeUInt16((end - start) / 0x20);
            BatchStream.seek(end);
        }
        uint32_t BatchSectionSize = BatchStream.tell();

        for(auto [idx, node] : mGraphNodes){
            node.Write(&NodeStream);
        }

        for(auto [idx, node] : mGraphNodes){
            NodeStream.writeOffsetAt32((0x8C * idx) + 80);
            for(auto element : node.mDrawElements){
                NodeStream.writeInt16(element.MaterialIndex);
                NodeStream.writeInt16(element.BatchIndex);
            }
        }
        uint32_t NodeSectionSize = NodeStream.tell();

        stream->seek(0);
        stream->writeUInt8(mHeader.Version);
        stream->writeString(mHeader.Name);
        for(int i = 0; i < 21; i++) stream->writeUInt32(0);

        stream->writeOffsetAt32(12);
        stream->writeBytes(TextureStream.getBuffer(), TextureSectionSize);
        stream->writeOffsetAt32(16);
        stream->writeBytes(SamplerStream.getBuffer(), SamplerSectionSize);
        stream->writeOffsetAt32(20);
        for(int i = 0; i < newPositions.size(); i++){
            stream->writeInt16(static_cast<int16_t>(newPositions[i].x));
            stream->writeInt16(static_cast<int16_t>(newPositions[i].y));
            stream->writeInt16(static_cast<int16_t>(newPositions[i].z));
        }
        stream->alignTo(32);

        stream->writeOffsetAt32(24);
        for(int i = 0; i < newNormals.size(); i++){
            stream->writeFloat(newNormals[i].x);
            stream->writeFloat(newNormals[i].y);
            stream->writeFloat(newNormals[i].z);
        }
        stream->alignTo(32);

        stream->writeOffsetAt32(36);
        for(int i = 0; i < newTexCoords.size(); i++){
            stream->writeFloat(newTexCoords[i].x);
            stream->writeFloat(newTexCoords[i].y);
        }
        stream->alignTo(32);

        stream->writeOffsetAt32(52);
        stream->writeBytes(MaterialStream.getBuffer(), MaterialSectionSize);

        stream->writeOffsetAt32(56);
        stream->writeBytes(BatchStream.getBuffer(), BatchSectionSize);
        stream->alignTo(32);

        stream->writeOffsetAt32(60);
        stream->writeBytes(NodeStream.getBuffer(), NodeSectionSize);
        stream->alignTo(16);
    }

    void Model::Load(bStream::CStream* stream){
        stream->seek(0);
        mHeader.Version = stream->readUInt8();
        mHeader.Name = stream->readString(11);
        mHeader.TextureOffset = stream->readUInt32();     // 0
        mHeader.SamplerOffset = stream->readUInt32();     // 1
        mHeader.PositionOffset = stream->readUInt32();    // 2
        mHeader.NormalOffset = stream->readUInt32();      // 3
        mHeader.Color0Offset = stream->readUInt32();      // 4
        mHeader.Color1Offest = stream->readUInt32();      // 5
        mHeader.TexCoord0Offset = stream->readUInt32();   // 6
        mHeader.TexCoord1Offset = stream->readUInt32();   // 7
        mHeader.UnknownOffsets[0] = stream->readUInt32();
        mHeader.UnknownOffsets[1] = stream->readUInt32();
        mHeader.MaterialOffset = stream->readUInt32();    // 10
        mHeader.BatchOffset = stream->readUInt32();       // 11
        mHeader.SceneGraphOffset = stream->readUInt32();  // 12


        uint32_t vertexCount = 0;
        uint32_t texcoordCount = 0;
        uint32_t normalCount = 0;
        {
            uint32_t chunkOffsets[21];
            stream->seek(12);
            for (std::size_t o = 0; o < 21; o++)
            {
                chunkOffsets[o] = stream->readUInt32();
            }



            for(std::size_t o = 3; o < 21; o++)
            {
                vertexCount = (uint32_t)((chunkOffsets[o] - chunkOffsets[2]) / 6);
                if(chunkOffsets[o] != 0) break;
            }

            for(std::size_t o = 4; o < 21; o++)
            {
                normalCount = (uint32_t)((chunkOffsets[o] - chunkOffsets[3]) / 12);
                if(chunkOffsets[o] != 0) break;
            }

            for(std::size_t o = 7; o < 21; o++)
            {
                texcoordCount = (uint32_t)((chunkOffsets[o] - chunkOffsets[6]) / 8);
                if(chunkOffsets[o] != 0) break;
            }
        }

        stream->seek(mHeader.PositionOffset);
        for(int i = 0; i < vertexCount; i++){
            mPositions.push_back({stream->readInt16(), stream->readInt16(), stream->readInt16()});
        }

        stream->seek(mHeader.NormalOffset);
        for(int i = 0; i < normalCount; i++){
            mNormals.push_back({stream->readFloat(), stream->readFloat(), stream->readFloat()});
        }

        stream->seek(mHeader.TexCoord0Offset);
        for(int i = 0; i < texcoordCount; i++){
            mTexCoords.push_back({stream->readFloat(), stream->readFloat()});
        }

        stream->seek(0);
        ReadSceneGraphNode(stream, 0);
    }

    void Model::DrawScenegraphNode(uint32_t idx, glm::mat4 transform){
        if(idx == -1) return;

        SceneGraphNode* node = &mGraphNodes[idx];
        glm::mat4 mtx = transform * node->Transform;

        if(mAnim.mLoaded && mAnim.mPlaying){
            GraphNodeTrack* track = &mAnimationTracks[idx];
            float sx = MixTrack(&track->mXScaleTrack, mAnim.mFrameCount, mAnim.mCurrentFrame, track->mNextScaleKeyX);
            float sy = MixTrack(&track->mYScaleTrack, mAnim.mFrameCount, mAnim.mCurrentFrame, track->mNextScaleKeyY);
            float sz = MixTrack(&track->mZScaleTrack, mAnim.mFrameCount, mAnim.mCurrentFrame, track->mNextScaleKeyZ);

            float rz = MixTrack(&track->mXRotTrack, mAnim.mFrameCount, mAnim.mCurrentFrame, track->mNextRotKeyX);
            float ry = MixTrack(&track->mYRotTrack, mAnim.mFrameCount, mAnim.mCurrentFrame, track->mNextRotKeyY);
            float rx = MixTrack(&track->mZRotTrack, mAnim.mFrameCount, mAnim.mCurrentFrame, track->mNextRotKeyZ);

            float px = MixTrack(&track->mXPosTrack, mAnim.mFrameCount, mAnim.mCurrentFrame, track->mNextPosKeyX);
            float py = MixTrack(&track->mYPosTrack, mAnim.mFrameCount, mAnim.mCurrentFrame, track->mNextPosKeyY);
            float pz = MixTrack(&track->mZPosTrack, mAnim.mFrameCount, mAnim.mCurrentFrame, track->mNextPosKeyZ);

            glm::mat4 animTrasform(1.0f);
            //animTrasform = glm::scale(animTrasform, glm::vec3(sx, sy, sz));
            //animTrasform = glm::rotate(animTrasform, glm::radians(rx * 0.0001533981f), glm::vec3(1, 0, 0));
            //animTrasform = glm::rotate(animTrasform, glm::radians(ry * 0.0001533981f), glm::vec3(0, 1, 0));
            //animTrasform = glm::rotate(animTrasform, glm::radians(rz * 0.0001533981f), glm::vec3(0, 0, 1));
            animTrasform = glm::translate(animTrasform, glm::vec3(px, py, pz));

            glUniformMatrix4fv(glGetUniformLocation(mProgram, "transform"), 1, 0, &(mtx * animTrasform)[0][0]);
        } else {

            glUniformMatrix4fv(glGetUniformLocation(mProgram, "transform"), 1, 0, &(mtx)[0][0]);
        }


        for(auto element : node->mDrawElements){
            if(element.BatchIndex == -1) continue;
            glBindVertexArray(mBatches[element.BatchIndex].Vao);

            int16_t samplerIdx = 0;
            if(element.MaterialIndex >= 0) samplerIdx = mMaterials[element.MaterialIndex].SamplerIndices[0];
            if(samplerIdx != -1 && mSamplers[samplerIdx].TextureIndex != -1){
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, mTexturesHeaders[mSamplers[samplerIdx].TextureIndex].TextureID);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (mSamplers[samplerIdx].WrapU == 2 ? GL_MIRRORED_REPEAT : (mSamplers[samplerIdx].WrapU == 1 ? GL_REPEAT : GL_CLAMP_TO_EDGE)));
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (mSamplers[samplerIdx].WrapV == 2 ? GL_MIRRORED_REPEAT : (mSamplers[samplerIdx].WrapV == 1 ? GL_REPEAT : GL_CLAMP_TO_EDGE)));
            } else {
                // bind dev texture MissingTexID
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, MissingTexID);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            }
            glDrawArrays(GL_TRIANGLES, 0, mBatches[element.BatchIndex].VertexCount);
        }

        if(node->ChildIndex != -1 && node->ChildIndex != 0 && node->ChildIndex != node->Index){
            DrawScenegraphNode(node->ChildIndex, mtx);
        }

        if(node->NextSibIndex != -1 && node->NextSibIndex != 0 && node->NextSibIndex != node->Index){
            DrawScenegraphNode(node->NextSibIndex, transform);
        }
    }

    void Model::Draw(glm::mat4* transform, int32_t id, bool selected){
        glUseProgram(mProgram);
        glUniform1i(glGetUniformLocation(mProgram, "pickID"), id);
        glUniform1i(glGetUniformLocation(mProgram, "selected"), selected);

        DrawScenegraphNode(0, *transform);
    }

    void Model::LoadAnimation(bStream::CStream* stream){
        mAnim.mLoaded = true;

        stream->readUInt8(); // version
        mAnim.mLoop = stream->readUInt8(); // loop

        stream->readUInt16(); // padding
        mAnim.mFrameCount = stream->readUInt32(); // frame count

        uint32_t scaleKeyOffset = stream->readUInt32();
        uint32_t rotateKeyOffset = stream->readUInt32();
        uint32_t translateKeyOffset = stream->readUInt32();
        uint32_t groupOffset = stream->readUInt32();

        for(int i = 0; i < mGraphNodes.rbegin()->first+1; i++){
            stream->seek(groupOffset + (i * 0x36));

            GraphNodeTrack track;

            track.mXScaleTrack.LoadTrack(stream, scaleKeyOffset, ETrackType::ANM);
            track.mYScaleTrack.LoadTrack(stream, scaleKeyOffset, ETrackType::ANM);
            track.mZScaleTrack.LoadTrack(stream, scaleKeyOffset, ETrackType::ANM);

            track.mXRotTrack.LoadTrack(stream, rotateKeyOffset, ETrackType::ANM);
            track.mYRotTrack.LoadTrack(stream, rotateKeyOffset, ETrackType::ANM);
            track.mZRotTrack.LoadTrack(stream, rotateKeyOffset, ETrackType::ANM);

            track.mXPosTrack.LoadTrack(stream, translateKeyOffset, ETrackType::ANM);
            track.mYPosTrack.LoadTrack(stream, translateKeyOffset, ETrackType::ANM);
            track.mZPosTrack.LoadTrack(stream, translateKeyOffset, ETrackType::ANM);

            mAnimationTracks[i] = track;
        }

    }

    void ClearAnimation(){

    }

    Model::~Model(){
        for(auto [idx, shape] : mBatches){
            shape.Destroy();
        }

        for(auto [idx, texture] : mTexturesHeaders){
            texture.Destroy();
        }
    }
};
