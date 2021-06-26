#pragma once

#include "DOM.hpp"
#include "glm/glm.hpp"
#include "history/EditorHistory.hpp"

struct LVec3HistoryItem : public LEditorHistoryItem
{
private:
	// The node that this history item affects.
	std::shared_ptr<LBGRenderDOMNode> mAffectedNode;
	// The delta that will be applied on undo/redo.
	glm::vec3 mDelta;
	// Whether this history item affects scale (true) or translation (false).
	bool mIsScale;

public:
	LVec3HistoryItem(std::shared_ptr<LBGRenderDOMNode> node, glm::vec3 delta, bool is_scale = false);

	virtual void Undo() override;
	virtual void Redo() override;
};
