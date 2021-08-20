#pragma once

#include "DOM.hpp"
#include "modes/EditorModeBase.hpp"
#include "modes/ActorMode.hpp"
#include "modes/EnemyMode.hpp"

#include <filesystem>
#include <memory>

enum class EEditorMode : uint32_t
{
	Actor_Mode,
	Enemy_Mode,
	Door_Mode,
	Item_Mode,
	Event_Mode,
	Collision_Mode
};

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
	LEnemyMode mEnemyMode;

	void OpenMap(std::string file_path);

public:
	LBooldozerEditor();

	EEditorMode CurrentMode;

	void Render(float dt, LEditorScene* renderer_scene);

	// Callback for when the user requests to open a map folder.
	void onOpenMapCB();
	// Callback for when the user requests to open one or more room arcs.
	void onOpenRoomsCB();

	// Callback for when the user requests to save a map
	void onSaveMapCB();

	// Switches the current mode to the given new one.
	void ChangeMode();
};
