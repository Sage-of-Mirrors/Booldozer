#pragma once

#include "DOM.hpp"
#include "glm/glm.hpp"
#include "history/EditorHistory.hpp"
#include "scene/EditorScene.hpp"

struct LRoomMoveHistoryItem : public LEditorHistoryItem
{
private:
	// The node that this history item affects.
	std::shared_ptr<LRoomDOMNode> mAffectedNode;
	LEditorScene* mScene;
	glm::vec3 mDelta;


public:
	LRoomMoveHistoryItem(std::shared_ptr<LRoomDOMNode> node, glm::vec3 delta, LEditorScene* scene);

	virtual void Undo() override;
	virtual void Redo() override;
};
