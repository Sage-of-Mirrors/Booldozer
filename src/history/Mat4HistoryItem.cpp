#include "history/Mat4HistoryItem.hpp"
#include "ImGuiNotify.hpp"

LMat4HistoryItem::LMat4HistoryItem(std::shared_ptr<LBGRenderDOMNode> node, glm::mat4 other) : mAffectedNode(node), mOther(other)
{
}

void LMat4HistoryItem::Undo()
{
    glm::mat4 temp = *mAffectedNode->GetMat();
	if(mAffectedNode != nullptr) (*mAffectedNode->GetMat()) = mOther;
    mOther = temp;

    ImGuiToast undoNotif(ImGuiToastType::Success, 3000);
    undoNotif.setTitle("Undo");
    undoNotif.setContent("Transform %s", mAffectedNode->GetName().c_str());
    ImGui::InsertNotification(undoNotif);
}

void LMat4HistoryItem::Redo()
{
    glm::mat4 temp = *mAffectedNode->GetMat();
	if(mAffectedNode != nullptr) (*mAffectedNode->GetMat()) = mOther;
    mOther = temp;
}
