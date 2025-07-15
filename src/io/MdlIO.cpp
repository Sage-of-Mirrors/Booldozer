#include "io/MdlIO.hpp"
#include "io/TxpIO.hpp"
#include <Bti.hpp>
#include <glad/glad.h>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <format>
#include <array>

static float angle = 0.0f;

namespace MDL {
    void SceneGraphNode::Read(bStream::CStream* stream){
        InverseMatrixIndex = stream->readUInt16();
        ChildIndexShift = stream->readUInt16();
        SiblingIndexShift = stream->readUInt16();
        PaddingFirst = stream->readUInt16();
        DrawElementCount = stream->readUInt16();
        DrawElementBeginIndex = stream->readUInt16();
        PaddingSecond = stream->readUInt32();
    }

    void SceneGraphNode::Save(bStream::CStream* stream){
        stream->writeUInt16(InverseMatrixIndex);
        stream->writeUInt16(ChildIndexShift);
        stream->writeUInt16(SiblingIndexShift);
        stream->writeUInt16(PaddingFirst);
        stream->writeUInt16(DrawElementCount);
        stream->writeUInt16(DrawElementBeginIndex);
        stream->writeUInt32(PaddingSecond);
    }

    void Sampler::Read(bStream::CStream* stream){
        TextureIndex = stream->readUInt16();
        UnknownIndex = stream->readUInt16();
        WrapU = stream->readUInt8();
        WrapV = stream->readUInt8();
        Unknown1 = stream->readUInt8();
        Unknown2 = stream->readUInt8();
    }

    void Sampler::Save(bStream::CStream* stream){
        stream->writeUInt16(TextureIndex);
        stream->writeUInt16(UnknownIndex);
        stream->writeUInt8(WrapU);
        stream->writeUInt8(WrapV);
        stream->writeUInt8(Unknown1);
        stream->writeUInt8(Unknown2);
    }

    void Material::Read(bStream::CStream* stream){
        DiffuseColor = {stream->readUInt8() / 255, stream->readUInt8() / 255, stream->readUInt8() / 255, stream->readUInt8() / 255};
        Unknown = stream->readUInt16();
        AlphaFlag = stream->readUInt8();
        TevStageCount = stream->readUInt8();
        Unknown2 = stream->readUInt8();
        stream->readBytesTo(Padding, sizeof(Padding));

        for(int i = 0; i < 8; i++){
            TevStages.push_back({
                stream->readUInt16(),
                stream->readUInt16(),
                stream->readFloat(),
                stream->readFloat(),
                stream->readFloat(),
                stream->readFloat(),
                stream->readFloat(),
                stream->readFloat(),
                stream->readFloat()
            });
        }

    }

    void Material::Save(bStream::CStream* stream){
        stream->writeUInt8(DiffuseColor.r * 255);
        stream->writeUInt8(DiffuseColor.g * 255);
        stream->writeUInt8(DiffuseColor.b * 255);
        stream->writeUInt8(DiffuseColor.a * 255);
        stream->writeUInt16(Unknown);
        stream->writeUInt8(AlphaFlag);
        stream->writeUInt8(TevStageCount);
        stream->writeUInt8(Unknown2);
        stream->readBytesTo(Padding, sizeof(Padding));

        for(int i = 0; i < 8; i++){
            stream->writeUInt16(TevStages[i].Unknown);
            stream->writeUInt16(TevStages[i].SamplerIndex);
            stream->writeFloat(TevStages[i].UnknownFloats[0]);
            stream->writeFloat(TevStages[i].UnknownFloats[1]);
            stream->writeFloat(TevStages[i].UnknownFloats[2]);
            stream->writeFloat(TevStages[i].UnknownFloats[3]);
            stream->writeFloat(TevStages[i].UnknownFloats[4]);
            stream->writeFloat(TevStages[i].UnknownFloats[5]);
            stream->writeFloat(TevStages[i].UnknownFloats[6]);
        }

    }

    void DrawElement::Read(bStream::CStream* stream){
        MaterialIndex = stream->readUInt16();
        ShapeIndex = stream->readUInt16();
    }

    void DrawElement::Save(bStream::CStream* stream){
        stream->writeUInt16(MaterialIndex);
        stream->writeUInt16(ShapeIndex);
    }

    void Shape::Read(bStream::CStream* stream){
        NormalFlag = stream->readUInt8();
        Unknown = stream->readUInt8();
        SurfaceFlags = stream->readUInt8();
        UnknownFlag = stream->readUInt8();
        PacketCount = stream->readUInt16();
        PacketBeginIndex = stream->readUInt16();
    }

    void Shape::Save(bStream::CStream* stream){
        stream->writeUInt8(NormalFlag);
        stream->writeUInt8(Unknown);
        stream->writeUInt8(SurfaceFlags);
        stream->writeUInt8(UnknownFlag);
        stream->writeUInt16(PacketCount);
        stream->writeUInt16(PacketBeginIndex);
    }

    void Packet::Read(bStream::CStream* stream){
        DataOffset = stream->readUInt32();
        DataSize = stream->readUInt32();
        Unknown = stream->readUInt16();
        MatrixCount = stream->readUInt16();
        for(int i = 0; i < 10; i++) MatrixIndices[i] = stream->readUInt16();
    }

    void Packet::Save(bStream::CStream* stream){
        stream->writeUInt32(DataOffset);
        stream->writeUInt32(DataSize);
        stream->writeUInt16(Unknown);
        stream->writeUInt16(MatrixCount);
        for(int i = 0; i < 10; i++) stream->writeUInt16(MatrixIndices[i]);
    }    

