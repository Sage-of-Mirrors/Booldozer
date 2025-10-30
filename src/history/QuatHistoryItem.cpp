#include "history/QuatHistoryItem.hpp"
#include "ImGuiNotify.hpp"

LQuatHistoryItem::LQuatHistoryItem(std::shared_ptr<LBGRenderDOMNode> node, glm::quat delta, bool is_scale) :
	mAffectedNode(node), mDelta(delta), mIsScale(is_scale)
{

}

void LQuatHistoryItem::Undo()
{
	if (mAffectedNode == nullptr)
		return;

	*mAffectedNode->GetMat() *= glm::inverse(glm::toMat4(mDelta));
    ImGuiToast undoNotif(ImGuiToastType::Success, 3000);
    undoNotif.setTitle("Undo");
    undoNotif.setContent("Rotate %s", mAffectedNode->GetName().c_str());
    ImGui::InsertNotification(undoNotif);
}

void LQuatHistoryItem::Redo()
{
	if (mAffectedNode == nullptr)
		return;

	*mAffectedNode->GetMat() *= glm::toMat4(mDelta);
}
