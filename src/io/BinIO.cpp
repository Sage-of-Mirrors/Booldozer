#include <vector>
#include "glm/glm.hpp"
#include "glm/gtx/quaternion.hpp"
#include "io/BinIO.hpp"
#include "../lib/bigg/deps/bgfx.cmake/bimg/3rdparty/libsquish/colourblock.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
/*
*
* The bin loader needs *serious* cleanup. As it is now, it exists just to make bin loading possible.
*
*/

//This whole thing needs to be redone in a better way.


struct BinVertex
{
	float x;
	float y;
	float z;
	float u;
	float v;

	static void init()
	{
		attributes
			.begin()
			.add(bgfx::Attrib::Position,  3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
			.end();
	}
	static bgfx::VertexLayout attributes;
};

bgfx::VertexLayout BinVertex::attributes;

std::vector<BinVertex>ReadGXPrimitives(bStream::CStream* stream, std::vector<glm::vec3>& vertices, std::vector<glm::vec2>& texcoords, std::vector<GXAttribute>& attributes){
    std::vector<BinVertex> vd_out;

    uint8_t primitiveType = stream->readUInt8(); 
    while (primitiveType == Triangles || primitiveType == TriangleStrip || primitiveType == TriangleFan || primitiveType == Quads)
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
                default:
                    stream->readUInt16(); //TODO: NBT Fix
                    break;
                }
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
                texcoord = vertices.at(primVertices.at(v).second);
                vtx.x = pos.x;
                vtx.y = pos.y;
                vtx.z = pos.z;
                vtx.u = texcoord.x;
                vtx.v = texcoord.y;
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
                vtx.u = texcoord.y;
                vtx.v = texcoord.x;
                vd_out.push_back(vtx);

                pos = vertices.at((v % 2 != 0 ? primVertices.at(v) : primVertices.at(v - 1)).first);
                texcoord = texcoords.at((v % 2 != 0 ? primVertices.at(v) : primVertices.at(v - 1)).second);
                vtx.x = pos.x;
                vtx.y = pos.y;
                vtx.z = pos.z;
                vtx.u = texcoord.y;
                vtx.v = texcoord.x;
                vd_out.push_back(vtx);

                pos = vertices.at((v % 2 != 0 ? primVertices.at(v - 1) : primVertices.at(v)).first);
                texcoord = texcoords.at((v % 2 != 0 ? primVertices.at(v - 1) : primVertices.at(v)).second);
                vtx.x = pos.x;
                vtx.y = pos.y;
                vtx.z = pos.z;
                vtx.u = texcoord.y;
                vtx.v = texcoord.x;
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
    uint16_t listSize = stream->readUInt16() << 5;
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
    std::cout << "Reading Primitives at " << std::hex << stream->tell() << std::endl;
    
    std::vector<BinVertex> buffer = ReadGXPrimitives(stream, vertexData, texcoordData, attributes);
    vbo = bgfx::createVertexBuffer(bgfx::copy(buffer.data(), buffer.size() * sizeof(BinVertex)), BinVertex::attributes);


    stream->seek(ret);
}

void BinMesh::bind(){
    bgfx::setVertexBuffer(0, vbo);
	//bgfx::setIndexBuffer(ebo);
}

BinMesh::~BinMesh(){
}

///
/// Materials and Textures
///

void BinMaterial::LoadCMPRTex(bStream::CStream* stream, uint16_t w, uint16_t h, uint32_t* out){
    uint32_t rgba[64];
    for (size_t ty = 0; ty < h; ty += 8){
        for (size_t tx = 0; tx < w; tx += 8){
            for (size_t by = 0; by < 8; by += 4){
                for (size_t bx = 0; bx < 8; bx += 4){
                    uint8_t block[8];
                    stream->readBytesTo(block, 8);
                    squish::Decompress((uint8_t*)rgba, block, 0);
                    
                    for(size_t y = 0; y < 4; y++){
                        for(size_t x = 0; x < 4; x++){
	                    	out[(ty + by + y) * w + (tx + bx + (3-x))] = rgba[(y * 4 + x)];
                        }       
                    }
                }
            }
        }        
    }

}

void BinMaterial::bind(bgfx::UniformHandle& texUniform){
    bgfx::setTexture(0, texUniform, mTexture);
}

