#include "io/MdlIo.hpp"
#include "io/BinIO.hpp"
#include <glad/glad.h>
#include <glm/gtc/matrix_inverse.hpp>

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
        DiffuseColor = stream->readUInt32();
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
            BinMaterial::DecodeRGB565(stream, Width, Height, imageData);
            break;
        case 0x08:
            BinMaterial::DecodeRGB5A3(stream, Width, Height, imageData);
            break;
        case 0x0A:
            BinMaterial::DecodeCMPR(stream, Width, Height, imageData);
            break;

        default:
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

    const char* default_mdl_frg_shader_source = "#version 460\n\
        #extension GL_ARB_separate_shader_objects : enable\n\
        \
        uniform sampler2D texSampler;\n\
        \
        layout(location = 0) in vec2 fragTexCoord;\n\
        layout(location = 1) in vec4 fragColor;\n\
        \
        layout(location = 0) out vec4 outColor;\n\
        \
        void main()\n\
        {\n\
            vec4 baseColor = texture(texSampler, vec2(fragTexCoord.x, fragTexCoord.y));\n\
            outColor = baseColor; // * fragColor; //vec4(1.0, 1.0, 1.0, 1.0);\n\
        }\
    ";
    
    void DestroyShaders(){
        glDeleteProgram(mProgram);
    }

    void InitShaders(){
        char glErrorLogBuffer[4096];
        GLuint vs = glCreateShader(GL_VERTEX_SHADER);
        GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(vs, 1, &default_mdl_vtx_shader_source, NULL);
        glShaderSource(fs, 1, &default_mdl_frg_shader_source, NULL);
        glCompileShader(vs);
        GLint status;
        glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
        if(status == GL_FALSE){
            GLint infoLogLength;
            glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &infoLogLength);
            glGetShaderInfoLog(vs, infoLogLength, NULL, glErrorLogBuffer);
            printf("Compile failure in mdl vertex shader:\n%s\n", glErrorLogBuffer);
        }
        glCompileShader(fs);
        glGetShaderiv(fs, GL_COMPILE_STATUS, &status);
        if(status == GL_FALSE){
            GLint infoLogLength;
            glGetShaderiv(fs, GL_INFO_LOG_LENGTH, &infoLogLength);
            glGetShaderInfoLog(fs, infoLogLength, NULL, glErrorLogBuffer);
            printf("Compile failure in mdl fragment shader:\n%s\n", glErrorLogBuffer);
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
            printf("Shader Program Linking Error:\n%s\n", glErrorLogBuffer);
        } 
        glDetachShader(mProgram, vs);
        glDetachShader(mProgram, fs);
        glDeleteShader(vs);
        glDeleteShader(fs);
    }

};


