#include "io/CollisionIO.hpp"
#include <Archive.hpp>
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <fmt/format.h>

#include <GenUtil.hpp>
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

void LCollisionIO::LoadObj(std::filesystem::path path, std::weak_ptr<LMapDOMNode> map, std::map<std::string, std::string> propertyMap){    
    
    glm::vec3 bbmin, bbmax;

    std::vector<glm::vec3> normals {glm::vec3(0.0f)};
    std::vector<CollisionTriangle> triangles;
    std::vector<GridCell> grid; 

    tinyobj::attrib_t attributes;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string warn;
    std::string err;
    bool ret = tinyobj::LoadObj(&attributes, &shapes, &materials, &warn, &err, path.string().c_str(), path.parent_path().string().c_str(), true);

    if(!ret){
        return;
    }

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
            
            if(!LGenUtility::VectorContains<glm::vec3>(normals, normal1)) normals.push_back(normal1);
            tri.mNormal = LGenUtility::VectorIndexOf<glm::vec3>(normals, normal1);

            // Tangents
            glm::vec3 tan1 = -glm::normalize(glm::cross(normal1, e10));
            glm::vec3 tan2 = glm::normalize(glm::cross(normal1, e20));
            glm::vec3 tan3 = glm::normalize(glm::cross(normal2, e21));
            
            if(!LGenUtility::VectorContains<glm::vec3>(normals, tan1)) normals.push_back(tan1);
            tri.mEdgeTan1 = LGenUtility::VectorIndexOf<glm::vec3>(normals, tan1);

            if(!LGenUtility::VectorContains<glm::vec3>(normals, tan2)) normals.push_back(tan2);
            tri.mEdgeTan2 = LGenUtility::VectorIndexOf<glm::vec3>(normals, tan2);

            if(!LGenUtility::VectorContains<glm::vec3>(normals, tan3)) normals.push_back(tan3);
            tri.mEdgeTan3 = LGenUtility::VectorIndexOf<glm::vec3>(normals, tan3);
            
            glm::vec3 up(0.0f, 1.0f, 0.0f);
            float angle = (float)glm::acos(glm::dot(normal1, up));

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
            tri.mTriIdx = triangles.size();

            tri.mSound = 0;
            tri.mSoundEchoSwitch = 0;
            tri.mLadder = 0;
            tri.mIgnorePointer = 0;
            tri.mSurfMaterial = 0;

            if(materials.size() > 0 && shp.mesh.material_ids.size() > 0 && shp.mesh.material_ids[0] < materials.size()){
                tinyobj::material_t polyMat = materials[shp.mesh.material_ids[0]];

                for(auto [prop, val] : polyMat.unknown_parameter){
                    try {
                        if(prop == propertyMap["Group"]){
                            if(val.substr(0,2) == "0x"){
                                tri.mMask = std::stoi(val.substr(2), nullptr, 16);
                            } else {
                                tri.mMask = std::stoi(val, nullptr, 16);
                            }
                        } else if(prop == propertyMap["Sound"]){
                            tri.mSound = std::stoi(val);
                        } else if(prop == propertyMap["SoundEchoSwitch"]){
                            tri.mSoundEchoSwitch = std::stoi(val);
                        } else if(prop == propertyMap["Friction"]){
                            tri.mFriction = std::stoi(val);
                        } else if(prop == propertyMap["Ladder"]){
                            tri.mLadder = std::stoi(val);
                        } else if(prop == propertyMap["IgnorePointer"]){
                            tri.mIgnorePointer = std::stoi(val);
                        } else if(prop == propertyMap["SurfaceMaterial"]){
                            tri.mSurfMaterial = std::stoi(val);
                        } 
                    } catch(std::invalid_argument e){
                        continue;
                    }
                }
            }
            

            triangles.push_back(tri);
        }
    }

    std::cout << "[CollisionIO:ObjImport]: Bounding Box ["
        << bbmin.x << ", " << bbmin.y << ", " << bbmin.z << "] ["
        << bbmax.x << ", " << bbmax.y << ", " << bbmax.z << "]" << std::endl; 


    glm::vec3 axisLengths((bbmax.x - bbmin.x), (bbmax.y - bbmin.y), (bbmax.z - bbmin.z));

    std::vector<uint32_t> gridData = {};
    std::vector<int16_t> groupData = {};

    // Now that we have all the triangles, generate the grid and the groups.
	int xCellCount = (int)(glm::ceil(axisLengths.x / 256.0f));
	int yCellCount = (int)(glm::ceil(axisLengths.y / 512.0f));
	int zCellCount = (int)(glm::ceil(axisLengths.z / 256.0f));

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

                glm::vec3 boxExtents = {xCellSize, yCellSize, zCellSize};
                glm::vec3 boxCenter = {curX + (xCellSize / 2), curY + (yCellSize / 2), curZ + (zCellSize / 2)};
                for(std::vector<CollisionTriangle>::iterator tri = triangles.begin(); tri != triangles.end(); tri++){
                    glm::vec3 triVerts[3] = {tri->v1, tri->v2, tri->v3};

                    if(LGenUtility::TriBoxIntersect(triVerts, boxCenter, boxExtents)){
                         // add to cell all group!
                        cell.mAll.push_back(tri->mTriIdx);
                        // add to cell floor group!
                        if(tri->mFloor) cell.mFloor.push_back(tri->mTriIdx);
                    }

                }
                grid.push_back(cell);
                curX += xCellSize;
            }
            curX = bbmin.x;
            curY += yCellSize;
        }
        curY = bbmin.y;
        curZ += zCellSize;
    }

    groupData.push_back(0xFFFF);
    
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
                    int prevIdx = x + ((y - 1) * xCellCount) + (z * xCellCount * yCellCount);
                    if(grid[prevIdx].mAll.size() > 0){
                        gridData.push_back(groupData.size());
                        for(auto triIdx : grid[prevIdx].mAll){
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
    auto mapArc = map.lock()->GetArchive().lock();
    
    {
        bStream::CMemoryStream stream(1024, bStream::Endianess::Big, bStream::OpenMode::Out);

        // Write col.mp Header

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

        for(std::vector<int16_t>::iterator t = groupData.begin(); t != groupData.end(); t++){
            stream.writeInt16(*t);
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

        long end = stream.tell();
        stream.setSize(end);

        stream.seek(0x28);
        stream.writeUInt32(normalsOffset);
        stream.writeUInt32(trianglesOffset);
        stream.writeUInt32(groupOffset);
        stream.writeUInt32(gridOffset);
        stream.writeUInt32(gridOffset);
        stream.writeUInt32(unkData);
        stream.seek(0);
        auto colFile = mapArc->GetFile("col.mp");

        colFile->SetData(stream.getBuffer(), stream.getSize());
    }


    {
        bStream::CMemoryStream polygoninfo(100, bStream::Endianess::Big, bStream::OpenMode::Out);
        
        // Write polygoninfo jmp header
        polygoninfo.writeUInt32(triangles.size());
        polygoninfo.writeUInt32(3);
        polygoninfo.writeUInt32(0x34);
        polygoninfo.writeUInt32(4);

        polygoninfo.writeUInt32(0x002AAF7F);
        polygoninfo.writeUInt32(3);
        polygoninfo.writeUInt32(0);

        polygoninfo.writeUInt32(0x01C2B94A);
        polygoninfo.writeUInt32(4);
        polygoninfo.writeUInt32(0x200);

        polygoninfo.writeUInt32(0x00AF2BA5);
        polygoninfo.writeUInt32(8);
        polygoninfo.writeUInt32(0x300);

        for(std::vector<CollisionTriangle>::iterator t = triangles.begin(); t != triangles.end(); t++){
            uint32_t polyProps = 0x00;
            polyProps |= t->mIgnorePointer << 3;
            polyProps |= t->mLadder << 2;
            polyProps |= t->mSurfMaterial;
            polygoninfo.writeUInt32(polyProps);
        }

        // pad and set polygoninfo data
        long aligned = (polygoninfo.tell() + 31) & ~31;
        long delta = aligned - polygoninfo.tell();
        for(int x = 0; x < delta; x++) polygoninfo.writeUInt8(0x40);

        polygoninfo.setSize(polygoninfo.tell());
        std::cout << "[CollisionIO:ObjImport]: polygoninfo size is " << polygoninfo.getSize() << std::endl;
        auto polygoninfoFile = mapArc->GetFolder("jmp")->GetFile("polygoninfo");
        if(polygoninfoFile == nullptr){
            polygoninfoFile = Archive::File::Create();
            polygoninfoFile->SetName("polygoninfo");
            mapArc->GetFolder("jmp")->AddFile(polygoninfoFile);
        }
        polygoninfoFile->SetData(polygoninfo.getBuffer(), polygoninfo.getSize());
    }

    {

        bStream::CMemoryStream sndpolygoninfo(100, bStream::Endianess::Big, bStream::OpenMode::Out);

        // Write sndpolygoninfo jmp header
        sndpolygoninfo.writeUInt32(triangles.size());
        sndpolygoninfo.writeUInt32(2);
        sndpolygoninfo.writeUInt32(0x28);
        sndpolygoninfo.writeUInt32(4);

        sndpolygoninfo.writeUInt32(0x006064D7);
        sndpolygoninfo.writeUInt32(0xF);
        sndpolygoninfo.writeUInt32(0);

        sndpolygoninfo.writeUInt32(0x005169FA);
        sndpolygoninfo.writeUInt32(0x70);
        sndpolygoninfo.writeUInt32(0x400);

        for(std::vector<CollisionTriangle>::iterator t = triangles.begin(); t != triangles.end(); t++){
        uint32_t soundProps = 0x00;
            soundProps |= t->mSoundEchoSwitch << 4;
            soundProps |= t->mSound;
            sndpolygoninfo.writeUInt32(soundProps);
        }

        // pad and set soundpolygoninfo data
        long aligned = (sndpolygoninfo.tell() + 31) & ~31;
        long delta = aligned - sndpolygoninfo.tell();
        for(int x = 0; x < delta; x++) sndpolygoninfo.writeUInt8(0x40);

        sndpolygoninfo.setSize(sndpolygoninfo.tell());
        std::cout << "[CollisionIO:ObjImport]: soundpolygoninfo size is " << sndpolygoninfo.getSize() << std::endl;
        auto sndpolygoninfoFile = mapArc->GetFolder("jmp")->GetFile("soundpolygoninfo");
        if(sndpolygoninfoFile == nullptr){
            sndpolygoninfoFile = Archive::File::Create();
            sndpolygoninfoFile->SetName("soundpolygoninfo");
            mapArc->GetFolder("jmp")->AddFile(sndpolygoninfoFile);
        }
        sndpolygoninfoFile->SetData(sndpolygoninfo.getBuffer(), sndpolygoninfo.getSize());
    }
}