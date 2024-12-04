#pragma once

#include "DOM.hpp"
#include "glm/glm.hpp"
#include "history/EditorHistory.hpp"

struct LMat4HistoryItem : public LEditorHistoryItem
{
private:
	// The node that this history item affects.
	std::shared_ptr<LBGRenderDOMNode> mAffectedNode;
	// The delta that will be applied on undo/redo.
	glm::mat4 mOther;

public:
	LMat4HistoryItem(std::shared_ptr<LBGRenderDOMNode> node, glm::mat4 other);

	virtual void Undo() override;
	virtual void Redo() override;
};
