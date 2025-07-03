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
#include "tiny_obj_loader.h"
#include "ufbx.h"
#include "tri_stripper.h"

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

    void Batch::ReloadMeshes(){
        glDeleteVertexArrays(1, &Vao);
        glDeleteBuffers(1, &Vbo);
                
        glGenVertexArrays(1, &Vao);
        glBindVertexArray(Vao);

        glGenBuffers(1, &Vbo);
        glBindBuffer(GL_ARRAY_BUFFER, Vbo);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Color));

        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Texcoord));

        std::vector<Vertex> triangulatedPrimitives;
        for (auto primitive : Primitives){
            switch (primitive.Opcode){
                case GXPrimitiveType::Triangles: {
                        for(auto vtx : primitive.Vertices){
                            triangulatedPrimitives.push_back(vtx);
                        }
                    }
                    break;
                case GXPrimitiveType::TriangleStrip: {
                        for (std::size_t v = 2; v < primitive.Vertices.size(); v++){
                            triangulatedPrimitives.push_back(primitive.Vertices[v-2]);
                            triangulatedPrimitives.push_back(primitive.Vertices[(v % 2 != 0 ? v : v-1)]);
                            triangulatedPrimitives.push_back(primitive.Vertices[(v % 2 != 0 ? v-1 : v)]);
                        }
                    }
                    break;
                case GXPrimitiveType::TriangleFan:{
                        for(std::size_t v = 0; v < 3; v++){
                            triangulatedPrimitives.push_back(primitive.Vertices[v]);
                        }

                        for (std::size_t v = 2; v < primitive.Vertices.size(); v++){

                            if(primitive.Vertices[v].Position == primitive.Vertices[v-1].Position ||
                                primitive.Vertices[v-1].Position == primitive.Vertices[0].Position ||
                                primitive.Vertices[v].Position == primitive.Vertices[0].Position){
                                continue;
                            }

                            triangulatedPrimitives.push_back(primitive.Vertices[0]);
                            triangulatedPrimitives.push_back(primitive.Vertices[v-1]);
                            triangulatedPrimitives.push_back(primitive.Vertices[v]);
                        }
                    }
                    break;
                default: {
                    std::cout << "[BIN Loader]: Unimplemented primitive " << std::format("{0}", primitive.Opcode) << std::endl;
                    break;
                }
            }
        }

        
        VertexCount = triangulatedPrimitives.size();
        glBufferData(GL_ARRAY_BUFFER, triangulatedPrimitives.size() * sizeof(Vertex), triangulatedPrimitives.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }    

    void Batch::Destroy(){
        glDeleteVertexArrays(1, &Vao);
        glDeleteBuffers(1, &Vbo);
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
            vec4 baseColor = texture(texSampler, vec2(fragTexCoord.x, fragTexCoord.y)) * fragColor;\n\
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
                                } else if((GXAttribute)a == GXAttribute::Color0){
                                    vtxData.Color = mColors[stream->readUInt16()];
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
                                if(HasAttribute<GXAttribute::Color0>(batch->VertexAttributes)) vtx.Color = mColors[vtxIdx.Color];
                                vtx.Texcoord = mTexCoords[vtxIdx.Texcoord];

                                triangulatedPrimitives.push_back(vtx);
                            }
                        }
                        break;
                    case GXPrimitiveType::TriangleStrip: {
                            for (std::size_t v = 2; v < primitiveVertices.size(); v++){
                                Vertex vtx1 = {{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {1,1,1,1}, {0,0}, {0,0}},
                                       vtx2 = {{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {1,1,1,1}, {0,0}, {0,0}},
                                       vtx3 = {{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {1,1,1,1}, {0,0}, {0,0}};

                                vtx1.Position = mPositions[primitiveVertices[v-2].Position];
                                vtx1.Normal = mNormals[primitiveVertices[v-2].Normal];
                                if(HasAttribute<GXAttribute::Color0>(batch->VertexAttributes)) vtx1.Color = mColors[primitiveVertices[v-2].Color];
                                vtx1.Texcoord = mTexCoords[primitiveVertices[v-2].Texcoord];

                                vtx2.Position = mPositions[primitiveVertices[(v % 2 != 0 ? v : v-1)].Position];
                                vtx2.Normal = mNormals[primitiveVertices[(v % 2 != 0 ? v : v-1)].Normal];
                                if(HasAttribute<GXAttribute::Color0>(batch->VertexAttributes)) vtx2.Color = mColors[primitiveVertices[(v % 2 != 0 ? v : v-1)].Color];
                                vtx2.Texcoord = mTexCoords[primitiveVertices[(v % 2 != 0 ? v : v-1)].Texcoord];

                                vtx3.Position = mPositions[primitiveVertices[(v % 2 != 0 ? v-1 : v)].Position];
                                vtx3.Normal = mNormals[primitiveVertices[(v % 2 != 0 ? v-1 : v)].Normal];
                                if(HasAttribute<GXAttribute::Color0>(batch->VertexAttributes)) vtx3.Color = mColors[primitiveVertices[(v % 2 != 0 ? v-1 : v)].Color];
                                vtx3.Texcoord = mTexCoords[primitiveVertices[(v % 2 != 0 ? v-1 : v)].Texcoord];

                                triangulatedPrimitives.push_back(vtx1);
                                triangulatedPrimitives.push_back(vtx2);
                                triangulatedPrimitives.push_back(vtx3);

                            }
                        }
                        break;
                    case GXPrimitiveType::TriangleFan:{
                            for(std::size_t v = 0; v < 3; v++){
                                Vertex vtx = {{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {1,1,1,1}, {0,0}, {0,0}};

                                vtx.Position = mPositions[primitiveVertices[v].Position];
                                vtx.Normal = mNormals[primitiveVertices[v].Normal];
                                if(HasAttribute<GXAttribute::Color0>(batch->VertexAttributes)) vtx.Color = mColors[primitiveVertices[v].Color];
                                vtx.Texcoord = mTexCoords[primitiveVertices[v].Texcoord];

                                triangulatedPrimitives.push_back(vtx);
                            }

                            for (std::size_t v = 2; v < primitiveVertices.size(); v++){
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
                                if(HasAttribute<GXAttribute::Color0>(batch->VertexAttributes)) vtx1.Color = mColors[primitiveVertices[0].Color];
                                vtx1.Texcoord = mTexCoords[primitiveVertices[0].Texcoord];

                                vtx2.Position = mPositions[primitiveVertices[v-1].Position];
                                vtx2.Normal = mNormals[primitiveVertices[v-1].Normal];
                                if(HasAttribute<GXAttribute::Color0>(batch->VertexAttributes)) vtx2.Color = mColors[primitiveVertices[v-1].Color];
                                vtx2.Texcoord = mTexCoords[primitiveVertices[v-1].Texcoord];

                                vtx3.Position = mPositions[primitiveVertices[v].Position];
                                vtx3.Normal = mNormals[primitiveVertices[v].Normal];
                                if(HasAttribute<GXAttribute::Color0>(batch->VertexAttributes)) vtx3.Color = mColors[primitiveVertices[v].Color];
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
                        if(sampler->TextureIndex >= 0 && !mTextureHeaders.contains(sampler->TextureIndex)){
                            TextureHeader* texture = Read<TextureHeader>(mTextureHeaders, stream, mHeader.TextureOffset, sampler->TextureIndex, 0xC);

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
        // This will write one blank resource if there are none of that resource. Really, this should never happen, should add basic missing texture for imports if model has no textures.
        bStream::CMemoryStream TextureStream(mTextureHeaders.size() > 0 ? (mTextureHeaders.rbegin()->first * 0x0C) : 0x0C, bStream::Endianess::Big, bStream::OpenMode::Out);
        bStream::CMemoryStream SamplerStream(mSamplers.size() > 0 ? (mSamplers.rbegin()->first * 0x14) : 0x14, bStream::Endianess::Big, bStream::OpenMode::Out);
        bStream::CMemoryStream MaterialStream(mMaterials.size() > 0 ? (mMaterials.rbegin()->first * 0x28) : 0x28, bStream::Endianess::Big, bStream::OpenMode::Out);
        bStream::CMemoryStream BatchStream(mBatches.size() > 0 ? (mBatches.rbegin()->first * 0x18) : 0x18, bStream::Endianess::Big, bStream::OpenMode::Out); // this doesnt account for primitive size
        bStream::CMemoryStream NodeStream(mGraphNodes.size() > 0 ? (mGraphNodes.rbegin()->first * 0x8C) + (mGraphNodes.rbegin()->first * 0x4) : 0x8C, bStream::Endianess::Big, bStream::OpenMode::Out);

        std::vector<glm::vec3> newPositions;
        std::vector<glm::vec3> newNormals;
        std::vector<glm::vec4> newColors;
        std::vector<glm::vec2> newTexCoords;

        for(auto [idx, texture] : mTextureHeaders){
            texture.Write(&TextureStream);
        }
        TextureStream.alignTo(32);

        for(auto [idx, texture] : mTextureHeaders){
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
                    if(HasAttribute<GXAttribute::Color0>(batch.VertexAttributes)){
                        std::vector<glm::vec4>::iterator pos = std::find(newColors.begin(), newColors.end(), vertex.Color);
                        if(pos == newColors.end()){
                            BatchStream.writeUInt16(newColors.size());
                            newColors.push_back(vertex.Color);
                        } else {
                            BatchStream.writeUInt16(pos - newColors.begin());
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
        stream->alignTo(32);

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

        if(newColors.size() > 0){
            stream->writeOffsetAt32(28);
            for(int i = 0; i < newColors.size(); i++){
                stream->writeUInt8(newColors[i].x*0xFF);
                stream->writeUInt8(newColors[i].y*0xFF);
                stream->writeUInt8(newColors[i].z*0xFF);
                stream->writeUInt8(newColors[i].w*0xFF);
            }
            stream->alignTo(32);
        }

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
        uint32_t colorCount = 0;
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

            for(std::size_t o = 5; o < 21; o++)
            {
                colorCount = (uint32_t)((chunkOffsets[o] - chunkOffsets[4]) / 4);
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

        if(mHeader.Color0Offset != 0 && colorCount != 0){
            stream->seek(mHeader.Color0Offset);
            for(int i = 0; i < colorCount; i++){
                mColors.push_back({static_cast<float>(stream->readUInt8()) / 0xFF, static_cast<float>(stream->readUInt8()) / 0xFF, static_cast<float>(stream->readUInt8()) / 0xFF, static_cast<float>(stream->readUInt8()) / 0xFF});
            }
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
                glBindTexture(GL_TEXTURE_2D, mTextureHeaders[mSamplers[samplerIdx].TextureIndex].TextureID);
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

    Model Model::FromFBX(std::string path){
        Model mdl;

        ufbx_load_opts opts = {};
        opts.target_axes = ufbx_axes_right_handed_y_up;
        opts.target_unit_meters = 1.0f;
        ufbx_scene* scene = ufbx_load_file(path.c_str(), &opts, nullptr);

        // set up resources per node
        for(ufbx_node* node : scene->nodes) mdl.mGraphNodes[node->typed_id] = {};
        for(ufbx_texture* texture : scene->textures){
            if(texture->content.data == nullptr && std::string(texture->absolute_filename.data) == "") continue;
            mdl.mSamplers[texture->typed_id] = {};
            mdl.mTextureHeaders[texture->typed_id] = {};

            mdl.mSamplers[texture->typed_id].TextureIndex = texture->typed_id;
            mdl.mSamplers[texture->typed_id].WrapU = 1;//texture->wrap_u;
            mdl.mSamplers[texture->typed_id].WrapV = 1;//texture->wrap_v;
            mdl.mTextureHeaders[texture->typed_id].Format = 0x0E;
            
            // if the texture is embedded
            if(texture->content.data != nullptr){
                int x, y, c;
                unsigned char* data = stbi_load_from_memory((const unsigned char*)texture->content.data, texture->content.size, &x, &y, &c, 4);
                mdl.mTextureHeaders[texture->typed_id].SetImage(data, x*y*4, x, y);
                mdl.mTextureHeaders[texture->typed_id].Width = x;
                mdl.mTextureHeaders[texture->typed_id].Height = y;
                stbi_image_free(data);
            } else {
                int x, y, c;
                unsigned char* data = stbi_load(texture->absolute_filename.data, &x, &y, &c, 4);
                mdl.mTextureHeaders[texture->typed_id].SetImage(data, x*y*4, x, y);
                mdl.mTextureHeaders[texture->typed_id].Width = x;
                mdl.mTextureHeaders[texture->typed_id].Height = y;
                stbi_image_free(data);
            }
        }

        for(ufbx_material* material : scene->materials){
            mdl.mMaterials[material->typed_id] = {};
            //mdl.mMaterials[material->typed_id].Color = { material->fbx.ambient_color.value_vec4.x, material->fbx.ambient_color.value_vec4.y, material->fbx.ambient_color.value_vec4.z, material->fbx.ambient_color.value_vec4.w };
            int idx = 0;
            for(ufbx_material_texture texture : material->textures){
                if(idx >= 8) break;
                mdl.mMaterials[material->typed_id].SamplerIndices[idx] = texture.texture->typed_id;
                idx++;
            }
        }

        std::map<std::pair<uint32_t, uint32_t>, uint16_t> meshIdxRemap;
        uint16_t batchIdx = 0;
        for(ufbx_mesh* mesh : scene->meshes){
            for(ufbx_mesh_part part : mesh->material_parts){
                meshIdxRemap[{mesh->typed_id, part.index}] = batchIdx;
                mdl.mBatches[batchIdx] = {};

                if(mesh->vertex_color.exists) mdl.mBatches[batchIdx].VertexAttributes |= (1 << (int)GXAttribute::Color0);
                if(mesh->vertex_tangent.exists && mesh->vertex_bitangent.exists) mdl.mBatches[batchIdx].NBTFlag = 1;

                std::vector<Vertex> vertices;
                std::vector<uint32_t> triIndices;
                triIndices.resize(mesh->max_face_triangles * 3);
            
                // Iterate over each face using the specific material.
                for (uint32_t face_index : part.face_indices) {
                    ufbx_face face = mesh->faces[face_index];
            
                    // Triangulate the face into `tri_indices[]`.
                    uint32_t numTris = ufbx_triangulate_face(triIndices.data(), triIndices.size(), mesh, face);
            
                    // Iterate over each triangle corner contiguously.
                    for (std::size_t i = 0; i < numTris * 3; i++) {
                        uint32_t index = triIndices[i];
            
                        Vertex v;
                        v.Position = { mesh->vertex_position[index].x*100, mesh->vertex_position[index].y*100, mesh->vertex_position[index].z*100 };
                        v.Normal = { mesh->vertex_normal[index].x, mesh->vertex_normal[index].y, mesh->vertex_normal[index].z };
                        if(mesh->vertex_color.exists) v.Color = { mesh->vertex_color[index].x, mesh->vertex_color[index].y, mesh->vertex_color[index].z, mesh->vertex_color[index].w };
                        if(mesh->vertex_tangent.exists && mesh->vertex_bitangent.exists){
                            v.Binormal = { mesh->vertex_bitangent[index].x, mesh->vertex_bitangent[index].y, mesh->vertex_bitangent[index].z };
                            v.Tangent = { mesh->vertex_tangent[index].x, mesh->vertex_tangent[index].y, mesh->vertex_tangent[index].z };
                        }
                        v.Texcoord = { mesh->vertex_uv[index].x, mesh->vertex_uv[index].y };
                        vertices.push_back(v);
                    }
                }

                ufbx_vertex_stream streams[1] = { { vertices.data(), vertices.size(), sizeof(Vertex) } };
                
                std::vector<uint32_t> indicesDeuplicated;
                indicesDeuplicated.resize(part.num_triangles * 3);

                std::size_t numVertices = ufbx_generate_indices(streams, 1, indicesDeuplicated.data(), indicesDeuplicated.size(), nullptr, nullptr);
                vertices.resize(numVertices);

                std::vector<std::size_t> indices;
                indices.assign(indicesDeuplicated.begin(), indicesDeuplicated.end());

                triangle_stripper::tri_stripper stripify(indices);
                triangle_stripper::primitive_vector primitives;
                stripify.SetBackwardSearch(false);
                stripify.Strip(&primitives);

                for(auto p : primitives){
                    BIN::Primitive primitive;
                    primitive.Opcode = (p.Type == triangle_stripper::TRIANGLE_STRIP ? GXPrimitiveType::TriangleStrip : GXPrimitiveType::Triangles);
                    for(int i = 0; i < p.Indices.size(); i++){
                        primitive.Vertices.push_back(vertices[p.Indices[i]]);
                    }
                    mdl.mBatches[batchIdx].Primitives.push_back(primitive);
                }
                batchIdx++;
            }
        }

        for(ufbx_node* node : scene->nodes) mdl.mGraphNodes[node->typed_id] = {}; // Initialize ndoes

        for(ufbx_node* node : scene->nodes){
            mdl.mGraphNodes[node->typed_id].Index = node->typed_id;
            mdl.mGraphNodes[node->typed_id].Position = { node->local_transform.translation.x, node->local_transform.translation.y, node->local_transform.translation.z };
            mdl.mGraphNodes[node->typed_id].Rotation = { node->local_transform.rotation.x, node->local_transform.rotation.y, node->local_transform.rotation.z };
            mdl.mGraphNodes[node->typed_id].Scale = { node->local_transform.scale.x, node->local_transform.scale.y, node->local_transform.scale.z };

            if(node->parent != nullptr){
                mdl.mGraphNodes[node->typed_id].ParentIndex = node->parent->typed_id;
            }

            for(auto child = node->children.begin(); child != node->children.end(); child++){
                if(*child == node->children[0]){
                    mdl.mGraphNodes[node->typed_id].ChildIndex = (*child)->typed_id;
                }
                if(*child != node->children[0]){
                    mdl.mGraphNodes[(*(child-1))->typed_id].NextSibIndex = (*child)->typed_id;
                    mdl.mGraphNodes[(*child)->typed_id].PreviousSibIndex = (*(child-1))->typed_id;
                }
            }

            // dont set up mesh parts if we have none
            if(node->attrib_type != ufbx_element_type::UFBX_ELEMENT_MESH){
                continue;
            }

            for(ufbx_mesh_part part : node->mesh->material_parts){
                DrawElement element;
                element.BatchIndex = meshIdxRemap[{node->mesh->typed_id, part.index}];
                element.MaterialIndex = node->mesh->materials[part.index]->typed_id;
                mdl.mGraphNodes[node->typed_id].mDrawElements.push_back(element);
            }

        }


        ufbx_free_scene(scene);

        return mdl;
    }

    Model Model::FromOBJ(std::string path){
        Model mdl;
        tinyobj::attrib_t attributes;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;

        std::string warn;
        std::string err;
        bool ret = tinyobj::LoadObj(&attributes, &shapes, &materials, &warn, &err, std::filesystem::path(path).string().c_str(), std::filesystem::path(path).parent_path().string().c_str(), true);

        mdl.mGraphNodes[0] = {};
        mdl.mGraphNodes[0].mDrawElements.resize(shapes.size());
        int elementIDX = 0;
        for(auto shp : shapes){
            if(shp.mesh.indices.size() == 0) continue;

            Batch batch;

            std::vector<Vertex> vertices;
            std::vector<std::size_t> indices;
            for(int i = 0; i < shp.mesh.indices.size(); i++){
                Vertex vtx;
                int v = shp.mesh.indices[i].vertex_index;
                int n = shp.mesh.indices[i].normal_index;
                int t = shp.mesh.indices[i].texcoord_index;
                vtx.Position = glm::vec3(attributes.vertices[v * 3], attributes.vertices[(v * 3) + 1], attributes.vertices[(v * 3) + 2]);
                vtx.Normal = glm::vec3(attributes.normals[n * 3], attributes.normals[(n * 3) + 1], attributes.normals[(n * 3) + 2]);
                vtx.Texcoord = glm::vec2(attributes.texcoords[t * 2], attributes.texcoords[(t * 2) + 1]);

                auto vtxIdx = std::find_if(vertices.begin(), vertices.end(), [i=vtx](const Vertex& o){
                    return i.Position == o.Position && i.Normal == o.Normal && i.Texcoord == o.Texcoord;
                });

                if(vtxIdx != vertices.end()){
                    indices.push_back(vtxIdx - vertices.begin());
                } else {
                    indices.push_back(vertices.size());
                    vertices.push_back(vtx);
                }
            }

            triangle_stripper::tri_stripper stripify(indices);
            triangle_stripper::primitive_vector primitives;
            stripify.SetBackwardSearch(false);
            stripify.Strip(&primitives);

            int indexCount = 0;
            for(auto p : primitives){
                BIN::Primitive primitive;
                primitive.Opcode = (p.Type == triangle_stripper::TRIANGLE_STRIP ? GXPrimitiveType::TriangleStrip : GXPrimitiveType::Triangles);
                for(int i = 0; i < p.Indices.size(); i++){
                    primitive.Vertices.push_back(vertices[p.Indices[i]]);
                }
                indexCount += p.Indices.size();
                batch.Primitives.push_back(primitive);
            }
            mdl.mGraphNodes[0].mDrawElements[elementIDX].BatchIndex = mdl.mBatches.size();
            mdl.mBatches[mdl.mBatches.size()] = batch;

            if(shp.mesh.material_ids[0] != -1){
                Material material;
                Sampler sampler;
                TextureHeader tex;
                tex.Format = 0x0E;

                // TODO: load ambient color
                //material.Color = materials[shp.mesh.material_ids[0]].ambient
                
                int w, h, channels;
                unsigned char* img = stbi_load(materials[shp.mesh.material_ids[0]].diffuse_texname.c_str(), &w, &h, &channels, 4);
                tex.SetImage(img, w*h*4, w, h);
                stbi_image_free(img);
                
                sampler.TextureIndex = mdl.mTextureHeaders.size();
                mdl.mTextureHeaders[mdl.mTextureHeaders.size()] = tex;
                material.SamplerIndices[0] = mdl.mSamplers.size();
                mdl.mSamplers[mdl.mSamplers.size()] = sampler;


                mdl.mGraphNodes[0].mDrawElements[elementIDX].MaterialIndex = mdl.mMaterials.size();
                mdl.mMaterials[mdl.mMaterials.size()] = material;
            }

            elementIDX++;
        }
        return mdl;
    }

    void ClearAnimation(){

    }

    Model::~Model(){
        for(auto [idx, shape] : mBatches){
            shape.Destroy();
        }

        for(auto [idx, texture] : mTextureHeaders){
            texture.Destroy();
        }
    }
};
