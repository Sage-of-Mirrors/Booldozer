#include "io/MdlIO.hpp"
#include "io/TxpIO.hpp"
#include <Bti.hpp>
#include <glad/glad.h>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <format>

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

    void Sampler::Read(bStream::CStream* stream){
        TextureIndex = stream->readUInt16();
        UnknownIndex = stream->readUInt16();
        WrapU = stream->readUInt8();
        WrapV = stream->readUInt8();
        Unknown1 = stream->readUInt8();
        Unknown2 = stream->readUInt8();
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

    void DrawElement::Read(bStream::CStream* stream){
        MaterialIndex = stream->readUInt16();
        ShapeIndex = stream->readUInt16();
    }

    void Shape::Read(bStream::CStream* stream){
        NormalFlag = stream->readUInt8();
        Unknown = stream->readUInt8();
        SurfaceFlags = stream->readUInt8();
        UnknownFlag = stream->readUInt8();
        PacketCount = stream->readUInt16();
        PacketBeginIndex = stream->readUInt16();
    }

    void Packet::Read(bStream::CStream* stream){
        DataOffset = stream->readUInt32();
        DataSize = stream->readUInt32();
        Unknown = stream->readUInt16();
        MatrixCount = stream->readUInt16();
        for(int i = 0; i < 10; i++) MatrixIndices[i] = stream->readUInt16();

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


        uint8_t* imageData = new uint8_t[Width * Height * 4]{};

        //todo read image
        switch (Format)
        {
        case 0x07:
            ImageFormat::Decode::RGB565(stream, Width, Height, imageData);
            break;
        case 0x08:
            ImageFormat::Decode::RGB5A3(stream, Width, Height, imageData);
            break;
        case 0x0A:
            ImageFormat::Decode::CMPR(stream, Width, Height, imageData);
            break;
        case 0x03:
            ImageFormat::Decode::I4(stream, Width, Height, imageData);
            break;
        case 0x04:
            ImageFormat::Decode::I8(stream, Width, Height, imageData);
            break;
        default:
            std::cout << "[MDL Loader]: No Decoder for 0x" << (int)Format << std::endl;
            break;
        }

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);

        glBindTexture(GL_TEXTURE_2D, 0);

        delete[] imageData;
        stream->seek(returnOffset);

        //Loaded = true;
    }

    void TextureHeader::Destroy(){
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
    std::vector<T> ReadSection(bStream::CStream* stream, uint32_t offset, uint16_t count){
        std::vector<T> collection;

        stream->seek(LGenUtility::SwapEndian<uint32_t>(offset));
        for (size_t i = 0; i < LGenUtility::SwapEndian<uint16_t>(count); i++)
        {
            T item;
            item.Read(stream);
            collection.push_back(item);
        }

        return collection;
    };


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

        stream->seek(LGenUtility::SwapEndian<uint32_t>(mHeader.InverseMatrixOffset));
        for (std::size_t i = 0; i < LGenUtility::SwapEndian<uint16_t>(mHeader.JointCount); i++){
            mMatrixTable[i] = {
                    stream->readFloat(), stream->readFloat(), stream->readFloat(), stream->readFloat(),
                    stream->readFloat(), stream->readFloat(), stream->readFloat(), stream->readFloat(),
                    stream->readFloat(), stream->readFloat(), stream->readFloat(), stream->readFloat(),
                    0,                                     0,                   0,                   1
            };
            mMatrixTable[i] = glm::inverseTranspose(mMatrixTable[i]);
            Bone bone { nullptr, glm::vec3(0.0f), glm::identity<glm::quat>(), glm::vec3(1.0f)};
            glm::vec3 skew;
            glm::vec4 persp;
            glm::decompose(mMatrixTable[i], bone.Scale, bone.Rotation, bone.Translation, skew, persp);
            mSkeleton.push_back(bone);
        }

        BuildScenegraphSkeleton(0, -1);
        ConvertBonesToLocalSpace();
        ConvertBonesToWorldSpace();
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
        for (DrawElement drawElement : mDrawElements){
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
                    
                    std::vector<PrimitiveVertex> primitiveVertices;

                    uint16_t vertexCount = stream->readUInt16();
                    for(int v = 0; v < vertexCount; v++){
                        PrimitiveVertex vtx = {0};
                        //TODO: LOD Check
                        vtx.Matrix = stream->readInt8();
                        if(vtx.Matrix != -1){
                            vtx.Matrix = matIndices[vtx.Matrix / 3];
                        }

                        int8_t tex0MatrixIndex = stream->readInt8();
                        int8_t tex1MatrixIndex = stream->readInt8();

                        vtx.Position = stream->readInt16();

                        if(LGenUtility::SwapEndian<uint16_t>(mHeader.NormalCount) > 0){
                            vtx.Normal = stream->readInt16();
                        }

                        if(mShapes[drawElement.ShapeIndex].NormalFlag > 1){
                            stream->readInt16();
                            stream->readInt16();
                        }

                        if(LGenUtility::SwapEndian<uint16_t>(mHeader.ColorCount) > 0){
                            vtx.Color = stream->readInt16();
                        }

                        if(LGenUtility::SwapEndian<uint16_t>(mHeader.TexCoordCount) > 0){
                            vtx.Texcoord = stream->readInt16();
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
                        //vtx.BoneIndices[0] = vtxIndices.Matrix;
                        //vtx.Weights[0] = 1.0f;

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

    void Model::ConvertBonesToLocalSpace(){
        for(auto& bone : mSkeleton){
            if(bone.Parent != nullptr){
                glm::mat4 transform = bone.Transform() * glm::inverse(bone.Parent->GetTransform());
                glm::vec3 scale, skew;
                glm::vec4 persp;
                glm::decompose(transform, scale, bone.Rotation, bone.Translation, skew, persp);
            }
        }
    }

    void Model::ConvertBonesToWorldSpace(){
        for(auto& bone : mSkeleton){
            if(bone.Parent != nullptr){
                glm::mat4 transform = bone.Transform() * bone.Parent->GetTransform();
                glm::vec3 scale, skew;
                glm::vec4 persp;
                glm::decompose(transform, scale, bone.Rotation, bone.Translation, skew, persp);
            }
        }
    }

    void Model::BuildScenegraphSkeleton(uint32_t index, uint32_t parentIndex){
        auto node = mGraphNodes[index];
        
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
                { { mSkeleton[index].Translation.z, mSkeleton[index].Translation.y, mSkeleton[index].Translation.x }, {0xFF, 0x00, 0xFF, 0xFF}, 6400, -1 },
                { { mSkeleton[parentIndex].Translation.z, mSkeleton[parentIndex].Translation.y, mSkeleton[parentIndex].Translation.x }, {0xFF, 0x00, 0xFF, 0xFF}, 6400, -1 }
            });
        }
        
        if(node.ChildIndexShift > 0){
            InitSkeletonRenderer(index + node.ChildIndexShift, index);
        }
        
        if(node.SiblingIndexShift > 0){
            InitSkeletonRenderer(index + node.SiblingIndexShift, parentIndex);
        }

        if(index == 0) mSkeletonRenderer.UpdateData();
    }

    void Model::Draw(glm::mat4* transform, int32_t id, bool selected, TXP::Animation* materialAnimtion){

        std::vector<glm::mat4> skeleton;
        for(int i = 0; i < mSkeleton.size(); i++) skeleton.push_back(mSkeleton[i].Transform());

        glUseProgram(mProgram);
        glUniformMatrix4fv(glGetUniformLocation(mProgram, "transform"), 1, 0, &(*transform)[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(mProgram, "joints"), skeleton.size(), 0, &skeleton[0][0][0]);
        glUniform1i(glGetUniformLocation(mProgram, "pickID"), id);
        glUniform1i(glGetUniformLocation(mProgram, "selected"), selected);

        for (DrawElement element : mDrawElements){
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
        for(Shape shape : mShapes){
            shape.Destroy();
        }

        for(TextureHeader texture : mTextureHeaders){
            texture.Destroy();
        }
    }
};
