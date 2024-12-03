#include "history/Mat4HistoryItem.hpp"

LMat4HistoryItem::LMat4HistoryItem(std::shared_ptr<LBGRenderDOMNode> node, glm::mat4 delta) : mAffectedNode(node), mDelta(delta)
{
    std::cout << "Added mat4 undo item" << std::endl;
}

void LMat4HistoryItem::Undo()
{
	if(mAffectedNode != nullptr) (*mAffectedNode->GetMat()) *= glm::inverse(mDelta);
}

void LMat4HistoryItem::Redo()
{
    if(mAffectedNode != nullptr) (*mAffectedNode->GetMat()) *= mDelta;
}