    void TextureHeader::Read(bStream::CStream* stream){

        uint32_t offset = stream->readUInt32();
        uint32_t returnOffset = stream->tell();
        stream->seek(offset);

        Format = stream->readUInt8();
        Padding = stream->readUInt8();
        Width = stream->readUInt16();
        Height = stream->readUInt16();
        stream->readBytesTo(Padding2, sizeof(Padding2));

        glGenTextures(1, &TextureID);
        glBindTexture(GL_TEXTURE_2D, TextureID);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


        ImageData = new uint8_t[Width * Height * 4]{0};

        //todo read image
        switch (Format)
        {
        case 0x07:
            ImageFormat::Decode::RGB565(stream, Width, Height, ImageData);
            break;
        case 0x08:
            ImageFormat::Decode::RGB5A3(stream, Width, Height, ImageData);
            break;
        case 0x0A:
            ImageFormat::Decode::CMPR(stream, Width, Height, ImageData);
            break;
        case 0x03:
            ImageFormat::Decode::I4(stream, Width, Height, ImageData);
            break;
        case 0x04:
            ImageFormat::Decode::I8(stream, Width, Height, ImageData);
            break;
        default:
            std::cout << "[MDL Loader]: No Decoder for 0x" << (int)Format << std::endl;
            break;
        }

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, ImageData);

        glBindTexture(GL_TEXTURE_2D, 0);

        //delete[] imageData;
        stream->seek(returnOffset);

