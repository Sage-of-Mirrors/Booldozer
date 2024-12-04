#include "history/RoomBoundsHistoryItem.hpp"

LRoomBoundsHistoryItem::LRoomBoundsHistoryItem(std::shared_ptr<LRoomDataDOMNode> node, glm::vec3 other, bool isMin, LEditorScene* scene) :
	mRoomDataNode(node), mOther(other), mIsMin(isMin), mScene(scene)
{

}

void LRoomBoundsHistoryItem::Undo()
{
	if (mRoomDataNode == nullptr)
		return;

    glm::vec3 temp = mIsMin ? mRoomDataNode->GetMin() : mRoomDataNode->GetMax();
	if(mIsMin){
        mRoomDataNode->SetMin(mOther);
    } else {
        mRoomDataNode->SetMax(mOther);
    }
    mOther = temp;
    mScene->SetDirty();
}

void LRoomBoundsHistoryItem::Redo()
{
	if (mRoomDataNode == nullptr)
		return;

    glm::vec3 temp = mIsMin ? mRoomDataNode->GetMin() : mRoomDataNode->GetMax();
	if(mIsMin){
        mRoomDataNode->SetMin(mOther);
    } else {
        mRoomDataNode->SetMax(mOther);
    }
    mOther = temp;
    mScene->SetDirty();
}