BinMaterial::BinMaterial(bStream::CStream* stream, uint32_t textureOffset){
    std::cout << "Reading Material at " << std::hex << stream->tell() << std::endl;
    int16_t textureID = stream->readInt16();
    stream->skip(2);
    mWrapU = stream->readUInt8();
    mWrapV = stream->readUInt8();

    stream->seek(textureOffset + (textureID * 0xC));
    uint16_t w = stream->readUInt16();
    uint16_t h = stream->readUInt16();

    uint8_t format = stream->readUInt8();
    stream->skip(3);
    uint32_t dataOffset = stream->readUInt32() + textureOffset;
    stream->seek(dataOffset);
    std::cout << "Reading Texture Data at " << stream->tell() << std::endl;

    const bgfx::Memory* textureData = bgfx::alloc(w*h*4);

    switch (format)
    {
    case 0x0E:
        LoadCMPRTex(stream, w, h, (uint32_t*)textureData->data);
        break;
    }

    mTexture = bgfx::createTexture2D(w, h, false, 1, bgfx::TextureFormat::RGBA8, 0, textureData);

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
    meshes.push_back(std::pair(material, mesh));
}

void BinScenegraphNode::Draw(glm::mat4 localTransform, std::vector<std::shared_ptr<glm::mat4>>& instances, BGFXBin* bin, bgfx::ProgramHandle& program, bgfx::UniformHandle& texUniform){
    for(auto& instance : instances)
    {
        for (auto& mesh : meshes)
        {
            bin->BindMesh(mesh.first);
            bin->BindMaterial(mesh.second, texUniform);

            bgfx::setTransform(&(*instance.get() * localTransform * transform)[0][0]);
            bgfx::submit(0, program);
        }
    }
    
    if(child != nullptr){
        child->Draw(localTransform * transform, instances, bin, program, texUniform);
    }

    if(next != nullptr){
        next->Draw(localTransform, instances, bin, program, texUniform);
    }
}

void BGFXBin::BindMesh(uint16_t id){
    mMeshes[id].bind();
}

void BGFXBin::BindMaterial(uint16_t id, bgfx::UniformHandle& texUniform){
    mMaterials[mSamplers[id].mTextureID].bind(texUniform);
}

BGFXBin::BGFXBin(bStream::CStream* stream){
    
    uint32_t chunkOffsets[21];
    stream->seek(12);

    for (size_t o = 0; o < 21; o++)
    {
        chunkOffsets[o] = stream->readUInt32();
    }

    uint32_t vertexCount = (uint32_t)((chunkOffsets[3] - chunkOffsets[2]) / 6);
    uint32_t texcoordCount = (uint32_t)((chunkOffsets[10] - chunkOffsets[6]) / 8);
    uint32_t material_count = (uint32_t)((chunkOffsets[2] - chunkOffsets[1]) / 0x14);
    
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
        mMaterials.push_back(BinMaterial(stream, chunkOffsets[0]));
    }
    
    mRoot = ParseSceneraph(stream, chunkOffsets, 0, vertices, texcoords);
    std::cout << mRoot.get();
    std::cout << '\n';
}

std::shared_ptr<BinScenegraphNode> BGFXBin::ParseSceneraph(bStream::CStream* stream, uint32_t* offsets, uint16_t index, std::vector<glm::vec3>& vertexData, std::vector<glm::vec2>& texcoordData, std::shared_ptr<BinScenegraphNode> parent, std::shared_ptr<BinScenegraphNode> previous){
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
    glm::quat r = glm::quat(glm::vec3(stream->readFloat(), stream->readFloat(), stream->readFloat()));
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

            mMeshes[meshIndex] = BinMesh(stream, offsets[11], vertexData, texcoordData);

            stream->seek(offsets[10] + 0x28 * matIndex);

            mSamplers[matIndex] = BinSampler(stream);
            
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

void BGFXBin::Draw(std::vector<std::shared_ptr<glm::mat4>>& transforms, bgfx::ProgramHandle& program, bgfx::UniformHandle& texUniform){
    mRoot->Draw(glm::identity<glm::mat4>(), transforms, this, program, texUniform);
}

void BGFXBin::InitBinVertex(){
    BinVertex::init();
}

BGFXBin::~BGFXBin(){}