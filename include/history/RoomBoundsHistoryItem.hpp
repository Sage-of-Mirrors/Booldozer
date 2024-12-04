#pragma once

#include "DOM.hpp"
#include "glm/glm.hpp"
#include "history/EditorHistory.hpp"
#include "scene/EditorScene.hpp"

struct LRoomBoundsHistoryItem : public LEditorHistoryItem
{
private:
	std::shared_ptr<LRoomDataDOMNode> mRoomDataNode;
	LEditorScene* mScene;
	glm::vec3 mOther;
	bool mIsMin;

public:
	LRoomBoundsHistoryItem(std::shared_ptr<LRoomDataDOMNode> node, glm::vec3 other, bool isMin, LEditorScene* scene);

	virtual void Undo() override;
	virtual void Redo() override;
};
