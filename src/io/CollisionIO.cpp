#include "io/CollisionIO.hpp"
#include <glm/gtx/matrix_decompose.hpp>
#include <Archive.hpp>
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <format>

#include "DOM/MapDOMNode.hpp"
#include "DOM/RoomDOMNode.hpp"
#include "DOM/FurnitureDOMNode.hpp"
#include "DOM/MapCollisionDOMNode.hpp"

#include <GenUtil.hpp>

ColModel CUBE {
    .mVertices = {{ -1.0, 1.0, -1.0 }, { 1.0, 1.0, 1.0 }, { 1.0, 1.0, -1.0 }, { -1.0, -1.0, 1.0 }, { 1.0, -1.0, 1.0 }, { -1.0, 1.0, 1.0 }, { -1.0, -1.0, -1.0 }, { 1.0, -1.0, -1.0 }},
    .mTriangles = {{ .mVtx1 = 0, .mVtx2 = 1, .mVtx3 = 2, .mNormal = 1, .mEdgeTan1 = 2, .mEdgeTan2 = 3, .mEdgeTan3 = 4, .mUnkIdx = 2, .mDot = 2.0, .mMask = 0x8000, .mFriction = 0, .mSound = 0, .mSoundEchoSwitch = 0, .mLadder = 0, .mIgnorePointer = 0, .mSurfMaterial = 0, .mFloor = true, .v1 = { -1.0, 1.0, -1.0 }, .v2 = { 1.0, 1.0, 1.0 }, .v3 = { 1.0, 1.0, -1.0 }, .mTriIdx = 0},{ .mVtx1 = 1, .mVtx2 = 3, .mVtx3 = 4, .mNormal = 5, .mEdgeTan1 = 6, .mEdgeTan2 = 4, .mEdgeTan3 = 7, .mUnkIdx = 0, .mDot = 2.0, .mMask = 0x8000, .mFriction = 0, .mSound = 0, .mSoundEchoSwitch = 0, .mLadder = 0, .mIgnorePointer = 0, .mSurfMaterial = 0, .mFloor = false, .v1 = { 1.0, 1.0, 1.0 }, .v2 = { -1.0, -1.0, 1.0 }, .v3 = { 1.0, -1.0, 1.0 }, .mTriIdx = 1},{ .mVtx1 = 5, .mVtx2 = 6, .mVtx3 = 3, .mNormal = 8, .mEdgeTan1 = 9, .mEdgeTan2 = 5, .mEdgeTan3 = 7, .mUnkIdx = 0, .mDot = 2.0, .mMask = 0x8000, .mFriction = 0, .mSound = 0, .mSoundEchoSwitch = 0, .mLadder = 0, .mIgnorePointer = 0, .mSurfMaterial = 0, .mFloor = false, .v1 = { -1.0, 1.0, 1.0 }, .v2 = { -1.0, -1.0, -1.0 }, .v3 = { -1.0, -1.0, 1.0 }, .mTriIdx = 2},{ .mVtx1 = 7, .mVtx2 = 3, .mVtx3 = 6, .mNormal = 7, .mEdgeTan1 = 10, .mEdgeTan2 = 3, .mEdgeTan3 = 8, .mUnkIdx = 0, .mDot = 2.0, .mMask = 0x8000, .mFriction = 0, .mSound = 0, .mSoundEchoSwitch = 0, .mLadder = 0, .mIgnorePointer = 0, .mSurfMaterial = 0, .mFloor = false, .v1 = { 1.0, -1.0, -1.0 }, .v2 = { -1.0, -1.0, 1.0 }, .v3 = { -1.0, -1.0, -1.0 }, .mTriIdx = 3},{ .mVtx1 = 2, .mVtx2 = 4, .mVtx3 = 7, .mNormal = 4, .mEdgeTan1 = 11, .mEdgeTan2 = 3, .mEdgeTan3 = 7, .mUnkIdx = 0, .mDot = 2.0, .mMask = 0x8000, .mFriction = 0, .mSound = 0, .mSoundEchoSwitch = 0, .mLadder = 0, .mIgnorePointer = 0, .mSurfMaterial = 0, .mFloor = false, .v1 = { 1.0, 1.0, -1.0 }, .v2 = { 1.0, -1.0, 1.0 }, .v3 = { 1.0, -1.0, -1.0 }, .mTriIdx = 4},{ .mVtx1 = 0, .mVtx2 = 7, .mVtx3 = 6, .mNormal = 3, .mEdgeTan1 = 12, .mEdgeTan2 = 8, .mEdgeTan3 = 7, .mUnkIdx = 0, .mDot = 2.0, .mMask = 0x8000, .mFriction = 0, .mSound = 0, .mSoundEchoSwitch = 0, .mLadder = 0, .mIgnorePointer = 0, .mSurfMaterial = 0, .mFloor = false, .v1 = { -1.0, 1.0, -1.0 }, .v2 = { 1.0, -1.0, -1.0 }, .v3 = { -1.0, -1.0, -1.0 }, .mTriIdx = 5},{ .mVtx1 = 0, .mVtx2 = 5, .mVtx3 = 1, .mNormal = 1, .mEdgeTan1 = 8, .mEdgeTan2 = 13, .mEdgeTan3 = 5, .mUnkIdx = 8, .mDot = 2.0, .mMask = 0x8000, .mFriction = 0, .mSound = 0, .mSoundEchoSwitch = 0, .mLadder = 0, .mIgnorePointer = 0, .mSurfMaterial = 0, .mFloor = true, .v1 = { -1.0, 1.0, -1.0 }, .v2 = { -1.0, 1.0, 1.0 }, .v3 = { 1.0, 1.0, 1.0 }, .mTriIdx = 6},{ .mVtx1 = 1, .mVtx2 = 5, .mVtx3 = 3, .mNormal = 5, .mEdgeTan1 = 1, .mEdgeTan2 = 14, .mEdgeTan3 = 8, .mUnkIdx = 0, .mDot = 2.0, .mMask = 0x8000, .mFriction = 0, .mSound = 0, .mSoundEchoSwitch = 0, .mLadder = 0, .mIgnorePointer = 0, .mSurfMaterial = 0, .mFloor = false, .v1 = { 1.0, 1.0, 1.0 }, .v2 = { -1.0, 1.0, 1.0 }, .v3 = { -1.0, -1.0, 1.0 }, .mTriIdx = 7},{ .mVtx1 = 5, .mVtx2 = 0, .mVtx3 = 6, .mNormal = 8, .mEdgeTan1 = 1, .mEdgeTan2 = 15, .mEdgeTan3 = 3, .mUnkIdx = 0, .mDot = 2.0, .mMask = 0x8000, .mFriction = 0, .mSound = 0, .mSoundEchoSwitch = 0, .mLadder = 0, .mIgnorePointer = 0, .mSurfMaterial = 0, .mFloor = false, .v1 = { -1.0, 1.0, 1.0 }, .v2 = { -1.0, 1.0, -1.0 }, .v3 = { -1.0, -1.0, -1.0 }, .mTriIdx = 8},{ .mVtx1 = 7, .mVtx2 = 4, .mVtx3 = 3, .mNormal = 7, .mEdgeTan1 = 4, .mEdgeTan2 = 16, .mEdgeTan3 = 5, .mUnkIdx = 0, .mDot = 2.0, .mMask = 0x8000, .mFriction = 0, .mSound = 0, .mSoundEchoSwitch = 0, .mLadder = 0, .mIgnorePointer = 0, .mSurfMaterial = 0, .mFloor = false, .v1 = { 1.0, -1.0, -1.0 }, .v2 = { 1.0, -1.0, 1.0 }, .v3 = { -1.0, -1.0, 1.0 }, .mTriIdx = 9},{ .mVtx1 = 2, .mVtx2 = 1, .mVtx3 = 4, .mNormal = 4, .mEdgeTan1 = 1, .mEdgeTan2 = 17, .mEdgeTan3 = 5, .mUnkIdx = 0, .mDot = 2.0, .mMask = 0x8000, .mFriction = 0, .mSound = 0, .mSoundEchoSwitch = 0, .mLadder = 0, .mIgnorePointer = 0, .mSurfMaterial = 0, .mFloor = false, .v1 = { 1.0, 1.0, -1.0 }, .v2 = { 1.0, 1.0, 1.0 }, .v3 = { 1.0, -1.0, 1.0 }, .mTriIdx = 10},{ .mVtx1 = 0, .mVtx2 = 2, .mVtx3 = 7, .mNormal = 3, .mEdgeTan1 = 1, .mEdgeTan2 = 18, .mEdgeTan3 = 4, .mUnkIdx = 0, .mDot = 2.0, .mMask = 0x8000, .mFriction = 0, .mSound = 0, .mSoundEchoSwitch = 0, .mLadder = 0, .mIgnorePointer = 0, .mSurfMaterial = 0, .mFloor = false, .v1 = { -1.0, 1.0, -1.0 }, .v2 = { 1.0, 1.0, -1.0 }, .v3 = { 1.0, -1.0, -1.0 }, .mTriIdx = 11}}
};

