#include "history/Mat4HistoryItem.hpp"

LMat4HistoryItem::LMat4HistoryItem(std::shared_ptr<LBGRenderDOMNode> node, glm::mat4 other) : mAffectedNode(node), mOther(other)
{
}

void LMat4HistoryItem::Undo()
{
    glm::mat4 temp = *mAffectedNode->GetMat();
	if(mAffectedNode != nullptr) (*mAffectedNode->GetMat()) = mOther;
    mOther = temp;
}

void LMat4HistoryItem::Redo()
{
    glm::mat4 temp = *mAffectedNode->GetMat();
	if(mAffectedNode != nullptr) (*mAffectedNode->GetMat()) = mOther;
    mOther = temp;
}
