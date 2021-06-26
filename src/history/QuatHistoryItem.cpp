#include "history/QuatHistoryItem.hpp"

LQuatHistoryItem::LQuatHistoryItem(std::shared_ptr<LBGRenderDOMNode> node, glm::quat delta, bool is_scale) :
	mAffectedNode(node), mDelta(delta), mIsScale(is_scale)
{

}

void LQuatHistoryItem::Undo()
{
	if (mAffectedNode == nullptr)
		return;

	glm::quat curQuat = mAffectedNode->GetRotation();
	curQuat -= mDelta;
	mAffectedNode->SetRotation(curQuat);
}

void LQuatHistoryItem::Redo()
{
	if (mAffectedNode == nullptr)
		return;

	glm::quat curQuat = mAffectedNode->GetRotation();
	curQuat += mDelta;
	mAffectedNode->SetRotation(curQuat);
}
