#include "history/RoomMoveHistoryItem.hpp"

LRoomMoveHistoryItem::LRoomMoveHistoryItem(std::shared_ptr<LRoomDOMNode> node, glm::vec3 delta, LEditorScene* scene) :
	mAffectedNode(node), mDelta(delta), mScene(scene)
{

}

void LRoomMoveHistoryItem::Undo()
{
	if (mAffectedNode == nullptr)
		return;

    std::shared_ptr<LRoomDataDOMNode> data = mAffectedNode->GetChildrenOfType<LRoomDataDOMNode>(EDOMNodeType::RoomData).front();

	data->SetMax(data->GetMax() - mDelta);
	data->SetMin(data->GetMin() - mDelta);

	mAffectedNode->SetRoomModelDelta(mAffectedNode->GetRoomModelDelta() - mDelta);
	// add delta to all child transforms
	for(auto child : mAffectedNode->GetChildrenOfType<LEntityDOMNode>(EDOMNodeType::Entity)){
		(*child->GetMat())[3].x -= mDelta.x;
		(*child->GetMat())[3].y -= mDelta.y;
		(*child->GetMat())[3].z -= mDelta.z;
		child->SetPosition(child->GetPosition() - mDelta);
	}

    mScene->SetDirty();
}

void LRoomMoveHistoryItem::Redo()
{
	if (mAffectedNode == nullptr)
		return;

    std::shared_ptr<LRoomDataDOMNode> data = mAffectedNode->GetChildrenOfType<LRoomDataDOMNode>(EDOMNodeType::RoomData).front();

	data->SetMax(data->GetMax() + mDelta);
	data->SetMin(data->GetMin() + mDelta);

	mAffectedNode->SetRoomModelDelta(mAffectedNode->GetRoomModelDelta() + mDelta);
	// add delta to all child transforms
	for(auto child : mAffectedNode->GetChildrenOfType<LEntityDOMNode>(EDOMNodeType::Entity)){
		(*child->GetMat())[3].x += mDelta.x;
		(*child->GetMat())[3].y += mDelta.y;
		(*child->GetMat())[3].z += mDelta.z;
		child->SetPosition(child->GetPosition() + mDelta);
	}

    mScene->SetDirty();
}
