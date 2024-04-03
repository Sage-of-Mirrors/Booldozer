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

	uint32_t xCellCount = (uint32_t)(glm::round(mAxisLengths.x / mGridScale.x));
	uint32_t yCellCount = (uint32_t)(glm::round(mAxisLengths.y / mGridScale.y));
	uint32_t zCellCount = (uint32_t)(glm::round(mAxisLengths.z / mGridScale.z));

	mGridDimension[0] = xCellCount;
	mGridDimension[1] = yCellCount;
	mGridDimension[2] = zCellCount;

	//std::cout << "Reading Grid at " << std::hex << gridIndexOffset << std::endl; 
	stream->seek(gridIndexOffset);
	for (int z = 0; z < zCellCount; z++){
		std::vector<std::vector<std::shared_ptr<LCollisionGridCell>>> yAxis;
		for (int y = 0; y < yCellCount; y++){
			std::vector<std::shared_ptr<LCollisionGridCell>> xAxis;
			for (int x = 0; x < xCellCount; x++){
				std::shared_ptr<LCollisionGridCell> gridCell = std::make_shared<LCollisionGridCell>();

				std::shared_ptr<LTriangleGroup> allGroup = std::make_shared<LTriangleGroup>();
				std::shared_ptr<LTriangleGroup> floorGroup = std::make_shared<LTriangleGroup>();

				allGroup->render = true;
				floorGroup->render = false;

				uint32_t allGroupIdx = stream->readUInt32();
				uint32_t floorGroupIdx = stream->readUInt32();

				size_t gridPos = stream->tell();

				stream->seek(triangleGroupOffset + (allGroupIdx * 2));
				uint16_t triangleIdx = stream->readUInt16();
				while(triangleIdx != 0xFFFF){
					allGroup->triangles.push_back(mTriangles.at(triangleIdx));
					triangleIdx = stream->readUInt16();
				}

				stream->seek(triangleGroupOffset + (floorGroupIdx * 2));
				triangleIdx = stream->readUInt16();
				while(triangleIdx != 0xFFFF){
					floorGroup->triangles.push_back(mTriangles.at(triangleIdx));
					triangleIdx = stream->readUInt16();
				}

				stream->seek(gridPos);

				mTriangleGroups.push_back(allGroup);
				mTriangleGroups.push_back(floorGroup);

				gridCell->allTriangles = allGroup;
				gridCell->floorTriangles = floorGroup;

				xAxis.push_back(gridCell);
			}
			yAxis.push_back(xAxis);
		}
		mGrid.push_back(yAxis);
	}

	std::cout << "Finished reading Collision File" << std::endl;

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
