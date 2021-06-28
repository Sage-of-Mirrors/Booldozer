#pragma once

#include "DOM.hpp"
#include "modes/EditorModeBase.hpp"
#include "modes/ActorMode.hpp"

#include <filesystem>

// Backend for the main window's UI.
class LBooldozerEditor
{
	// The map that is currently being edited.
	std::shared_ptr<LMapDOMNode> mLoadedMap;

/*=== Editor modes ===*/
	// The mode that is currently executing.
	LEditorModeBase* mCurrentMode;
	// The mode responsible for general actor editing.
	LActorMode mActorMode;

public:
	LBooldozerEditor();

	void Render(float dt, LEditorScene* renderer_scene);

	// Callback for when the user requests to open a map folder.
	void onOpenMapCB();
	// Callback for when the user requests to open one or more room arcs.
	void onOpenRoomsCB();
};