void MDLModel::Load(bStream::CStream* stream){
    stream->readBytesTo((uint8_t*)&mHeader, sizeof(mHeader));

    std::cout << "Reading Model Start" << std::endl;

    mTexturesHeaders = MDL::ReadSection<MDL::TextureHeader>(stream, mHeader.TextureOffsetArray, mHeader.TextureCount);
    mSamplers = MDL::ReadSection<MDL::Sampler>(stream, mHeader.SamplerOffset, mHeader.SamplerCount);
    mShapes = MDL::ReadSection<MDL::Shape>(stream, mHeader.ShapeOffset, mHeader.ShapeCount);
    mPackets = MDL::ReadSection<MDL::Packet>(stream, mHeader.PacketOffset, mHeader.PacketCount);
    mDrawElements = MDL::ReadSection<MDL::DrawElement>(stream, mHeader.DrawElementOffset, mHeader.DrawElementCount);
    
    mMaterials = MDL::ReadSection<MDL::Material>(stream, mHeader.MaterialOffset, mHeader.MaterialCount);
    mGraphNodes = MDL::ReadSection<MDL::SceneGraphNode>(stream, mHeader.SceneGraphOffset, mHeader.SceneGraphNodeCount);

    stream->seek(LGenUtility::SwapEndian<uint32_t>(mHeader.PositionOffset));
    for (size_t i = 0; i < LGenUtility::SwapEndian<uint16_t>(mHeader.PositionCount); i++){
        mPositions.push_back({stream->readFloat(), stream->readFloat(), stream->readFloat()});
    }
    
    stream->seek(LGenUtility::SwapEndian<uint32_t>(mHeader.NormalsOffset));
    for (size_t i = 0; i < LGenUtility::SwapEndian<uint16_t>(mHeader.NormalCount); i++){
        mNormals.push_back({stream->readFloat(), stream->readFloat(), stream->readFloat()});
    }

    stream->seek(LGenUtility::SwapEndian<uint32_t>(mHeader.TexCoordsOffset));
    for (size_t i = 0; i < LGenUtility::SwapEndian<uint16_t>(mHeader.TexCoordCount); i++){
        mTexCoords.push_back({stream->readFloat(), stream->readFloat()});
    }

    stream->seek(LGenUtility::SwapEndian<uint32_t>(mHeader.ColorsOffset));
    for (size_t i = 0; i < LGenUtility::SwapEndian<uint16_t>(mHeader.ColorCount); i++){
        mColors.push_back({stream->readUInt8() / 255.0f, stream->readUInt8() / 255.0f, stream->readUInt8() / 255.0f, stream->readUInt8() / 255.0f});
    }

    mMatrixTable.reserve(LGenUtility::SwapEndian<uint16_t>(mHeader.JointCount) + LGenUtility::SwapEndian<uint16_t>(mHeader.WeightCount));

    std::fill(mMatrixTable.begin(), mMatrixTable.end(), glm::mat4(1.0f));

    stream->seek(LGenUtility::SwapEndian<uint32_t>(mHeader.InverseMatrixOffset));
    for (size_t i = 0; i < LGenUtility::SwapEndian<uint16_t>(mHeader.JointCount); i++){
        mMatrixTable[i] = {
            stream->readFloat(), stream->readFloat(), stream->readFloat(), stream->readFloat(),
            stream->readFloat(), stream->readFloat(), stream->readFloat(), stream->readFloat(),
            stream->readFloat(), stream->readFloat(), stream->readFloat(), stream->readFloat(),
            0,                                     0,                   0,                   1
        };
        mMatrixTable[i] = glm::inverseTranspose(mMatrixTable[i]);
    }

    // This whole section of the code is broken I think, but it isn't needed unless skinning gets added
    
    //if(LGenUtility::SwapEndian<uint32_t>(mHeader.WeightCountTableOffset != 0)){
    //    stream->seek(LGenUtility::SwapEndian<uint32_t>(mHeader.WeightCountTableOffset));
    //
    //    std::vector<uint8_t> weightCounters;
    //    for (size_t i = 0; i < LGenUtility::SwapEndian<uint16_t>(mHeader.WeightCount); i++){
    //        weightCounters.push_back(stream->readUInt8());
    //    }
    //    
    //    stream->seek(LGenUtility::SwapEndian<uint32_t>(mHeader.WeightOffset));
    //    for (size_t i = 0; i < weightCounters.size(); i++){
    //        for (size_t j = 0; j < weightCounters[i]; j++){
    //            mWeights[i].Weights.push_back(stream->readFloat());
    //        }
    //    }
    //    
    //    stream->seek(LGenUtility::SwapEndian<uint32_t>(mHeader.JointIndexOffset));
    //    for (size_t i = 0; i < weightCounters.size(); i++){
    //        for (size_t j = 0; j < weightCounters[i]; j++){
    //            mWeights[i].JointIndices.push_back(stream->readUInt16());
    //        }
    //    }
    //}


    // Setup OpenGL Meshes for rendering
    for (MDL::DrawElement drawElement : mDrawElements){
        uint16_t matIndices[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

        std::vector<MDL::Vertex> triangulatedPrimitives;
        
        for (size_t i = mShapes[drawElement.ShapeIndex].PacketBeginIndex; i < mShapes[drawElement.ShapeIndex].PacketBeginIndex + mShapes[drawElement.ShapeIndex].PacketCount; i++){
            //Read Packet
            MDL::Packet packet = mPackets[i];


            for (size_t m = 0; m < packet.MatrixCount; m++){
                if(packet.MatrixIndices[m] == UINT16_MAX) continue;
                matIndices[m] = packet.MatrixIndices[m];
            }

            stream->seek(packet.DataOffset);
            while(stream->tell() < packet.DataOffset + packet.DataSize){
                uint8_t opcode = stream->readUInt8();

                if(opcode == 0) continue;

                std::vector<MDL::PrimitiveVertex> primitiveVertices;

                uint16_t vertexCount = stream->readUInt16();
                for(int v = 0; v < vertexCount; v++){
                    MDL::PrimitiveVertex vtx = {0};
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
                        int8_t prevMtx = -1;
                        for(MDL::PrimitiveVertex vtxIdx : primitiveVertices){
                            MDL::Vertex vtx = {{0,0,0}, {0,0,0}, {1,1,1,1}, {0,0}};
                            
                            int8_t mtxIdx = vtxIdx.Matrix == -1 ? prevMtx : vtxIdx.Matrix;
                            
                            vtx.Position = mPositions[vtxIdx.Position];
                            if(mHeader.NormalCount != 0) vtx.Normal = mNormals[vtxIdx.Normal];
                            if(mHeader.TexCoordCount != 0) vtx.Texcoord = mTexCoords[vtxIdx.Texcoord];
                            if(mHeader.ColorCount != 0) vtx.Color = mColors[vtxIdx.Color];

                            triangulatedPrimitives.push_back(vtx);
                        }
                    }
                    break;
                case GXPrimitiveType::TriangleStrip: {
                        for (size_t v = 2; v < primitiveVertices.size(); v++){
                            MDL::Vertex vtx1 = {{0,0,0}, {0,0,0}, {1,1,1,1}, {0,0}}, vtx2 = {{0,0,0}, {0,0,0}, {1,1,1,1}, {0,0}}, vtx3 = {{0,0,0}, {0,0,0}, {1,1,1,1}, {0,0}};

                            glm::mat4 vtx1Mat(1.0f), vtx2Mat(1.0f), vtx3Mat(1.0f);
                            if(primitiveVertices[v-2].Matrix != -1 && primitiveVertices[v-2].Matrix < LGenUtility::SwapEndian<uint16_t>(mHeader.JointCount)){
                                vtx1Mat = mMatrixTable[primitiveVertices[v-2].Matrix];
                            }
                            if(primitiveVertices[(v % 2 != 0 ? v : v-1)].Matrix != -1 && primitiveVertices[(v % 2 != 0 ? v : v-1)].Matrix < LGenUtility::SwapEndian<uint16_t>(mHeader.JointCount)){
                                vtx2Mat = mMatrixTable[primitiveVertices[(v % 2 != 0 ? v : v-1)].Matrix];
                            }
                            if(primitiveVertices[(v % 2 != 0 ? v-1 : v)].Matrix != -1 && primitiveVertices[(v % 2 != 0 ? v-1 : v)].Matrix < LGenUtility::SwapEndian<uint16_t>(mHeader.JointCount)){
                                vtx3Mat = mMatrixTable[primitiveVertices[(v % 2 != 0 ? v-1 : v)].Matrix];
                            }

                            vtx1.Position = glm::vec3(vtx1Mat * glm::vec4(mPositions[primitiveVertices[v-2].Position], 1.0f));
                            if(mHeader.NormalCount != 0) vtx1.Normal = mNormals[primitiveVertices[v-2].Normal];
                            if(mHeader.TexCoordCount != 0) vtx1.Texcoord = mTexCoords[primitiveVertices[v-2].Texcoord];
                            if(mHeader.ColorCount != 0) vtx1.Color = mColors[primitiveVertices[v-2].Color];

                            vtx2.Position = glm::vec3(vtx2Mat * glm::vec4(mPositions[primitiveVertices[(v % 2 != 0 ? v : v-1)].Position], 1.0f));
                            if(mHeader.NormalCount != 0) vtx2.Normal = mNormals[primitiveVertices[(v % 2 != 0 ? v : v-1)].Normal];
                            if(mHeader.TexCoordCount != 0) vtx2.Texcoord = mTexCoords[primitiveVertices[(v % 2 != 0 ? v : v-1)].Texcoord];
                            if(mHeader.ColorCount != 0) vtx2.Color = mColors[primitiveVertices[(v % 2 != 0 ? v : v-1)].Color];


                            vtx3.Position = glm::vec3(vtx3Mat * glm::vec4(mPositions[primitiveVertices[(v % 2 != 0 ? v-1 : v)].Position], 1.0f));
                            if(mHeader.NormalCount != 0) vtx3.Normal = mNormals[primitiveVertices[(v % 2 != 0 ? v-1 : v)].Normal];
                            if(mHeader.TexCoordCount != 0) vtx3.Texcoord = mTexCoords[primitiveVertices[(v % 2 != 0 ? v-1 : v)].Texcoord];
                            if(mHeader.ColorCount != 0) vtx3.Color = mColors[primitiveVertices[(v % 2 != 0 ? v-1 : v)].Color];

                            triangulatedPrimitives.push_back(vtx1);
                            triangulatedPrimitives.push_back(vtx2);
                            triangulatedPrimitives.push_back(vtx3);

                        }
                    }
                    break;
                
                default:
                    std::cout << "Oops unimplemented primitive type in mdl" << std::endl; 
                    break;
                }
            }

        }
        glGenVertexArrays(1, &mShapes[drawElement.ShapeIndex].Vao);
        glBindVertexArray(mShapes[drawElement.ShapeIndex].Vao);

        glGenBuffers(1, &mShapes[drawElement.ShapeIndex].Vbo);
        glBindBuffer(GL_ARRAY_BUFFER, mShapes[drawElement.ShapeIndex].Vbo);
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MDL::Vertex), (void*)offsetof(MDL::Vertex, Position));
        
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(MDL::Vertex), (void*)offsetof(MDL::Vertex, Normal));
        
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(MDL::Vertex), (void*)offsetof(MDL::Vertex, Color));
        
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(MDL::Vertex), (void*)offsetof(MDL::Vertex, Texcoord));
        
        glBufferData(GL_ARRAY_BUFFER, triangulatedPrimitives.size() * sizeof(MDL::Vertex), triangulatedPrimitives.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        
        mShapes[drawElement.ShapeIndex].VertexCount = triangulatedPrimitives.size();

    }


    std::cout << "Reading Model End" << std::endl;

}


