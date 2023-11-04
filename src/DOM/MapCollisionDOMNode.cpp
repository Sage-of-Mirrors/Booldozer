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
	if(ImGui::TreeNode("Groups")){
		ImGui::Separator();
		for(auto group : mTriangleGroups){
			if(ImGui::Checkbox(fmt::format("##enabledNode{}", group->name).data(), &group->render)) mDirty = true;
			ImGui::SameLine();
			if(ImGui::TreeNode(group->name.data())){
				for(auto triangle : group->triangles){
					if(auto triangleLocked = triangle.lock()){
						ImGui::Text(triangleLocked->name.c_str());
					}
				}
				ImGui::TreePop();
			}
		}
		ImGui::TreePop();
	}

	ImGui::Separator();

	if(ImGui::TreeNode("Collision Grid")){
		ImGui::Checkbox("Render Walkable Grid", &mGridRender);
		ImGui::InputInt("Y Level", &mGridYLevel);
		if(mGridYLevel >= mGridDimension[1]) mGridYLevel = 0;
		if(ImGui::BeginTable("##ColGridTable", mGridDimension[2])){
			for(int z = 0; z <  mGridDimension[2]; z++){
				for(int x = 0; x < mGridDimension[0]; x++){
					uint32_t index = x + (mGridYLevel * mGridDimension[0]) + (z * mGridDimension[0] * mGridDimension[1]);

					std::shared_ptr<LCollisionGridCell> cell = mGrid[index];

					ImGui::TableNextColumn();
					if(cell->allTriangles.lock() == mTriangleGroups.front()){
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0, 0.0, 0.0, 1.0));
					}
					if(ImGui::Button(fmt::format("##{},{},{}", x, mGridYLevel, z).c_str(), ImVec2(-FLT_MIN, 0.0f))){
						mSelectedCell = cell;
						mDirty = true;
					}
					
					if(cell->allTriangles.lock() == mTriangleGroups.front()){
						ImGui::PopStyleColor();
					}

				}
				ImGui::TableNextRow();
			}
			ImGui::EndTable();
		}
		ImGui::TreePop();
	}
	ImGui::Separator();

	if(mSelectedCell != nullptr){
		if(auto allGroup = mSelectedCell->allTriangles.lock()){
			if(ImGui::TreeNode("All Triangles")){
				for(auto triangle : allGroup->triangles){
					if(auto triangleLocked = triangle.lock()){
						ImGui::Text(triangleLocked->name.c_str());
					} else {
						std::cout << "failed to lock group" << std::endl; 
					}
				}
				ImGui::TreePop();
			}
		}

		if(auto floorGroup = mSelectedCell->floorTriangles.lock()){
			if(ImGui::TreeNode("Floor Triangles")){
				for(auto triangle : floorGroup->triangles){
					if(auto triangleLocked = triangle.lock()){
						ImGui::Text(triangleLocked->name.c_str());
					} else {
						std::cout << "failed to lock floor group" << std::endl; 
					}
				}
				ImGui::TreePop();
			}
		}
	}
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

	stream->seek(triangleGroupOffset);
	int groupIdx = 0;
	while(stream->tell() < gridIndexOffset){

		std::shared_ptr<LTriangleGroup> group = std::make_shared<LTriangleGroup>();

		group->startOffset = (stream->tell() - triangleGroupOffset) / 2;
		std::cout << "Reading group with start offest of " << group->startOffset << std::endl;

		uint16_t triangleIdx = stream->readUInt16();
		while(triangleIdx != 0xFFFF){
			if(triangleIdx < mTriangles.size()) 
				group->triangles.push_back(mTriangles.at(triangleIdx));

			triangleIdx = stream->readUInt16();
		}

		group->name = fmt::format("Group {}", groupIdx++);
		group->endOffset = (stream->tell() - triangleGroupOffset) / 2;
		mTriangleGroups.push_back(group);
	}

	std::cout << "Read " << groupIdx << " groups" << std::endl;

	uint32_t xCellCount = (uint32_t)(glm::round(mAxisLengths.x / mGridScale.x));
	uint32_t yCellCount = (uint32_t)(glm::round(mAxisLengths.y / mGridScale.y));
	uint32_t zCellCount = (uint32_t)(glm::round(mAxisLengths.z / mGridScale.z));

	mGridDimension[0] = xCellCount;
	mGridDimension[1] = yCellCount;
	mGridDimension[2] = zCellCount;

	//std::cout << "Reading Grid at " << std::hex << gridIndexOffset << std::endl; 
	stream->seek(gridIndexOffset);
	mGrid.reserve(xCellCount * yCellCount * zCellCount);
	for (size_t i = 0; i < xCellCount * yCellCount * zCellCount; i++)
	{
		uint32_t allTriangleGroupIdx = stream->readUInt32();
		uint32_t floorTriangleGroupIdx = stream->readUInt32();

		std::shared_ptr<LCollisionGridCell> cell = std::make_shared<LCollisionGridCell>();

		auto allGroupSearch = std::find_if(mTriangleGroups.begin(), mTriangleGroups.end(), [allTriangleGroupIdx] (std::shared_ptr<LTriangleGroup> group) { return group->startOffset == allTriangleGroupIdx; });
		auto floorGroupSearch = std::find_if(mTriangleGroups.begin(), mTriangleGroups.end(), [floorTriangleGroupIdx] (std::shared_ptr<LTriangleGroup> group) { return group->startOffset <= floorTriangleGroupIdx && group->endOffset > floorTriangleGroupIdx;  });

		if(allGroupSearch != mTriangleGroups.end()){
			cell->allTriangles = *allGroupSearch;
		} else {
			std::cout << "Couldnt find all group starting at " << allTriangleGroupIdx << std::endl;
		}

		if(floorGroupSearch != mTriangleGroups.end()){
			cell->floorTriangles = *floorGroupSearch;
		} else {
			std::cout << "Couldnt find floor group starting at " << floorTriangleGroupIdx << std::endl;
		}

		mGrid.push_back(cell);
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