        //Loaded = true;
    }

    void TextureHeader::Save(bStream::CStream* stream){
        stream->writeUInt8(Format);
        stream->writeUInt8(Padding);
        stream->writeUInt16(Width);
        stream->writeUInt16(Height);
        stream->writeBytes(Padding2, sizeof(Padding2));

        switch (Format)
        {
        case 0x07:
            ImageFormat::Encode::RGB565(stream, Width, Height, ImageData);
            break;
        case 0x08:
            ImageFormat::Encode::RGB5A3(stream, Width, Height, ImageData);
            break;
        case 0x0A:
            ImageFormat::Encode::CMPR(stream, Width, Height, ImageData);
            break;
        case 0x03:
            ImageFormat::Encode::I4(stream, Width, Height, ImageData);
            break;
        case 0x04:
            ImageFormat::Encode::I8(stream, Width, Height, ImageData);
            break;
        default:
            std::cout << "[MDL Loader]: No Encoder for 0x" << (int)Format << std::endl;
            break;
        }

    }

    void TextureHeader::Destroy(){
        if(ImageData != nullptr){
            delete[] ImageData;
            ImageData = nullptr;
        }
        glDeleteTextures(1, &TextureID);
    }

    void Shape::Destroy(){
        glDeleteVertexArrays(1, &Vao);
        glDeleteBuffers(1, &Vbo);
    }

    const char* default_mdl_vtx_shader_source = "#version 460\n\
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
        uniform mat4 joints[255];\n\
        \
        layout(location = 0) in vec3 inPosition;\n\
        layout(location = 1) in vec3 inNormal;\n\
        layout(location = 2) in vec4 inColor;\n\
        layout(location = 3) in vec2 inTexCoord;\n\
        layout(location = 4) in ivec4 inJoints;\n\
        layout(location = 5) in vec4 inWeights;\n\
        \
        layout(location = 0) out vec2 fragTexCoord;\n\
        layout(location = 1) out vec4 fragColor;\n\
        \
        void main()\n\
        {\
            vec4 pos = vec4(0.0);\n\
            for(int i = 0; i < 4; i++){\n\
                if(inJoints[i] == -1){\n\
                    break;\n\
                }\n\
                pos += (joints[inJoints[i]] * vec4(inPosition.x, inPosition.y, inPosition.z, 1.0)) * inWeights[i];\n\
            }\n\
            if(inJoints[0] == -1){\n\
                pos = vec4(inPosition.x, inPosition.y, inPosition.z, 1.0);\n\
            }\n\
            gl_Position = Proj * View * transform * vec4(pos.z, pos.y, pos.x, 1.0);\n\
            fragTexCoord = inTexCoord;\n\
            fragColor = inColor;\n\
        }\
    ";

    const char* default_mdl_frg_shader_source = "#version 460\n\
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
        char glErrorLogBuffer[4096];
        GLuint vs = glCreateShader(GL_VERTEX_SHADER);
        GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(vs, 1, &default_mdl_vtx_shader_source, nullptr);
        glShaderSource(fs, 1, &default_mdl_frg_shader_source, nullptr);
        glCompileShader(vs);
        GLint status;
        glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
        if(status == GL_FALSE){
            GLint infoLogLength;
            glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &infoLogLength);
            glGetShaderInfoLog(vs, infoLogLength, nullptr, glErrorLogBuffer);
            printf("[MDL Loader]: Compile failure in mdl vertex shader:\n%s\n", glErrorLogBuffer);
        }
        glCompileShader(fs);
        glGetShaderiv(fs, GL_COMPILE_STATUS, &status);
        if(status == GL_FALSE){
            GLint infoLogLength;
            glGetShaderiv(fs, GL_INFO_LOG_LENGTH, &infoLogLength);
            glGetShaderInfoLog(fs, infoLogLength, nullptr, glErrorLogBuffer);
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
            glGetProgramInfoLog(mProgram, logLen, nullptr, glErrorLogBuffer);
            printf("[MDL Loader]: Shader Program Linking Error:\n%s\n", glErrorLogBuffer);
        }
        glDetachShader(mProgram, vs);
        glDetachShader(mProgram, fs);
        glDeleteShader(vs);
        glDeleteShader(fs);
    }

    template<class T>
    std::map<int, T> ReadSection(bStream::CStream* stream, uint32_t offset, uint16_t count){
        std::map<int, T> collection;

        stream->seek(LGenUtility::SwapEndian<uint32_t>(offset));
        for (size_t i = 0; i < LGenUtility::SwapEndian<uint16_t>(count); i++)
        {
            T item;
            item.Read(stream);
            collection[i] = item;
        }

        return collection;
    };

    template<class T>
    uint32_t WriteSection(bStream::CStream* stream, std::map<int, T>& items, std::size_t itemSize){
        offset = stream->tell()
        for (auto [index, item] : items)
        {
            stream->seek(offset + (index * itemSize));
            item.Write(stream);
        }
        return offset;
    };
    
    void Model::Save(bStream::CStream* stream){
        
        std::vector<glm::vec3> newPositions;
        std::vector<glm::vec3> newNormals;
        std::vector<glm::vec4> newColors;
        std::vector<glm::vec2> newTexCoords;
        
        mHeader.SceneGraphNodeCount = mGraphNodes.size();
        mHeader.PacketCount = mPackets.size();
        mHeader.WeightCount = mWeights.size();
        mHeader.JointCount = mMatrixTable.size();
        mHeader.PositionCount = newPositions.size();
        mHeader.NormalCount = newNormals.size();
        mHeader.ColorCount = newColors.size();
        mHeader.TexCoordCount = newTexCoords.size();
        mHeader.SamplerCount = mSamplers.size();
        mHeader.DrawElementCount = mDrawElements.size();
        mHeader.MaterialCount = mMaterials.size();
        mHeader.ShapeCount = mShapes.size();
        stream->seek(128); // go past header and it's padding
        
        uint32_t runningPacketIndex = 0;
        for(auto [idx, shape] : mShapes){
            for(int i = 0; i < shape.PacketCount; i++){
                std::size_t packetPos = stream->tell();
                
                for(Primitive& primitive : mPackets[i].Primitives){
                    stream->writeUInt8(primitive.Opcode);
                    stream->writeUInt16(primitive.Vertices.size());
                    for(auto vertex : primitive.Vertices){
                        stream->writeUInt8(vertex.Matrix);
                        
                        std::vector<glm::vec3>::iterator pos = std::find(newPositions.begin(), newPositions.end(), vertex.Position);
                        if(pos == newPositions.end()){
                            stream->writeUInt16(newPositions.size());
                            newPositions.push_back(vertex.Position);
                        } else {
                            stream->writeUInt16((pos - newPositions.begin()));
                        }
                        if(mNormals.size() > 0){
                            std::vector<glm::vec3>::iterator pos = std::find(newNormals.begin(), newNormals.end(), vertex.Normal);
                            if(pos == newNormals.end()){
                                stream->writeUInt16(newNormals.size());
                                newNormals.push_back(vertex.Normal);
                            } else {
                                stream->writeUInt16(pos - newNormals.begin());
                            }
                            if(shape.NormalFlag > 1){
                                pos = std::find(newNormals.begin(), newNormals.end(), vertex.Binormal);
                                if(pos == newNormals.end()){
                                    stream->writeUInt16(newNormals.size());
                                    newNormals.push_back(vertex.Binormal);
                                } else {
                                    stream->writeUInt16(pos - newNormals.begin());
                                }
                                pos = std::find(newNormals.begin(), newNormals.end(), vertex.Tangent);
                                if(pos == newNormals.end()){
                                    stream->writeUInt16(newNormals.size());
                                    newNormals.push_back(vertex.Tangent);
                                } else {
                                    stream->writeUInt16(pos - newNormals.begin());
                                }
                            }
                        }
                        if(mColors.size() > 0){
                            std::vector<glm::vec4>::iterator pos = std::find(newColors.begin(), newColors.end(), vertex.Color);
                            if(pos == newColors.end()){
                                stream->writeUInt16(newColors.size());
                                newColors.push_back(vertex.Color);
                            } else {
                                stream->writeUInt16(pos - newColors.begin());
                            }
                        }
                        if(mTexCoords.size() > 0){
                            std::vector<glm::vec2>::iterator pos = std::find(newTexCoords.begin(), newTexCoords.end(), vertex.Texcoord);
                            if(pos == newTexCoords.end()){
                                stream->writeUInt16(newTexCoords.size());
                                newTexCoords.push_back(vertex.Texcoord);
                            } else {
                                stream->writeUInt16(pos - newTexCoords.begin());
                            }
                        }
                    }
                }
                
                
                std::size_t packetEnd = stream->tell();
                
                mPackets[shape.PacketBeginIndex + i].DataOffset = packetPos;
                mPackets[shape.PacketBeginIndex + i].DataSize = packetEnd - packetPos;
            }
        }
        stream->alignTo(32);
        
        std::vector<uint32_t> textureOffsets; 
        for(auto [idx, texture] : mTextureHeaders){
            textureOffsets.push_back(stream->tell());
            texture.Save(stream);
        }

        mHeader.MaterialOffset = stream->tell();
        for(auto& [idx, material] : mMaterials){
            material.Save(stream);
        }

        mHeader.SamplerOffset = WriteSection<Sampler>(stream, mSamplers, 0x08);
        mHeader.ShapeOffset = WriteSection<Shape>(stream, mShapes, 0x08);
        mHeader.DrawElementOffset = WriteSection<DrawElement>(stream, mDrawElements, 0x04);
        mHeader.PacketOffset = WriteSection<Packet>(stream, mPackets, 0x20);

        mHeader.TextureOffsetArray = stream->tell();
        for(auto offset : textureOffsets){
            stream->writeUInt32(offset);
        }
        
        //positions

        // normals

        // colors

        // texcoords

        mHeader.SceneGraphOffset = WriteSection<SceneGraphNode>(stream, mGraphNodes, 0x10);
        
        mHeader.InverseMatrixOffset = stream->tell();
        for(int m = 0; m < mMatrixTable.size(); m++){
            glm::mat4 mtx = glm::inverseTranspose(mMatrixTable[m]);
            for (std::size_t r = 0; r < 3; r++){
                for (std::size_t c = 0; c < 4; c++){
                    stream->writeFloat(mtx[r][c]);
                }
            }           
        }
        
        mHeader.WeightOffset = stream->tell();
        for(int m = 0; m < mWeights.size(); m++){
            for(int w = 0; w < mWeights[m].Weights.size(); w++){
                stream->writeFloat(mWeights[m].Weights[w]);
            }
        }
        
        mHeader.JointCount = stream->tell();
        for(int m = 0; m < mWeights.size(); m++){
            for(int w = 0; w < mWeights[m].JointIndices.size(); w++){
                stream->writeUInt16(mWeights[m].JointIndices[w]);
            }
        }
        
        mHeader.WeightCountTableOffset = stream->tell();
        for(int m = 0; m < mWeights.size(); m++){
            stream->writeUInt8(mWeights[m].Weights.size());
        }

    }

    void Model::Load(bStream::CStream* stream){
        stream->readBytesTo((uint8_t*)&mHeader, sizeof(mHeader));

        mSkeletonRenderer.Init();

        std::cout << "[MDL Loader]: Reading Model Start" << std::endl;

        mTextureHeaders = ReadSection<TextureHeader>(stream, mHeader.TextureOffsetArray, mHeader.TextureCount);
        mSamplers = ReadSection<Sampler>(stream, mHeader.SamplerOffset, mHeader.SamplerCount);
        mShapes = ReadSection<Shape>(stream, mHeader.ShapeOffset, mHeader.ShapeCount);
        mPackets = ReadSection<Packet>(stream, mHeader.PacketOffset, mHeader.PacketCount);
        mDrawElements = ReadSection<DrawElement>(stream, mHeader.DrawElementOffset, mHeader.DrawElementCount);

        mMaterials = ReadSection<Material>(stream, mHeader.MaterialOffset, mHeader.MaterialCount);
        mGraphNodes = ReadSection<SceneGraphNode>(stream, mHeader.SceneGraphOffset, mHeader.SceneGraphNodeCount);

        stream->seek(LGenUtility::SwapEndian<uint32_t>(mHeader.PositionOffset));
        for (std::size_t i = 0; i < LGenUtility::SwapEndian<uint16_t>(mHeader.PositionCount); i++){
            mPositions.push_back({stream->readFloat(), stream->readFloat(), stream->readFloat()});

            bbMin.z = (mPositions.back().x < bbMin.x ? mPositions.back().x : bbMin.x);
            bbMin.y = (mPositions.back().y < bbMin.y ? mPositions.back().y : bbMin.y);
            bbMin.x = (mPositions.back().z < bbMin.z ? mPositions.back().z : bbMin.z);

            bbMax.z = (mPositions.back().x > bbMax.x ? mPositions.back().x : bbMax.x);
            bbMax.y = (mPositions.back().y > bbMax.y ? mPositions.back().y : bbMax.y);
            bbMax.x = (mPositions.back().z > bbMax.z ? mPositions.back().z : bbMax.z);
        }

        stream->seek(LGenUtility::SwapEndian<uint32_t>(mHeader.NormalsOffset));
        for (std::size_t i = 0; i < LGenUtility::SwapEndian<uint16_t>(mHeader.NormalCount); i++){
            mNormals.push_back({stream->readFloat(), stream->readFloat(), stream->readFloat()});
        }

        stream->seek(LGenUtility::SwapEndian<uint32_t>(mHeader.TexCoordsOffset));
        for (std::size_t i = 0; i < LGenUtility::SwapEndian<uint16_t>(mHeader.TexCoordCount); i++){
            mTexCoords.push_back({stream->readFloat(), stream->readFloat()});
        }

        stream->seek(LGenUtility::SwapEndian<uint32_t>(mHeader.ColorsOffset));
        for (std::size_t i = 0; i < LGenUtility::SwapEndian<uint16_t>(mHeader.ColorCount); i++){
            mColors.push_back({stream->readUInt8() / 255.0f, stream->readUInt8() / 255.0f, stream->readUInt8() / 255.0f, stream->readUInt8() / 255.0f});
        }

        mMatrixTable.resize(LGenUtility::SwapEndian<uint16_t>(mHeader.JointCount) + LGenUtility::SwapEndian<uint16_t>(mHeader.WeightCount));
        mSkeleton.resize(mMatrixTable.size());
        stream->seek(LGenUtility::SwapEndian<uint32_t>(mHeader.InverseMatrixOffset));
        for (std::size_t i = 0; i < LGenUtility::SwapEndian<uint16_t>(mHeader.JointCount); i++){
            mMatrixTable[i] = {
                    stream->readFloat(), stream->readFloat(), stream->readFloat(), stream->readFloat(),
                    stream->readFloat(), stream->readFloat(), stream->readFloat(), stream->readFloat(),
                    stream->readFloat(), stream->readFloat(), stream->readFloat(), stream->readFloat(),
                    0,                                     0,                   0,                   1
            };
            mMatrixTable[i] = glm::inverseTranspose(mMatrixTable[i]);
            mSkeleton[i].Model = mMatrixTable[i];
            mSkeleton[i].InverseModel = glm::inverse(mMatrixTable[i]);
        }

        BuildScenegraphSkeleton(0, -1);

        for(auto& bone : mSkeleton){ // Generate local matrices for each bone in the
            if(bone.Parent != nullptr){
                bone.Local = glm::inverse(bone.Parent->Transform()) * bone.Model;
            } else {
                bone.Local = glm::mat4(1.0f);
            }
        }

        InitSkeletonRenderer(0, -1);

        // This whole section of the code is broken I think, but it isn't needed unless skinning gets added

        mWeights.reserve(LGenUtility::SwapEndian<uint16_t>(mHeader.WeightCount));

        if(LGenUtility::SwapEndian<uint32_t>(mHeader.WeightCountTableOffset) != 0){
            stream->seek(LGenUtility::SwapEndian<uint32_t>(mHeader.WeightCountTableOffset));

            std::vector<uint8_t> weightCounters;
            mWeights.resize(LGenUtility::SwapEndian<uint16_t>(mHeader.WeightCount));
            for (std::size_t i = 0; i < LGenUtility::SwapEndian<uint16_t>(mHeader.WeightCount); i++){
                weightCounters.push_back(stream->readUInt8());
            }

            stream->seek(LGenUtility::SwapEndian<uint32_t>(mHeader.WeightOffset));
            for (std::size_t i = 0; i < weightCounters.size(); i++){
                mWeights[i].Weights.resize(weightCounters[i]);
                for (std::size_t j = 0; j < weightCounters[i]; j++){
                    mWeights[i].Weights[j] = stream->readFloat();
                }
            }

            stream->seek(LGenUtility::SwapEndian<uint32_t>(mHeader.JointIndexOffset));
            for (std::size_t i = 0; i < weightCounters.size(); i++){
                mWeights[i].JointIndices.resize(weightCounters[i]);
                for (std::size_t j = 0; j < weightCounters[i]; j++){
                    mWeights[i].JointIndices[j] = stream->readUInt16();
                }
            }
        }


        // Setup OpenGL Meshes for rendering
        for (auto [idx, drawElement] : mDrawElements){
            uint16_t matIndices[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

            std::vector<Vertex> primitives;
            
            for (std::size_t i = mShapes[drawElement.ShapeIndex].PacketBeginIndex; i < mShapes[drawElement.ShapeIndex].PacketBeginIndex + mShapes[drawElement.ShapeIndex].PacketCount; i++){
                //Read Packet
                Packet packet = mPackets[i];
                
                
                for (std::size_t m = 0; m < packet.MatrixCount; m++){
                    if(packet.MatrixIndices[m] == UINT16_MAX) continue;
                    matIndices[m] = packet.MatrixIndices[m];
                }

                std::vector<PrimitiveVertex> triangulated;
                
                stream->seek(packet.DataOffset);
                while(stream->tell() < packet.DataOffset + packet.DataSize){
                    uint8_t opcode = stream->readUInt8();
                    
                    if(opcode == 0) continue;

                    Primitive& primitive = packet.Primitives.emplace_back();
                    primitive.Opcode = opcode;
                    
                    std::vector<PrimitiveVertex> primitiveVertices;

                    uint16_t vertexCount = stream->readUInt16();
                    for(int v = 0; v < vertexCount; v++){
                        PrimitiveVertex vtx = {0};
                        Vertex& pvtx = primitive.Vertices.emplace_back();
                        //TODO: LOD Check
                        vtx.Matrix = stream->readInt8();
                        if(vtx.Matrix != -1){
                            vtx.Matrix = matIndices[vtx.Matrix / 3];
                        }

                        pvtx.Matrix = vtx.Matrix;

                        int8_t tex0MatrixIndex = stream->readInt8();
                        int8_t tex1MatrixIndex = stream->readInt8();

                        vtx.Position = stream->readInt16();
                        pvtx.Position = mPositions[vtx.Position];

                        if(LGenUtility::SwapEndian<uint16_t>(mHeader.NormalCount) > 0){
                            vtx.Normal = stream->readInt16();
                            pvtx.Normal = mNormals[vtx.Normal];
                        }

                        if(mShapes[drawElement.ShapeIndex].NormalFlag > 1){
                            vtx.Binormal = stream->readInt16();
                            vtx.Tangent = stream->readInt16();
                            pvtx.Binormal = mNormals[vtx.Binormal];
                            pvtx.Tangent = mNormals[vtx.Tangent];
                        }

                        if(LGenUtility::SwapEndian<uint16_t>(mHeader.ColorCount) > 0){
                            vtx.Color = stream->readInt16();
                            pvtx.Color = mColors[vtx.Color];
                        }

                        if(LGenUtility::SwapEndian<uint16_t>(mHeader.TexCoordCount) > 0){
                            vtx.Texcoord = stream->readInt16();
                            pvtx.Texcoord = mTexCoords[vtx.Texcoord];
                        }

                        primitiveVertices.push_back(vtx);
                    }

                    
                    switch (opcode){
                        case GXPrimitiveType::Triangles: {
                            for(PrimitiveVertex vtxIdx : primitiveVertices){
                                triangulated.push_back(vtxIdx);
                            }
                        }
                        break;
                        case GXPrimitiveType::TriangleStrip: {
                            for (std::size_t v = 2; v < primitiveVertices.size(); v++){
                                triangulated.push_back(primitiveVertices[v - 2]);
                                triangulated.push_back(primitiveVertices[(v % 2 != 0 ? v : v - 1)]);
                                triangulated.push_back(primitiveVertices[(v % 2 != 0 ? v - 1 : v)]);
                            }
                        }
                        break;
                        case GXPrimitiveType::TriangleFan:{
                            for(std::size_t v = 0; v < 3; v++){
                                triangulated.push_back(primitiveVertices[v]);
                            }
                            
                            for (std::size_t v = 2; v < primitiveVertices.size(); v++){
                                if(primitiveVertices[v].Position == primitiveVertices[v-1].Position ||
                                    primitiveVertices[v-1].Position == primitiveVertices[0].Position ||
                                    primitiveVertices[v].Position == primitiveVertices[0].Position){
                                        continue;
                                    }
                                
                                triangulated.push_back(primitiveVertices[0]);
                                triangulated.push_back(primitiveVertices[v-1]);
                                triangulated.push_back(primitiveVertices[v]);
                                
                            }
                        }
                        break;
                        default:
                        std::cout << "[MDL Loader]: Unimplemented primitive " << std::format("{0}", opcode) << std::endl;
                        break;
                    }
                }
                
                for(PrimitiveVertex vtxIndices : triangulated){
                    Vertex vtx = {{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {1,1,1,1}, {-1, -1, -1, -1}, {0, 0, 0, 0}, {0,0}, {0,0}};

                    if(vtxIndices.Matrix >= LGenUtility::SwapEndian<uint16_t>(mHeader.JointCount)){
                        uint16_t weightIdx = vtxIndices.Matrix - LGenUtility::SwapEndian<uint16_t>(mHeader.JointCount);
                        Weight& weight = mWeights[weightIdx];
                        for(int i = 0; i < 4; i++){
                            if(i >= weight.JointIndices.size()) break;
                            vtx.BoneIndices[i] = weight.JointIndices[i];
                            vtx.Weights[i] = weight.Weights[i];
                        }
                        vtx.Position = mPositions[vtxIndices.Position];
                    } else if(vtxIndices.Matrix > -1 && vtxIndices.Matrix < LGenUtility::SwapEndian<uint16_t>(mHeader.JointCount)) {
                        glm::mat4 mtx = mMatrixTable[vtxIndices.Matrix];
                        vtx.Position = glm::vec3(mtx * glm::vec4(mPositions[vtxIndices.Position], 1.0f));
                        vtx.BoneIndices[0] = vtxIndices.Matrix;
                        vtx.Weights[0] = 1.0f;

                    }
                    
                    if(mHeader.NormalCount != 0) vtx.Normal = mNormals[vtxIndices.Normal];
                    if(mHeader.TexCoordCount != 0) vtx.Texcoord = mTexCoords[vtxIndices.Texcoord];
                    if(mHeader.ColorCount != 0) vtx.Color = mColors[vtxIndices.Color];
                    
                    primitives.push_back(vtx);
                }
                
            }
            
            glGenVertexArrays(1, &mShapes[drawElement.ShapeIndex].Vao);
            glBindVertexArray(mShapes[drawElement.ShapeIndex].Vao);
                
            glGenBuffers(1, &mShapes[drawElement.ShapeIndex].Vbo);
            glBindBuffer(GL_ARRAY_BUFFER, mShapes[drawElement.ShapeIndex].Vbo);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));

            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Color));

            glEnableVertexAttribArray(3);
            glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Texcoord));
            
            glEnableVertexAttribArray(4);
            glVertexAttribIPointer(4, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, BoneIndices));

            glEnableVertexAttribArray(5);
            glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Weights));

            glBufferData(GL_ARRAY_BUFFER, primitives.size() * sizeof(Vertex), primitives.data(), GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);

            mShapes[drawElement.ShapeIndex].VertexCount = primitives.size();

        }
    }

    void Model::BuildScenegraphSkeleton(uint32_t index, uint32_t parentIndex){
        auto node = mGraphNodes[index];
        
        mSkeleton[index].ParentIndex = parentIndex;
        if(parentIndex != -1){
            mSkeleton[index].Parent = &mSkeleton[parentIndex];
        }
        
        if(node.ChildIndexShift > 0){
            BuildScenegraphSkeleton(index + node.ChildIndexShift, index);
        }
        
        if(node.SiblingIndexShift > 0){
            BuildScenegraphSkeleton(index + node.SiblingIndexShift, parentIndex);
        }
        
    }

    void Model::InitSkeletonRenderer(uint32_t index, uint32_t parentIndex){
        auto node = mGraphNodes[index];
        
        if(parentIndex != -1){
            mSkeletonRenderer.mPaths.push_back({
                { { glm::vec3(mMatrixTable[index][3].z, mMatrixTable[index][3].y, mMatrixTable[index][3].x) }, {0xFF, 0x00, 0xFF, 0xFF}, 6400, -1 },
                { { glm::vec3(mMatrixTable[parentIndex][3].z, mMatrixTable[parentIndex][3].y, mMatrixTable[parentIndex][3].x) }, {0xFF, 0x00, 0xFF, 0xFF}, 6400, -1 }
            });
        } else {
            mSkeletonRenderer.mPaths.push_back({{{ glm::vec3(mMatrixTable[index][3].z, mMatrixTable[index][3].y, mMatrixTable[index][3].x) }, {0xFF, 0x00, 0xFF, 0xFF}, 6400, -1 }});
        }
        
        if(node.ChildIndexShift > 0){
            InitSkeletonRenderer(index + node.ChildIndexShift, index);
        }
        
        if(node.SiblingIndexShift > 0){
            InitSkeletonRenderer(index + node.SiblingIndexShift, parentIndex);
        }

        if(index == 0) mSkeletonRenderer.UpdateData();
    }

    void Model::Draw(glm::mat4* transform, int32_t id, bool selected, TXP::Animation* materialAnimtion, Animation* skeletalAnimation){

        std::vector<glm::mat4> skeleton(mSkeleton.size());
        mSkeletonRenderer.mPaths.clear();        
        for(int i = 0; i < skeleton.size(); i++){
            if(mSkeleton[i].ParentIndex != -1){
                skeleton[i] = skeleton[mSkeleton[i].ParentIndex] * (skeletalAnimation != nullptr && i < skeletalAnimation->mJointAnimations.size() ? skeletalAnimation->GetJoint(mSkeleton[i].Local, i) : mSkeleton[i].Local);
                mSkeletonRenderer.mPaths.push_back({
                    { glm::vec3(skeleton[mSkeleton[i].ParentIndex][3].z, skeleton[mSkeleton[i].ParentIndex][3].y, skeleton[mSkeleton[i].ParentIndex][3].x), {0xFF, 0x00, 0xFF, 0xFF}, 6400, -1 },
                    { glm::vec3(skeleton[i][3].z, skeleton[i][3].y, skeleton[i][3].x), {0xFF, 0x00, 0xFF, 0xFF}, 6400, -1 }
                });
            } else {
                skeleton[i] = (skeletalAnimation != nullptr && i < skeletalAnimation->mJointAnimations.size() ? skeletalAnimation->GetJoint(mSkeleton[i].Local, i) : mSkeleton[i].Local);
                mSkeletonRenderer.mPaths.push_back({
                    { glm::vec3(skeleton[i][3].z, skeleton[i][3].y, skeleton[i][3].x), {0xFF, 0x00, 0xFF, 0xFF}, 12800, -1 },
                    { glm::vec3(skeleton[i][3].z, skeleton[i][3].y, skeleton[i][3].x), {0xFF, 0x00, 0xFF, 0xFF}, 12800, -1 }
                });
            }
        }
        mSkeletonRenderer.UpdateData();
        for(int i = 0; i < skeleton.size(); i++){
            skeleton[i] = skeleton[i] * mSkeleton[i].InverseModel;
        }

        glUseProgram(mProgram);
        glUniformMatrix4fv(glGetUniformLocation(mProgram, "transform"), 1, 0, &(*transform)[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(mProgram, "joints"), skeleton.size(), 0, &skeleton[0][0][0]);
        glUniform1i(glGetUniformLocation(mProgram, "pickID"), id);
        glUniform1i(glGetUniformLocation(mProgram, "selected"), selected);

        for (auto [idx, element] : mDrawElements){
            if(mShapes[element.ShapeIndex].VertexCount == 0) continue;

            Sampler sampler;
            if(mMaterials[element.MaterialIndex].TevStages[0].SamplerIndex < mSamplers.size()){
                sampler = mSamplers[mMaterials[element.MaterialIndex].TevStages[0].SamplerIndex];
            }

            if(materialAnimtion != nullptr){
                uint32_t updatedSampler = materialAnimtion->GetSamplerIndex(element.MaterialIndex);
                if(updatedSampler != UINT32_MAX) sampler = mSamplers[updatedSampler];
            }

            glBindVertexArray(mShapes[element.ShapeIndex].Vao);

            glActiveTexture(GL_TEXTURE0);

            if(mTextureHeaders.size() > 0){
                if(sampler.TextureIndex != -1 && sampler.TextureIndex < mTextureHeaders.size() && mTextureHeaders[sampler.TextureIndex].TextureID != UINT32_MAX){
                    glBindTexture(GL_TEXTURE_2D, mTextureHeaders[sampler.TextureIndex].TextureID);
                } else {
                    glBindTexture(GL_TEXTURE_2D, mTextureHeaders[0].TextureID);
                }
            }

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (sampler.WrapU == 2 ? GL_MIRRORED_REPEAT : GL_REPEAT));
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (sampler.WrapV == 2 ? GL_MIRRORED_REPEAT : GL_REPEAT));

            glDrawArrays(GL_TRIANGLES, 0, mShapes[element.ShapeIndex].VertexCount);
        }
    }

    Model::~Model(){
        for(auto [idx, shape] : mShapes){
            shape.Destroy();
        }

        for(auto [idx, texture] : mTextureHeaders){
            texture.Destroy();
        }
    }

    void Animation::ResetTracks(){
        for(auto& joint : mJointAnimations){
            joint.mPreviousScaleKeyX = 0;
            joint.mPreviousScaleKeyY = 0;
            joint.mPreviousScaleKeyZ = 0;
            joint.mPreviousRotKeyX = 0;
            joint.mPreviousRotKeyY = 0;
            joint.mPreviousRotKeyZ = 0;
            joint.mPreviousPosKeyX = 0;
            joint.mPreviousPosKeyY = 0;
            joint.mPreviousPosKeyZ = 0;        
        
            joint.mNextScaleKeyX = 1;
            joint.mNextScaleKeyY = 1;
            joint.mNextScaleKeyZ = 1;
            joint.mNextRotKeyX = 1;
            joint.mNextRotKeyY = 1;
            joint.mNextRotKeyZ = 1;
            joint.mNextPosKeyX = 1;
            joint.mNextPosKeyY = 1;
            joint.mNextPosKeyZ = 1;
        }
    }

    glm::mat4 Animation::GetJoint(glm::mat4 local, uint32_t id){
        if(id >= mJointAnimations.size()) return local;
        //std::cout << "Id is " << id << std::endl;
        JointTrack& joint = mJointAnimations[id];
        glm::mat4 keyframe(1.0f);

        glm::vec3 translation, scale, skew;
        glm::vec4 persp;
        glm::quat rot;

        glm::decompose(local, scale, rot, translation, skew, persp);
        rot = glm::conjugate(rot);

        glm::vec3 rotEuler = glm::eulerAngles(rot);
        
        if(joint.ScaleX.mKeys.size() > 0) scale.x = MixTrack(joint.ScaleX, mTime, joint.mPreviousScaleKeyX, joint.mNextScaleKeyX);
        if(joint.ScaleY.mKeys.size() > 0) scale.y = MixTrack(joint.ScaleY, mTime, joint.mPreviousScaleKeyY, joint.mNextScaleKeyY);
        if(joint.ScaleZ.mKeys.size() > 0) scale.z = MixTrack(joint.ScaleZ, mTime, joint.mPreviousScaleKeyZ, joint.mNextScaleKeyZ);

        if(joint.RotationX.mKeys.size() > 0) rotEuler.x = glm::radians(MixTrack(joint.RotationX, mTime, joint.mPreviousRotKeyX, joint.mNextRotKeyX));
        if(joint.RotationY.mKeys.size() > 0) rotEuler.y = glm::radians(MixTrack(joint.RotationY, mTime, joint.mPreviousRotKeyY, joint.mNextRotKeyY));
        if(joint.RotationZ.mKeys.size() > 0) rotEuler.z = glm::radians(MixTrack(joint.RotationZ, mTime, joint.mPreviousRotKeyZ, joint.mNextRotKeyZ));

        if(joint.PositionX.mKeys.size() > 0) translation.x = MixTrack(joint.PositionX, mTime, joint.mPreviousPosKeyX, joint.mNextPosKeyX);
        if(joint.PositionY.mKeys.size() > 0) translation.y = MixTrack(joint.PositionY, mTime, joint.mPreviousPosKeyY, joint.mNextPosKeyY);
        if(joint.PositionZ.mKeys.size() > 0) translation.z = MixTrack(joint.PositionZ, mTime, joint.mPreviousPosKeyZ, joint.mNextPosKeyZ);

        keyframe = glm::scale(keyframe, scale);
        keyframe *= glm::toMat4(glm::quat(rotEuler));
        keyframe = glm::translate(keyframe, translation);

        return keyframe;
    }

    void Animation::Load(bStream::CStream* stream){
        uint32_t jointCount = stream->readUInt32();
        std::cout << std::format("Joint Count: {}", jointCount) << std::endl;
        uint16_t frameCount = stream->readUInt16();
        uint16_t frameSpeed = stream->readUInt16();
        uint32_t flags = stream->readUInt32();

        mSpeed = frameSpeed;
        mEnd = frameCount;
        
        uint32_t scaleKeyframeOffset = stream->readUInt32();
        uint32_t rotationKeyframeOffset = stream->readUInt32();
        uint32_t positionKeyframeOffset = stream->readUInt32();

        uint32_t keyStartOffset = stream->readUInt32();
        uint32_t keyCountOffset = stream->readUInt32();


        std::vector<std::array<uint32_t, 9>> beginIndices;
        std::vector<std::array<std::pair<uint8_t, uint8_t>, 9>> trackFlags;

        stream->seek(keyStartOffset);
        for(uint32_t i = 0; i < jointCount; i++){
            std::array<uint32_t, 9> indices;
            for(uint32_t j = 0; j < 9; j++) indices[j] = stream->readUInt32();
            beginIndices.push_back(indices);
        }
        stream->seek(keyCountOffset);
        for(uint32_t i = 0; i < jointCount; i++){
            std::array<std::pair<uint8_t, uint8_t>, 9> flags;
            for(uint32_t j = 0; j < 9; j++){
                flags[j].first = stream->readUInt8();
                flags[j].second = stream->readUInt8();
            }
            trackFlags.push_back(flags);
        }
        
        mJointAnimations.resize(jointCount);

        for(int j = 0; j < jointCount; j++){
            JointTrack& joint = mJointAnimations[j];
            joint.ScaleX.LoadTrackEx(stream, scaleKeyframeOffset, beginIndices[j][0], trackFlags[j][0].second, true, trackFlags[j][0].first == 0x80);
            joint.ScaleY.LoadTrackEx(stream, scaleKeyframeOffset, beginIndices[j][1], trackFlags[j][1].second, true, trackFlags[j][1].first == 0x80);
            joint.ScaleZ.LoadTrackEx(stream, scaleKeyframeOffset, beginIndices[j][2], trackFlags[j][2].second, true, trackFlags[j][2].first == 0x80);

            joint.RotationX.LoadTrackEx(stream, rotationKeyframeOffset, beginIndices[j][3], trackFlags[j][3].second, true, trackFlags[j][3].first == 0x80, 2);
            joint.RotationY.LoadTrackEx(stream, rotationKeyframeOffset, beginIndices[j][4], trackFlags[j][4].second, true, trackFlags[j][4].first == 0x80, 2);
            joint.RotationZ.LoadTrackEx(stream, rotationKeyframeOffset, beginIndices[j][5], trackFlags[j][5].second, true, trackFlags[j][5].first == 0x80, 2);
        
            joint.PositionX.LoadTrackEx(stream, positionKeyframeOffset, beginIndices[j][6], trackFlags[j][6].second, true, trackFlags[j][6].first == 0x80);
            joint.PositionY.LoadTrackEx(stream, positionKeyframeOffset, beginIndices[j][7], trackFlags[j][7].second, true, trackFlags[j][7].first == 0x80);
            joint.PositionZ.LoadTrackEx(stream, positionKeyframeOffset, beginIndices[j][8], trackFlags[j][8].second, true, trackFlags[j][8].first == 0x80);
        }
    }
};
