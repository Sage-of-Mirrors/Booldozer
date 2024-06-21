#include "io/CollisionIO.hpp"
#include <Archive.hpp>
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "intersect.h"
// todo: thread this so we can show a loading screen
//std::atomic<bool> mCollisonGenerated;

void LCollisionIO::LoadMp(std::filesystem::path path, std::weak_ptr<LMapDOMNode> map){
    auto mapArc = map.lock()->GetArchive().lock();
    auto colFile = mapArc->GetFile("col.mp");

    bStream::CFileStream importCol(path.string(), bStream::OpenMode::In);

    unsigned char* importColData = new unsigned char[importCol.getSize()];
    importCol.readBytesTo(importColData, importCol.getSize());

    colFile->SetData(importColData, importCol.getSize());

    delete[] importColData;
}

void LCollisionIO::LoadObj(std::filesystem::path path, std::weak_ptr<LMapDOMNode> map){    
    
    glm::vec3 bbmin, bbmax;

    int tangentIdx = 0;
    std::vector<glm::vec3> normals { glm::vec3(0.0f, 0.0f, 0.0f) };
    std::vector<CollisionTriangle> triangles;
    std::vector<GridCell> grid; 

    tinyobj::attrib_t attributes;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string warn;
    std::string err;
    bool ret = tinyobj::LoadObj(&attributes, &shapes, &materials, &warn, &err, path.string().c_str(), nullptr, true);

    if(!ret){
        return;
    }

    tangentIdx = attributes.normals.size();

    for(auto shp : shapes){
        for(int poly = 0; poly < shp.mesh.indices.size() / 3; poly++){
            CollisionTriangle tri;
            
            if(shp.mesh.indices.size() == 0) continue;

            tri.mVtx1 = shp.mesh.indices[(3*poly)+0].vertex_index;
            tri.mVtx2 = shp.mesh.indices[(3*poly)+1].vertex_index;
            tri.mVtx3 = shp.mesh.indices[(3*poly)+2].vertex_index;

            glm::vec3 v1 = glm::vec3(attributes.vertices[tri.mVtx1 * 3], attributes.vertices[(tri.mVtx1 * 3) + 1], attributes.vertices[(tri.mVtx1 * 3) + 2]);
            glm::vec3 v2 = glm::vec3(attributes.vertices[tri.mVtx2 * 3], attributes.vertices[(tri.mVtx2 * 3) + 1], attributes.vertices[(tri.mVtx2 * 3) + 2]);
            glm::vec3 v3 = glm::vec3(attributes.vertices[tri.mVtx3 * 3], attributes.vertices[(tri.mVtx3 * 3) + 1], attributes.vertices[(tri.mVtx3 * 3) + 2]);

            tri.v1 = v1;
            tri.v2 = v2;
            tri.v3 = v3;

            bbmin = glm::vec3(glm::min(bbmin.x, v1.x), glm::min(bbmin.y, v1.y), glm::min(bbmin.z, v1.z));
            bbmin = glm::vec3(glm::min(bbmin.x, v2.x), glm::min(bbmin.y, v2.y), glm::min(bbmin.z, v2.z));
            bbmin = glm::vec3(glm::min(bbmin.x, v3.x), glm::min(bbmin.y, v3.y), glm::min(bbmin.z, v3.z));

            bbmax = glm::vec3(glm::max(bbmax.x, v1.x), glm::max(bbmax.y, v1.y), glm::max(bbmax.z, v1.z));
            bbmax = glm::vec3(glm::max(bbmax.x, v2.x), glm::max(bbmax.y, v2.y), glm::max(bbmax.z, v2.z));
            bbmax = glm::vec3(glm::max(bbmax.x, v3.x), glm::max(bbmax.y, v3.y), glm::max(bbmax.z, v3.z));

            glm::vec3 e10 = v2 - v1; // u
            glm::vec3 e20 = v3 - v1; // v
            glm::vec3 e01 = v1 - v2;
            glm::vec3 e21 = v3 - v2; 

            glm::vec3 normal1 = glm::normalize(glm::cross(e10, e20));
            glm::vec3 normal2 = glm::normalize(glm::cross(e01, e21));
            
            if(std::find(normals.begin(), normals.end(), normal1) == normals.end()) normals.push_back(normal1);
            tri.mNormal = std::find(normals.begin(), normals.end(), normal1) - normals.begin();

            // Tangents
            glm::vec3 tan1 = -glm::normalize(glm::cross(normal1, e10));
            glm::vec3 tan2 = glm::normalize(glm::cross(normal1, e20));
            glm::vec3 tan3 = glm::normalize(glm::cross(normal2, e21));
            
            if(std::find(normals.begin(), normals.end(), tan1) == normals.end()) normals.push_back(tan1);
            tri.mEdgeTan1 = std::find(normals.begin(), normals.end(), tan1) - normals.begin();

            if(std::find(normals.begin(), normals.end(), tan2) == normals.end()) normals.push_back(tan2);
            tri.mEdgeTan2 = std::find(normals.begin(), normals.end(), tan2) - normals.begin();

            if(std::find(normals.begin(), normals.end(), tan3) == normals.end()) normals.push_back(tan3);
            tri.mEdgeTan3 = std::find(normals.begin(), normals.end(), tan3) - normals.begin();
            
            glm::vec3 up(0.0f, 1.0f, 0.0f);
            float numerator = glm::dot(normal1, up);
            float angle = (float)glm::acos(numerator);

            angle *= (float)(180.0f / glm::pi<float>());

            if(glm::abs(angle) >= 0.0f && glm::abs(angle) <= 0.5f){
                tri.mUnkIdx = tri.mEdgeTan1;
            } else {
                tri.mUnkIdx = 0;
            }

            if(glm::abs(angle) <= 65.0f){
                tri.mFloor = true;
            } else {
                tri.mFloor = false;
            }
            
            tri.mDot = glm::dot(tan3, e10);

            tri.mMask = 0x8000;
            tri.mFriction = 0;

            triangles.push_back(tri);
        }
    }

    std::cout << "[CollisionIO:ObjImport]: Bounding Box ["
        << bbmin.x << ", " << bbmin.y << ", " << bbmin.z << "] ["
        << bbmax.x << ", " << bbmax.y << ", " << bbmax.z << "]" << std::endl; 

    glm::vec3 axisLengths((bbmax.x - bbmin.x), (bbmax.y - bbmin.y), (bbmax.z - bbmin.z));

    std::vector<uint32_t> gridData = {};
    std::vector<uint16_t> groupData = { 0xFFFF };

    // Now that we have all the triangles, generate the grid and the groups.
	int xCellCount = (int)(glm::ceil(axisLengths.x / 256.0f) + 1);
	int yCellCount = (int)(glm::ceil(axisLengths.y / 512.0f) + 1);
	int zCellCount = (int)(glm::ceil(axisLengths.z / 256.0f) + 1);

    float xCellSize = axisLengths.x / xCellCount;
    float yCellSize = axisLengths.y / yCellCount;
    float zCellSize = axisLengths.z / zCellCount;

    float xHalfSize = xCellSize / 2;
    float yHalfSize = yCellSize / 2;
    float zHalfSize = zCellSize / 2;

    float curX = bbmin.x;
    float curY = bbmin.y;
    float curZ = bbmin.z;

    for (int z = 0; z < zCellCount; z++){
        for (int y = 0; y < yCellCount; y++){
            for (int x = 0; x < xCellCount; x++){
                GridCell cell;

                float halfSize[3] = {xHalfSize, yHalfSize, zHalfSize};
                float boxCenter[3] = {curX + xHalfSize, curY + yHalfSize, curZ + zHalfSize};
                for(std::vector<CollisionTriangle>::iterator tri = triangles.begin(); tri != triangles.end(); tri++){
                    float verts[3][3] = {
                        {tri->v1.x, tri->v1.y, tri->v1.z},
                        {tri->v2.x, tri->v2.y, tri->v2.z},
                        {tri->v3.x, tri->v3.y, tri->v3.z}
                    };

                    if(triBoxOverlap(boxCenter, halfSize, verts) != 0){
                        // add to cell all group!
                        cell.mAll.push_back(tri - triangles.begin());
                        // add to cell floor group!
                        if(tri->mFloor) cell.mFloor.push_back(tri - triangles.begin());
                    }

                }
                grid.push_back(cell);
            }
            curX = bbmin.x;
            curY += yCellSize;
        }
        curY = bbmin.y;
        curZ += zCellSize;
    }
    
    // gen grid
    for (int z = 0; z < zCellCount; z++){
        for (int y = 0; y < yCellCount; y++){
            for (int x = 0; x < xCellCount; x++){
                int idx = x + (y * xCellCount) + (z * xCellCount * yCellCount);

                if(grid[idx].mAll.size() > 0){
                    gridData.push_back(groupData.size());
                    for(auto triIdx : grid[idx].mAll){
                        groupData.push_back(triIdx);
                    }
                    groupData.push_back(0xFFFF);
                } else if(y > 0){
                    idx = x + ((y - 1) * xCellCount) + (z * xCellCount * yCellCount);
                    if(grid[idx].mAll.size() > 0){
                        gridData.push_back(groupData.size());
                        for(auto triIdx : grid[idx].mAll){
                            groupData.push_back(triIdx);
                        }
                        groupData.push_back(0xFFFF);
                    } else {
                        gridData.push_back(0);
                    }
                } else {
                    gridData.push_back(0);
                }

                if(grid[idx].mFloor.size() > 0){
                    gridData.push_back(groupData.size());
                    for(auto triIdx : grid[idx].mFloor){
                        groupData.push_back(triIdx);
                    }
                    groupData.push_back(0xFFFF);
                } else {
                    gridData.push_back(0);
                }

            }
        }
    }

    groupData.push_back(0xFFFF);

    // Write structures to file
    
    bStream::CMemoryStream stream(1024, bStream::Endianess::Big, bStream::OpenMode::Out);

    // Write scale
    stream.writeFloat(256.0f);
    stream.writeFloat(512.0f);
    stream.writeFloat(256.0f);

    // Write Min + Axis Len
    stream.writeFloat(bbmin.x);
    stream.writeFloat(bbmin.y);
    stream.writeFloat(bbmin.z);

    stream.writeFloat(axisLengths.x);
    stream.writeFloat(axisLengths.y);
    stream.writeFloat(axisLengths.z);

    stream.writeUInt32(0x40);
    stream.writeUInt32(0);
    stream.writeUInt32(0);
    stream.writeUInt32(0);
    stream.writeUInt32(0);
    stream.writeUInt32(0);
    stream.writeUInt32(0);

    for(int v = 0; v < attributes.vertices.size(); v++){
        stream.writeFloat(attributes.vertices[v]);
    }

    uint32_t normalsOffset = stream.tell();

    for(std::vector<glm::vec3>::iterator n = normals.begin(); n != normals.end(); n++){
        stream.writeFloat(n->x);
        stream.writeFloat(n->y);
        stream.writeFloat(n->z);
    }

    uint32_t trianglesOffset = stream.tell();

    for(std::vector<CollisionTriangle>::iterator t = triangles.begin(); t != triangles.end(); t++){
        stream.writeUInt16(t->mVtx1);
        stream.writeUInt16(t->mVtx2);
        stream.writeUInt16(t->mVtx3);

        stream.writeUInt16(t->mNormal);

        stream.writeUInt16(t->mEdgeTan1);
        stream.writeUInt16(t->mEdgeTan2);
        stream.writeUInt16(t->mEdgeTan3);

        stream.writeUInt16(t->mUnkIdx);

        stream.writeFloat(t->mDot);

        stream.writeUInt16(t->mMask);
        stream.writeUInt16(t->mFriction);
    }

    uint32_t groupOffset = stream.tell();

    for(std::vector<uint16_t>::iterator t = groupData.begin(); t != groupData.end(); t++){
        stream.writeUInt16(*t);
    }

    uint32_t gridOffset = stream.tell();
    
    for(std::vector<uint32_t>::iterator t = gridData.begin(); t != gridData.end(); t++){
        stream.writeUInt32(*t);
    }

    uint32_t unkData = stream.tell();

    long aligned = (stream.tell() + 31) & ~31;
    long delta = aligned - stream.tell();

    for(int x = 0; x < delta; x++){
        stream.writeUInt8(0x40);
    }

    stream.seek(0x28);
    stream.writeUInt32(normalsOffset);
    stream.writeUInt32(trianglesOffset);
    stream.writeUInt32(groupOffset);
    stream.writeUInt32(gridOffset);
    stream.writeUInt32(gridOffset);
    stream.writeUInt32(unkData);
    stream.seek(0);

    auto mapArc = map.lock()->GetArchive().lock();
    auto colFile = mapArc->GetFile("col.mp");

    colFile->SetData(stream.getBuffer(), stream.getSize());

}