void MDLModel::Draw(glm::mat4* transform){
    
    glUseProgram(MDL::mProgram);
    glUniformMatrix4fv(glGetUniformLocation(MDL::mProgram, "transform"), 1, 0, &(*transform)[0][0]);

    for (MDL::DrawElement element : mDrawElements){
        if(mShapes[element.ShapeIndex].VertexCount == 0) continue;

        MDL::Sampler sampler = mSamplers[mMaterials[element.MaterialIndex].TevStages[0].SamplerIndex];

        glBindVertexArray(mShapes[element.ShapeIndex].Vao);
        
        glActiveTexture(GL_TEXTURE0);

        if(mTexturesHeaders[sampler.TextureIndex].TextureID != UINT32_MAX){
            glBindTexture(GL_TEXTURE_2D, mTexturesHeaders[sampler.TextureIndex].TextureID);
        }

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (sampler.WrapU == 2 ? GL_MIRRORED_REPEAT : GL_REPEAT));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (sampler.WrapV == 2 ? GL_MIRRORED_REPEAT : GL_REPEAT));

        glDrawArrays(GL_TRIANGLES, 0, mShapes[element.ShapeIndex].VertexCount);
    
    }
}

MDLModel::~MDLModel(){
    for(MDL::Shape shape : mShapes){
        shape.Destroy();
    }

    for(MDL::TextureHeader texture : mTexturesHeaders){
        texture.Destroy();
    }
}