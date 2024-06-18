#include "DOM/MapCollisionDOMNode.hpp"
#include "DOM/RoomDOMNode.hpp"
#include "UIUtil.hpp"
#include "GenUtil.hpp"
#include "imgui.h"
#include "../lib/bStream/bstream.h"

LMapCollisionDOMNode::LMapCollisionDOMNode(std::string name) : Super(name)
{
	mType = EDOMNodeType::MapCollision;
}

std::string LMapCollisionDOMNode::GetName()
{
	return "Collision"; //uh
}

void LMapCollisionDOMNode::RenderDetailsUI(float dt)
{
	mDirty = false;

	if(ImGui::InputInt("Z Slice", &mGridZLevel)){
		mDirty = true;
	}
	
	if(ImGui::InputInt("Y Slice", &mGridYLevel)){
		mDirty = true;
	}
	
	mGridZLevel %= mGridDimension[2];
	mGridYLevel %= mGridDimension[1];
}

bool LMapCollisionDOMNode::Load(bStream::CMemoryStream* stream)
{
    
	mGridScale = glm::vec3(stream->readFloat(), stream->readFloat(), stream->readFloat());
	mMinBounds = glm::vec3(stream->readFloat(), stream->readFloat(), stream->readFloat());
	mAxisLengths = glm::vec3(stream->readFloat(), stream->readFloat(), stream->readFloat());

	uint32_t vertexDataOffset = stream->readUInt32();
	uint32_t normalDataOffset = stream->readUInt32();
	uint32_t triangleDataOffset = stream->readUInt32();
	uint32_t triangleGroupOffset = stream->readUInt32();
	uint32_t gridIndexOffset = stream->readUInt32();
	
	//	duplicated for some reason
	gridIndexOffset = stream->readUInt32();

	uint32_t endOffset = stream->readUInt32();

	stream->seek(vertexDataOffset);

	for (size_t i = 0; i < (normalDataOffset - vertexDataOffset) / 0x0C; i++)
	{
		mPositionData.push_back(glm::vec3(stream->readFloat(), stream->readFloat(), stream->readFloat()));
	}
	
	stream->seek(normalDataOffset);
	for (size_t i = 0; i < (triangleDataOffset - normalDataOffset) / 0x0C; i++)
	{
		mNormalData.push_back(glm::vec3(stream->readFloat(), stream->readFloat(), stream->readFloat()));
	}

	stream->seek(triangleDataOffset);
	for (size_t i = 0; i < (triangleGroupOffset - triangleDataOffset) / 0x18; i++)
	{
		std::shared_ptr<LCollisionTriangle> triangle = std::make_shared<LCollisionTriangle>();

		triangle->name = fmt::format("Triangle {}", i);

		triangle->positionIdx[0] = stream->readUInt16();
		triangle->positionIdx[1] = stream->readUInt16();
		triangle->positionIdx[2] = stream->readUInt16();

		triangle->normalIdx = stream->readUInt16();

		triangle->edgeTanIdx[0] = stream->readUInt16();
		triangle->edgeTanIdx[1] = stream->readUInt16();
		triangle->edgeTanIdx[2] = stream->readUInt16();

		triangle->unkIdx = stream->readUInt16();
		triangle->dot = stream->readFloat();
		triangle->enabledMask = stream->readUInt16();
		triangle->friction = stream->readUInt16();

		mTriangles.push_back(triangle);
	}

	uint32_t xCellCount = (uint32_t)(glm::floor(mAxisLengths.x / mGridScale.x)) + 1;
	uint32_t yCellCount = (uint32_t)(glm::floor(mAxisLengths.y / mGridScale.y)) + 1;
	uint32_t zCellCount = (uint32_t)(glm::floor(mAxisLengths.z / mGridScale.z)) + 1;

	mGridDimension[0] = xCellCount;
	mGridDimension[1] = yCellCount;
	mGridDimension[2] = zCellCount;

	stream->seek(triangleGroupOffset);
	while(stream->tell() < gridIndexOffset){
		std::shared_ptr<LTriangleGroup> allGroup = std::make_shared<LTriangleGroup>();

		allGroup->startOffset = stream->tell();

		uint16_t idx = stream->readUInt16();
		while(idx != 0xFFFF && stream->tell() < gridIndexOffset && idx < mTriangles.size()){
			allGroup->triangles.push_back(mTriangles.at(idx));
			idx = stream->readUInt16();
		}

		allGroup->endOffset = stream->tell();

		allGroup->render = true;
		mTriangleGroups.push_back(allGroup);
	}

	uint32_t cellCount = (endOffset - gridIndexOffset) / 8;
	for(int i = 0; i < cellCount; i++){
		mGrid.push_back(std::make_shared<LCollisionGridCell>());
	}

	for(int z = 0; z < zCellCount; z++){
		for(int y = 0; y < yCellCount; y++){
			for(int x = 0; x < xCellCount; x++){
				int32_t allGroupOffset = stream->readInt32();
				int32_t floorGroupOffset = stream->readInt32();

				auto gridCell = mGrid[x + (y * xCellCount) + (z * xCellCount * yCellCount)];

				int curIdxCount = 0;
				for(auto group : mTriangleGroups){
					if(allGroupOffset > curIdxCount && allGroupOffset < curIdxCount + group->triangles.size()){
						gridCell->allTriangles = group;
					}
					if(floorGroupOffset > curIdxCount && floorGroupOffset < curIdxCount + group->triangles.size()){
						gridCell->floorTriangles = group;
					}
					curIdxCount += group->triangles.size();
				}
				
			}
		}
	}

	//std::cout << "Reading Grid at " << std::hex << gridIndexOffset << std::endl; 
	stream->seek(gridIndexOffset);
	/*
	for (int z = 0; z < zCellCount; z++){
		std::vector<std::vector<std::shared_ptr<LCollisionGridCell>>> yAxis;
		for (int y = 0; y < yCellCount; y++){
			std::vector<std::shared_ptr<LCollisionGridCell>> xAxis;
			for (int x = 0; x < xCellCount; x++){
				uint32_t allGroupOffset = stream->readInt32();
				uint32_t floorGroupOffset = stream->readInt32();

				auto gridCell = std::make_shared<LCollisionGridCell>();

				int curIdxCount = 0;
				for(auto group : mTriangleGroups){
					if(allGroupOffset > curIdxCount && allGroupOffset < curIdxCount + group->triangles.size()){
						gridCell->allTriangles = group;
					}
					if(floorGroupOffset > curIdxCount && floorGroupOffset < curIdxCount + group->triangles.size()){
						gridCell->floorTriangles = group;
					}
					curIdxCount += group->triangles.size();
				}


				xAxis.push_back(gridCell);
			}
			yAxis.push_back(xAxis);
		}
		mGrid.push_back(yAxis);
	}
	*/

	std::cout << "[CollisionDOMNode]: Finished reading Collision File" << std::endl;

	return false;
}

bool LMapCollisionDOMNode::Save(bStream::CMemoryStream* stream)
{
	return false;
}

void LMapCollisionDOMNode::PostProcess()
{

}

void LMapCollisionDOMNode::PreProcess()
{

}
