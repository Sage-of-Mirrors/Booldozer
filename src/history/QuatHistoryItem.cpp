#include "history/QuatHistoryItem.hpp"

LQuatHistoryItem::LQuatHistoryItem(std::shared_ptr<LBGRenderDOMNode> node, glm::quat delta, bool is_scale) :
	mAffectedNode(node), mDelta(delta), mIsScale(is_scale)
{

}

void LQuatHistoryItem::Undo()
{
	if (mAffectedNode == nullptr)
		return;

	*mAffectedNode->GetMat() *= glm::inverse(glm::toMat4(mDelta));
}

void LQuatHistoryItem::Redo()
{
	if (mAffectedNode == nullptr)
		return;

	*mAffectedNode->GetMat() *= glm::toMat4(mDelta);
}
