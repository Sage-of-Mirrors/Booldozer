#include "history/Vec3HistoryItem.hpp"
#include "ImGuiNotify.hpp"

LVec3HistoryItem::LVec3HistoryItem(std::shared_ptr<LBGRenderDOMNode> node, glm::vec3 delta, bool is_scale) :
	mAffectedNode(node), mDelta(delta), mIsScale(is_scale)
{

}

void LVec3HistoryItem::Undo()
{
	if (mAffectedNode == nullptr)
		return;

	ImGuiToast undoNotif(ImGuiToastType::Success, 3000);
	undoNotif.setTitle("Undo");
	undoNotif.setContent("%s %s", mIsScale ? "Scale" : "Translate", mAffectedNode->GetName().c_str());
	ImGui::InsertNotification(undoNotif);

	glm::vec3 curVec = mIsScale ? mAffectedNode->GetScale() : mAffectedNode->GetPosition();
	curVec -= mDelta;
	mIsScale ? mAffectedNode->SetScale(curVec) : mAffectedNode->SetPosition(curVec);
}

void LVec3HistoryItem::Redo()
{
	if (mAffectedNode == nullptr)
		return;

	glm::vec3 curVec = mIsScale ? mAffectedNode->GetScale() : mAffectedNode->GetPosition();
	curVec += mDelta;
	mIsScale ? mAffectedNode->SetScale(curVec) : mAffectedNode->SetPosition(curVec);
}
