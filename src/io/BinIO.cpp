#include <vector>
#include "glm/glm.hpp"
#include "glm/gtx/quaternion.hpp"
#include "decode.h"
#include "io/BinIO.hpp"
#define STB_IMAGE_WRITE_IMPLEMENTATION


GXShape* ReadBinShape(bStream::CStream* stream, uint32_t offset){
    GXShape* shape = new GXShape();

    stream->skip(2);
    uint16_t listSize = stream->readUInt16() << 5;
    uint32_t attr = stream->readUInt32();

    std::vector<EGXAttribute> attributes;
    uint32_t mask = 1;

    for (size_t i = 0; i < 26; i++)
    {
        if((attr & mask) >> i)
        {
            attributes.push_back((EGXAttribute)i);
        }

        mask <<= 1;
    }
    
    stream->skip(3);
    
    uint8_t useNbt = stream->readUInt8();
    uint32_t primitiveOffset = stream->readUInt32();
    uint32_t ret = stream->tell();

    auto& shapePrimitives = shape->GetPrimitives();

    stream->seek(offset + primitiveOffset);
    
    // read primitives
    EGXPrimitiveType primType;

    while ((primType = (EGXPrimitiveType)stream->readUInt8()) != EGXPrimitiveType::None){

        GXPrimitive* primitive = new GXPrimitive(primType);
        auto& primVertices = primitive->GetVertices();

        uint16_t vtxCount = stream->readUInt16();
        for (uint16_t i = 0; i < vtxCount; i++){
            GXVertex vtx;
            for(auto attribute : attributes){
                vtx.SetIndex(attribute, stream->readUInt16());
            }
            primVertices.push_back(vtx);
        }
        shapePrimitives.push_back(primitive);
    }

    stream->seek(ret);
    return shape;
}

///
/// Materials and Textures
///

void BinMaterial::bind(){
}

BinMaterial::BinMaterial(bStream::CStream* stream, uint32_t textureOffset){
    ////std::cout << "Reading Material at " << std::hex << stream->tell() << std::endl;
    int16_t textureID = stream->readInt16();
    if(textureID == -1) return;

    stream->skip(2);
    mWrap[0] = stream->readUInt8();
    mWrap[1] = stream->readUInt8();

    stream->seek(textureOffset + (textureID * 0xC));
    uint16_t w = stream->readUInt16();
    uint16_t h = stream->readUInt16();

    uint8_t format = stream->readUInt8();
    stream->skip(3);
    uint32_t dataOffset = stream->readUInt32() + textureOffset;
    stream->seek(textureOffset + (textureID+1 * 0xC) + 8);
    uint32_t nextOffset = stream->readUInt32();
    stream->seek(dataOffset);


    mTextureData = new uint8_t[w*h*4];
    uint8_t* src = new uint8_t[nextOffset - dataOffset];

    //todo: more of these

    switch (format)
    {
    case 0x0E:
        //btiDecodeCMPR(mTextureData, src, w, h);
        break;
    }
    delete src;

}

BinSampler::BinSampler(bStream::CStream* stream){
    stream->skip(3);
    mAmbientColor = stream->readUInt32();
    stream->skip(1);
    mTextureID = stream->readUInt16();
}

///
/// Scenegraph Node
///

BinScenegraphNode::BinScenegraphNode(){}
BinScenegraphNode::~BinScenegraphNode(){}


void BinScenegraphNode::AddMesh(int16_t material, int16_t mesh){
    meshes.push_back(std::pair<uint16_t, uint16_t>(material, mesh));
}

void BinScenegraphNode::Draw(glm::mat4 localTransform, glm::mat4* instance, BinModel* bin, bool bIgnoreTransforms){

    for (auto& mesh : meshes)
    {
        //draw shape with material
    }
    
    if(child != nullptr){
        child->Draw(localTransform * transform, instance, bin, bIgnoreTransforms);
    }

    if(next != nullptr){
        next->Draw(localTransform, instance, bin, bIgnoreTransforms);
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
        vertexCount = (uint32_t)((chunkOffsets[o] - chunkOffsets[2]) / 6);
        if(chunkOffsets[o] != 0) break;
    }

    uint32_t texcoordCount = (uint32_t)((chunkOffsets[10] - chunkOffsets[6]) / 8);
    uint32_t material_count = (uint32_t)((chunkOffsets[2] - chunkOffsets[1]) / 0x14);
    
    if(chunkOffsets[1] == 0) material_count = 0;

    auto& positions = mVertexData.GetPositions();
    auto& texcoords = mVertexData.GetTexCoords(0);
    
    stream->seek(chunkOffsets[2]);
    for (size_t v = 0; v < vertexCount; v++)
    {
        positions.push_back(glm::vec4(stream->readInt16(), stream->readInt16(), stream->readInt16(), 0));
    }
    
    stream->seek(chunkOffsets[6]);
    for (size_t tc = 0; tc < texcoordCount; tc++)
    {
        texcoords.push_back(glm::vec3(stream->readFloat(), stream->readFloat(), 0.0f));
    }

    for (size_t m = 0; m < material_count; m++)
    {
        stream->seek(chunkOffsets[1] + (0x14 * m));
        mMaterials.push_back(BinMaterial(stream, chunkOffsets[0]));
    }
    
    mRoot = ParseSceneraph(stream, chunkOffsets, 0, nullptr);
    ////std::cout << mRoot.get();
    ////std::cout << '\n';
}

std::shared_ptr<BinScenegraphNode> BinModel::ParseSceneraph(bStream::CStream* stream, uint32_t* offsets, uint16_t index, std::shared_ptr<BinScenegraphNode> parent, std::shared_ptr<BinScenegraphNode> previous){
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
    current->transform = glm::rotate(current->transform, glm::radians(stream->readFloat()), glm::vec3(1,0,0));
    current->transform = glm::rotate(current->transform, glm::radians(stream->readFloat()), glm::vec3(0,1,0));
    current->transform = glm::rotate(current->transform, glm::radians(stream->readFloat()), glm::vec3(0,0,1));
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

            mMeshes[meshIndex] = ReadBinShape(stream, offsets[11]);

            stream->seek(offsets[10] + 0x28 * matIndex);

            mSamplers[matIndex] = BinSampler(stream);
            
            stream->seek(r);
        }

        current->AddMesh(meshIndex, matIndex);
    }

    if(childIndex != -1){
        current->child = ParseSceneraph(stream, offsets, childIndex, current);
    }
    
    if(nextIndex != -1){
        current->next = ParseSceneraph(stream, offsets, nextIndex, nullptr, current);
    }

    return current;
    
}

void BinModel::TranslateRoot(glm::vec3 translation){
    mRoot->transform = glm::translate(mRoot->transform, translation);
}

void BinModel::Draw(glm::mat4* transform, bool bIgnoreTransforms){
    mRoot->Draw(glm::identity<glm::mat4>(), transform, this, bIgnoreTransforms);
}

BinModel::~BinModel(){
    //TODO: Delete shapes!
}