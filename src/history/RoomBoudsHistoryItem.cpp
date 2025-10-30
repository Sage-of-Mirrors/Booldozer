#include "DOM/DOMNodeBase.hpp"
#include "DOM/RoomDOMNode.hpp"
#include "ResUtil.hpp"
#include "history/RoomBoundsHistoryItem.hpp"
#include "ImGuiNotify.hpp"

LRoomBoundsHistoryItem::LRoomBoundsHistoryItem(std::shared_ptr<LRoomDataDOMNode> node, glm::vec3 other, bool isMin, LEditorScene* scene) :
	mRoomDataNode(node), mOther(other), mIsMin(isMin), mScene(scene)
{

}

void LRoomBoundsHistoryItem::Undo()
{
	if (mRoomDataNode == nullptr)
		return;

    glm::vec3 temp = mIsMin ? mRoomDataNode->GetMin() : mRoomDataNode->GetMax();
    ImGuiToast undoNotif(ImGuiToastType::Success, 3000);
    undoNotif.setTitle("Undo");
    undoNotif.setContent("Transform %s %s Bounds", mIsMin ? "Scale" : "Translate", mRoomDataNode->GetParentOfType<LRoomDOMNode>(EDOMNodeType::Room).lock()->GetName().c_str());
    ImGui::InsertNotification(undoNotif);
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
