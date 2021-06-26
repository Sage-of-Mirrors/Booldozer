#pragma once

#include "DOM.hpp"
#include "glm/glm.hpp"
#include "history/EditorHistory.hpp"

struct LQuatHistoryItem : public LEditorHistoryItem
{
private:
	// The node that this history item affects.
	std::shared_ptr<LBGRenderDOMNode> mAffectedNode;
	// The delta that will be applied on undo/redo.
	glm::quat mDelta;
	// Whether this history item affects scale (true) or translation (false).
	bool mIsScale;

public:
	LQuatHistoryItem(std::shared_ptr<LBGRenderDOMNode> node, glm::quat delta, bool is_scale = false);

	virtual void Undo() override;
	virtual void Redo() override;
};
