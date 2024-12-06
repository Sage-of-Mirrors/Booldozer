#include <vector>
#include "glm/glm.hpp"
#include "glm/gtx/quaternion.hpp"
#include "io/BinIO.hpp"
#include <glad/glad.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <J3D/Texture/J3DTexture.hpp>
#include <GXGeometryEnums.hpp>
#include "io/BtiIO.hpp"
#include "GenUtil.hpp"

/*
*
* This file is full of hacks and duplicated code. I would like to do it properly later but for the time being, this works fine.
*
*/

// from https://github.com/Sage-of-Mirrors/libjstudio/blob/main/src/engine/value/interpolation.cpp
float InterpolateHermite(float factor, float timeA, float valueA, float outTangent, float timeB, float valueB, float inTangent)
{
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

uint32_t mProgramID = -1; // on setup handle this

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
        mat4 IndTexMatrices[10];\n\
        uint BillboardType;\n\
        uint ModelId;\n\
        uint MaterialId;\n\
        vec4 HighlightColor;\n\
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
    uniform int selected;\n\
    uniform int pickID;\n\
    layout(location = 0) in vec2 fragTexCoord;\n\
    \
    layout(location = 0) out vec4 outColor;\n\
    layout(location = 1) out int outPick;\n\
    \
    void main()\n\
    {\n\
        vec4 baseColor = texture(texSampler, vec2(fragTexCoord.y, fragTexCoord.x));\n\
        if(selected == 1){\n\
            outColor = baseColor * vec4(1.0, 1.0, 0.2, 1.0);\n\
        } else {\n\
            outColor = baseColor;\n\
        }\n\
        outPick = pickID;\n\
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
        for (std::size_t v = 0; v < count; v++)
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
                LGenUtility::Log << "[Bin Loader]: Error Loading Model! Primitives are wrong? Vertex " << vtx.first << " out of range " << vertices.size() << " or TexCoord " << vtx.second << " out of range " << texcoords.size() << std::endl;
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

            for (std::size_t v = 0; v < count; v++)
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
            for (std::size_t v = 2; v < count; v++)
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

    for (std::size_t i = 0; i < 26; i++)
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
    ////LGenUtility::Log << "Reading Primitives at " << std::hex << stream->tell() << std::endl;
    
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
    ////LGenUtility::Log << "Reading Material at " << std::hex << stream->tell() << std::endl;
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
            ImageFormat::Decode::RGB565(stream, w, h, textureData);
			break;
		case EGXTextureFormat::RGB5A3:
            ImageFormat::Decode::RGB5A3(stream, w, h, textureData);
			break;
		case EGXTextureFormat::CMPR:
			ImageFormat::Decode::CMPR(stream, w, h, textureData);
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

    delete[] textureData;

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

void BinScenegraphNode::Draw(glm::mat4 localTransform, glm::mat4* instance, BinModel* bin, bool ignoreTransforms, bool animate){
    glm::mat4 mtx;
    
    if(!ignoreTransforms){
        // bin->mAnimationInformation.mCurrentFrame;
        // Get Current frame xyz
        if(animate && bin->mAnimationInformation.mLoaded && bin->mAnimationInformation.mPlaying){
            //modify mtx based off of current frame 

	        float sx = MixTrack(&mXScaleTrack, bin->mAnimationInformation.mFrameCount, bin->mAnimationInformation.mCurrentFrame, mNextScaleKeyX);
	        float sy = MixTrack(&mYScaleTrack, bin->mAnimationInformation.mFrameCount, bin->mAnimationInformation.mCurrentFrame, mNextScaleKeyY);
	        float sz = MixTrack(&mZScaleTrack, bin->mAnimationInformation.mFrameCount, bin->mAnimationInformation.mCurrentFrame, mNextScaleKeyZ);

            float rz = MixTrack(&mXRotTrack, bin->mAnimationInformation.mFrameCount, bin->mAnimationInformation.mCurrentFrame, mNextRotKeyX);
            float ry = MixTrack(&mYRotTrack, bin->mAnimationInformation.mFrameCount, bin->mAnimationInformation.mCurrentFrame, mNextRotKeyY);
            float rx = MixTrack(&mZRotTrack, bin->mAnimationInformation.mFrameCount, bin->mAnimationInformation.mCurrentFrame, mNextRotKeyZ);

            float pz = MixTrack(&mXPosTrack, bin->mAnimationInformation.mFrameCount, bin->mAnimationInformation.mCurrentFrame, mNextPosKeyX);
            float py = MixTrack(&mYPosTrack, bin->mAnimationInformation.mFrameCount, bin->mAnimationInformation.mCurrentFrame, mNextPosKeyY);
            float px = MixTrack(&mZPosTrack, bin->mAnimationInformation.mFrameCount, bin->mAnimationInformation.mCurrentFrame, mNextPosKeyZ);

            glm::mat4 animTrasform(1.0f);
            
            //animTrasform = glm::scale(animTrasform, glm::vec3(sx, sy, sz));
            animTrasform = glm::rotate(animTrasform, glm::radians(rx * 0.0001533981f), glm::vec3(1, 0, 0));
            animTrasform = glm::rotate(animTrasform, glm::radians(ry * 0.0001533981f), glm::vec3(0, 1, 0));
            animTrasform = glm::rotate(animTrasform, glm::radians(rz * 0.0001533981f), glm::vec3(0, 0, 1));
            animTrasform = glm::translate(animTrasform, glm::vec3(px, py, pz));

            mtx = *instance * localTransform * (transform * animTrasform);
        } else {
            mtx = *instance * localTransform * transform;
        }
    } else {
        mtx = *instance;
    }

    glUniformMatrix4fv(glGetUniformLocation(mProgramID, "transform"), 1, 0, &(mtx)[0][0]);
    
    for (auto& mesh : meshes)
    {
        bin->BindMesh(mesh.first);
        bin->BindMaterial(mesh.second);

        glUniform4fv(glGetUniformLocation(mProgramID, "binMatColor"), 1, &bin->GetSampler(mesh.second)->mAmbientColor[0]);

        glDrawArrays(GL_TRIANGLES, 0, bin->GetMesh(mesh.first)->mVertexCount);

        glBindVertexArray(0);
    }
    
    if(child != nullptr){
        child->Draw(localTransform * transform, instance, bin, ignoreTransforms, animate);
    }

    if(next != nullptr){
        next->Draw(localTransform, instance, bin, ignoreTransforms, animate);
    }
}

void BinScenegraphNode::ResetAnimation(){
    mNextPosKeyX = 1;
    mNextPosKeyY = 1;
    mNextPosKeyZ = 1;
    mNextRotKeyX = 1;
    mNextRotKeyY = 1;
    mNextRotKeyZ = 1;
    mNextScaleKeyX = 1;
    mNextScaleKeyY = 1;
    mNextScaleKeyZ = 1;

    if(child != nullptr){
        child->ResetAnimation();
    }

    if(next != nullptr){
        next->ResetAnimation();
    }
}

void BinScenegraphNode::LoadNodeTracks(bStream::CStream* stream, uint32_t& idx, uint32_t groupOffset, uint32_t scaleKeysOffset, uint32_t rotateKeysOffset, uint32_t translateKeysOffset){
    stream->seek(groupOffset + (idx * 0x36));

	mXScaleTrack.LoadTrack(stream, scaleKeysOffset, ETrackType::ANM);
	mYScaleTrack.LoadTrack(stream, scaleKeysOffset, ETrackType::ANM);
	mZScaleTrack.LoadTrack(stream, scaleKeysOffset, ETrackType::ANM);

	mXRotTrack.LoadTrack(stream, rotateKeysOffset, ETrackType::ANM);
	mYRotTrack.LoadTrack(stream, rotateKeysOffset, ETrackType::ANM);
	mZRotTrack.LoadTrack(stream, rotateKeysOffset, ETrackType::ANM);

	mXPosTrack.LoadTrack(stream, translateKeysOffset, ETrackType::ANM);
	mYPosTrack.LoadTrack(stream, translateKeysOffset, ETrackType::ANM);
	mZPosTrack.LoadTrack(stream, translateKeysOffset, ETrackType::ANM);

    idx += 1;

    if(child != nullptr){
        child->LoadNodeTracks(stream, idx, groupOffset, scaleKeysOffset, rotateKeysOffset, translateKeysOffset);
    }

    if(next != nullptr){
        next->LoadNodeTracks(stream, idx, groupOffset, scaleKeysOffset, rotateKeysOffset, translateKeysOffset);
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
        //LGenUtility::Log << "Couldn't bind material " << mSamplers[id]->mTextureID << std::endl;
        return false;
    }
}

BinModel::BinModel(bStream::CStream* stream){
    
    uint32_t chunkOffsets[21];
    stream->seek(12);

    for (std::size_t o = 0; o < 21; o++)
    {
        chunkOffsets[o] = stream->readUInt32();
    }


    uint32_t vertexCount = 0;

    for(std::size_t o = 3; o < 21; o++)
    {
        vertexCount = (uint32_t)((chunkOffsets[o] - chunkOffsets[2]) / 6) + 5;
        if(chunkOffsets[o] != 0) break;
    }

    uint32_t texcoordCount = 0; //(uint32_t)((chunkOffsets[10] - chunkOffsets[6]) / 8);
    for(std::size_t o = 7; o < 21; o++)
    {
        texcoordCount = (uint32_t)((chunkOffsets[o] - chunkOffsets[6]) / 8);
        if(chunkOffsets[o] != 0) break;
    }

    uint32_t material_count = (uint32_t)((chunkOffsets[2] - chunkOffsets[1]) / 0x14);
    
    if(chunkOffsets[1] == 0) material_count = 0; //?

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> texcoords;
    
    stream->seek(chunkOffsets[2]);
    for (std::size_t v = 0; v < vertexCount; v++)
    {
        vertices.push_back(glm::vec3(stream->readInt16(), stream->readInt16(), stream->readInt16()));
    }
    
    stream->seek(chunkOffsets[6]);
    for (std::size_t tc = 0; tc < texcoordCount; tc++)
    {
        texcoords.push_back(glm::vec2(stream->readFloat(), stream->readFloat()));
    }

    for (std::size_t m = 0; m < material_count; m++)
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


    stream->readUInt8(); // padding
    stream->readUInt8(); //  renderflags
    stream->readUInt16();

    current->transform = glm::mat4(1.0f);
    glm::vec3 scale = glm::vec3(stream->readFloat(), stream->readFloat(), stream->readFloat());
    glm::vec3 rotation = glm::vec3(stream->readFloat(), stream->readFloat(), stream->readFloat());
    glm::vec3 translation = glm::vec3(stream->readFloat(), stream->readFloat(), stream->readFloat());

    
    current->transform = glm::scale(current->transform, scale); 
    current->transform = glm::rotate(current->transform, glm::radians(rotation.x * 0.0001533981f), glm::vec3(1, 0, 0));
    current->transform = glm::rotate(current->transform, glm::radians(rotation.y * 0.0001533981f), glm::vec3(0, 1, 0));
    current->transform = glm::rotate(current->transform, glm::radians(rotation.z * 0.0001533981f), glm::vec3(0, 0, 1));
    current->transform = glm::translate(current->transform, translation);

    stream->skip(4*7);

    uint16_t meshCount = stream->readUInt16();
    stream->skip(2); // Skip padding
    uint32_t meshOffset = stream->readUInt32();
    stream->seek(offsets[12] + meshOffset);

    for (std::size_t m = 0; m < meshCount; m++)
    {
        int16_t matIndex = stream->readInt16();
        int16_t meshIndex = stream->readInt16();
        
        if(mMeshes.count(meshIndex) == 0){
            std::size_t r = stream->tell();
            stream->seek(offsets[11] + 0x18 * meshIndex);

            mMeshes[meshIndex] = std::make_shared<BinMesh>(stream, offsets[11], vertexData, texcoordData);
            
            stream->seek(r);
        }

        if(mSamplers.count(matIndex) == 0){
            std::size_t r = stream->tell();

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

void BinModel::Draw(glm::mat4* transform, int32_t id, bool selected, bool ignoreTransforms, bool animate){
    glFrontFace(GL_CW);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glUseProgram(mProgramID);
    glUniform1i(glGetUniformLocation(mProgramID, "pickID"), id);
    glUniform1i(glGetUniformLocation(mProgramID, "selected"), selected);
    if(mRoot != nullptr){
        mRoot->Draw(glm::identity<glm::mat4>(), transform, this, ignoreTransforms, animate);
    }

    if(mAnimationInformation.mLoop && mAnimationInformation.mCurrentFrame >= mAnimationInformation.mFrameCount){
        mAnimationInformation.mCurrentFrame = 0.0f;
        mRoot->ResetAnimation();
    }

    if(mAnimationInformation.mLoaded && mAnimationInformation.mPlaying){
        mAnimationInformation.mCurrentFrame += mAnimationInformation.mPlaybackSpeed;
    }
}

void BinModel::LoadAnimation(bStream::CStream* stream){
    mAnimationInformation.mLoaded = true;

    stream->readUInt8(); // version
    mAnimationInformation.mLoop = stream->readUInt8(); // loop

    stream->readUInt16(); // padding
    mAnimationInformation.mFrameCount = stream->readUInt32(); // frame count

    uint32_t scaleKeyOffset = stream->readUInt32();
    uint32_t rotateKeyOffset = stream->readUInt32();
    uint32_t translateKeyOffset = stream->readUInt32();
    uint32_t groupOffset = stream->readUInt32();
    
    uint32_t nodeIdx = 0;

    mRoot->LoadNodeTracks(stream, nodeIdx, groupOffset, scaleKeyOffset, rotateKeyOffset, translateKeyOffset);
}

void BinModel::ClearAnimation(){
    //mRoot->ClearAnimation();
}

BinModel::~BinModel(){

}

void BinModel::DestroyShaders(){
    glDeleteProgram(mProgramID);
}


void BinModel::InitShaders(){
    //Compile Shaders
    char glErrorLogBuffer[4096];
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vs, 1, &default_vtx_shader_source, nullptr);
    glShaderSource(fs, 1, &default_frg_shader_source, nullptr);

    glCompileShader(vs);

    GLint status;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
    if(status == GL_FALSE){
        GLint infoLogLength;
        glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &infoLogLength);

        glGetShaderInfoLog(vs, infoLogLength, nullptr, glErrorLogBuffer);

        printf("[Bin Loader]: Compile failure in vertex shader:\n%s\n", glErrorLogBuffer);
    }

    glCompileShader(fs);

    glGetShaderiv(fs, GL_COMPILE_STATUS, &status);
    if(status == GL_FALSE){
        GLint infoLogLength;
        glGetShaderiv(fs, GL_INFO_LOG_LENGTH, &infoLogLength);

        glGetShaderInfoLog(fs, infoLogLength, nullptr, glErrorLogBuffer);

        printf("[Bin Loader]: Compile failure in fragment shader:\n%s\n", glErrorLogBuffer);
    }

    mProgramID = glCreateProgram();

    glAttachShader(mProgramID, vs);
    glAttachShader(mProgramID, fs);

    glLinkProgram(mProgramID);

    glGetProgramiv(mProgramID, GL_LINK_STATUS, &status); 
    if(GL_FALSE == status) {
        GLint logLen; 
        glGetProgramiv(mProgramID, GL_INFO_LOG_LENGTH, &logLen); 
        glGetProgramInfoLog(mProgramID, logLen, nullptr, glErrorLogBuffer); 
        printf("[Bin Loader]: Shader Program Linking Error:\n%s\n", glErrorLogBuffer);
    } 

    glDetachShader(mProgramID, vs);
    glDetachShader(mProgramID, fs);

    glDeleteShader(vs);
    glDeleteShader(fs);

}