void LCollisionIO::LoadMp(std::filesystem::path path, std::weak_ptr<LMapDOMNode> map){
    auto mapArc = map.lock()->GetArchive().lock();
    auto colFile = mapArc->GetFile("col.mp");

    bStream::CFileStream importCol(path.string(), bStream::OpenMode::In);

    unsigned char* importColData = new unsigned char[importCol.getSize()];
    importCol.readBytesTo(importColData, importCol.getSize());

    colFile->SetData(importColData, importCol.getSize());

    delete[] importColData;
}

void LCollisionIO::LoadObj(std::filesystem::path path, std::weak_ptr<LMapDOMNode> map, std::map<std::string, std::string> propertyMap, bool bakeFurniture){    
    
    glm::vec3 bbmin, bbmax;

    std::vector<glm::vec3> positions {};
    std::vector<glm::vec3> normals {};
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
                if(!LGenUtility::VectorContains<glm::vec3>(normals, glm::vec3(0.0f))) normals.push_back(glm::vec3(0.0f));
                tri.mUnkIdx = LGenUtility::VectorIndexOf<glm::vec3>(normals, glm::vec3(0.0f));
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

            if(!LGenUtility::VectorContains<glm::vec3>(positions, tri.v1)) positions.push_back(tri.v1);
            tri.mVtx1 = LGenUtility::VectorIndexOf<glm::vec3>(positions, tri.v1);

            if(!LGenUtility::VectorContains<glm::vec3>(positions, tri.v2)) positions.push_back(tri.v2);
            tri.mVtx2 = LGenUtility::VectorIndexOf<glm::vec3>(positions, tri.v2);

            if(!LGenUtility::VectorContains<glm::vec3>(positions, tri.v3)) positions.push_back(tri.v3);
            tri.mVtx3 = LGenUtility::VectorIndexOf<glm::vec3>(positions, tri.v3);            

            triangles.push_back(tri);
        }
    }

    if(bakeFurniture){
        map.lock()->ForEachChildOfType<LRoomDOMNode>(EDOMNodeType::Room, [&](std::shared_ptr<LRoomDOMNode> room){
            std::vector<std::shared_ptr<LFurnitureDOMNode>> roomFurniture = room->GetChildrenOfType<LFurnitureDOMNode>(EDOMNodeType::Furniture);
            for(std::shared_ptr<LFurnitureDOMNode> furniture : roomFurniture){
                LGenUtility::Log << "Adding Furniture " << furniture->GetName() << std::endl;

                glm::ivec3 scanbox = furniture->GetScanbox();
                glm::vec3 position = furniture->GetPosition();
                glm::vec3 rotation = furniture->GetRotation();

                glm::mat4 m = glm::identity<glm::mat4>();

                m = glm::scale(m, glm::vec3(scanbox.z, scanbox.y, scanbox.x));
                m = glm::rotate(m, glm::radians(rotation.z), glm::vec3(1, 0, 0));
                m = glm::rotate(m, glm::radians(rotation.y), glm::vec3(0, 1, 0));
                m = glm::rotate(m, glm::radians(rotation.x), glm::vec3(0, 0, 1));

                for(std::vector<CollisionTriangle>::iterator tri = CUBE.mTriangles.begin(); tri != CUBE.mTriangles.end(); tri++){
                    CollisionTriangle newTri = (*tri);

                    glm::vec4 v1 = glm::vec4(newTri.v1, 0);
                    glm::vec4 v2 = glm::vec4(newTri.v2, 0);
                    glm::vec4 v3 = glm::vec4(newTri.v3, 0);

                    newTri.v1 = glm::vec3(v1 * m) + glm::vec3(position.z, position.y + scanbox.y, position.x);
                    newTri.v2 = glm::vec3(v2 * m) + glm::vec3(position.z, position.y + scanbox.y, position.x);
                    newTri.v3 = glm::vec3(v3 * m) + glm::vec3(position.z, position.y + scanbox.y, position.x);

                    glm::vec3 e10 = newTri.v2 - newTri.v1; // u
                    glm::vec3 e20 = newTri.v3 - newTri.v1; // v
                    glm::vec3 e01 = newTri.v1 - newTri.v2;
                    glm::vec3 e21 = newTri.v3 - newTri.v2; 

                    glm::vec3 normal1 = glm::normalize(glm::cross(e10, e20));
                    glm::vec3 normal2 = glm::normalize(glm::cross(e01, e21));
                    
                    if(!LGenUtility::VectorContains<glm::vec3>(normals, normal1)) normals.push_back(normal1);
                    newTri.mNormal = LGenUtility::VectorIndexOf<glm::vec3>(normals, normal1);

                    // Edges
                    glm::vec3 tan1 = -glm::normalize(glm::cross(normal1, e10));
                    glm::vec3 tan2 = glm::normalize(glm::cross(normal1, e20));
                    glm::vec3 tan3 = glm::normalize(glm::cross(normal2, e21));
                    
                    if(!LGenUtility::VectorContains<glm::vec3>(normals, tan1)) normals.push_back(tan1);
                    newTri.mEdgeTan1 = LGenUtility::VectorIndexOf<glm::vec3>(normals, tan1);

                    if(!LGenUtility::VectorContains<glm::vec3>(normals, tan2)) normals.push_back(tan2);
                    newTri.mEdgeTan2 = LGenUtility::VectorIndexOf<glm::vec3>(normals, tan2);

                    if(!LGenUtility::VectorContains<glm::vec3>(normals, tan3)) normals.push_back(tan3);
                    newTri.mEdgeTan3 = LGenUtility::VectorIndexOf<glm::vec3>(normals, tan3);
                    
                    glm::vec3 up(0.0f, 1.0f, 0.0f);
                    float angle = (float)glm::acos(glm::dot(normal1, up));

                    angle *= (float)(180.0f / glm::pi<float>());

                    if(glm::abs(angle) >= 0.0f && glm::abs(angle) <= 0.5f){
                        newTri.mUnkIdx = newTri.mEdgeTan1;
                    } else {
                        if(!LGenUtility::VectorContains<glm::vec3>(normals, glm::vec3(0.0f))) normals.push_back(glm::vec3(0.0f));
                        newTri.mUnkIdx = LGenUtility::VectorIndexOf<glm::vec3>(normals, glm::vec3(0.0f));
                    }

                    if(glm::abs(angle) <= 65.0f){
                        newTri.mFloor = true;
                    } else {
                        newTri.mFloor = false;
                    }
                    
                    newTri.mDot = glm::dot(tan3, e21);

                    newTri.mMask = 0x8000;
                    newTri.mFriction = 0;
                    newTri.mTriIdx = triangles.size();

                    newTri.mSound = 0;
                    newTri.mSoundEchoSwitch = 0;
                    newTri.mLadder = 0;
                    newTri.mIgnorePointer = 0;
                    newTri.mSurfMaterial = 0;


                    if(!LGenUtility::VectorContains<glm::vec3>(positions, newTri.v1)) positions.push_back(newTri.v1);
                    newTri.mVtx1 = LGenUtility::VectorIndexOf<glm::vec3>(positions, newTri.v1);

                    if(!LGenUtility::VectorContains<glm::vec3>(positions, newTri.v2)) positions.push_back(newTri.v2);
                    newTri.mVtx2 = LGenUtility::VectorIndexOf<glm::vec3>(positions, newTri.v2);

                    if(!LGenUtility::VectorContains<glm::vec3>(positions, newTri.v3)) positions.push_back(newTri.v3);
                    newTri.mVtx3 = LGenUtility::VectorIndexOf<glm::vec3>(positions, newTri.v3);

                    triangles.push_back(newTri);
                }
            }

        });
    }

    for(auto vtx : positions){
        bbmin = glm::vec3(glm::min(bbmin.x, vtx.x), glm::min(bbmin.y, vtx.y), glm::min(bbmin.z, vtx.z));
        bbmax = glm::vec3(glm::max(bbmax.x, vtx.x), glm::max(bbmax.y, vtx.y), glm::max(bbmax.z, vtx.z));
    }

    LGenUtility::Log << "[CollisionIO:ObjImport]: Bounding Box ["
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

        for(std::vector<glm::vec3>::iterator p = positions.begin(); p != positions.end(); p++){
            stream.writeFloat(p->x);
            stream.writeFloat(p->y);
            stream.writeFloat(p->z);
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
        LGenUtility::Log << "[CollisionIO:ObjImport]: polygoninfo size is " << polygoninfo.getSize() << std::endl;
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
        LGenUtility::Log << "[CollisionIO:ObjImport]: soundpolygoninfo size is " << sndpolygoninfo.getSize() << std::endl;
        auto sndpolygoninfoFile = mapArc->GetFolder("jmp")->GetFile("soundpolygoninfo");
        if(sndpolygoninfoFile == nullptr){
            sndpolygoninfoFile = Archive::File::Create();
            sndpolygoninfoFile->SetName("soundpolygoninfo");
            mapArc->GetFolder("jmp")->AddFile(sndpolygoninfoFile);
        }
        sndpolygoninfoFile->SetData(sndpolygoninfo.getBuffer(), sndpolygoninfo.getSize());
    }
}