#pragma once

#include "DOM.hpp"
#include "modes/EditorModeBase.hpp"
#include "modes/ActorMode.hpp"
#include "modes/EnemyMode.hpp"
#include "modes/ItemMode.hpp"
#include "modes/PathMode.hpp"
#include "modes/DoorMode.hpp"
#include "modes/EventMode.hpp"
#include "io/PrmIO.hpp"
#include "ImGuizmo.h"

#include <filesystem>
#include <memory>
#include <nlohmann/json.hpp>

enum class EEditorMode : uint32_t
{
	Actor_Mode,
	Enemy_Mode,
	Door_Mode,
	Path_Mode,
	Item_Mode,
	Event_Mode,
	Collision_Mode,
};

// Backend for the main window's UI.
class LBooldozerEditor
{
	// The map that is currently being edited.
	std::shared_ptr<LMapDOMNode> mLoadedMap;

	// Param files for this root
	LPrmIO mGhostConfigs;

/*=== Editor modes ===*/
	// The mode that is currently executing.
	LEditorModeBase* mCurrentMode;
	// The mode responsible for general actor editing.
	LActorMode mActorMode;
	LEnemyMode mEnemyMode;
	LItemMode mItemMode;
	LPathMode mPathMode;
	LDoorMode mDoorMode;
	LEventMode mEventMode;

	void OpenMap(std::string file_path);
	void SaveMapToFiles(std::string folder_path);
	void SaveMapToArchive(std::string file_path);

	void RenderNoRootPopup();

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
	void onSaveMapArchiveCB();

	void onPlaytestCB();

	LEditorSelection* GetSelectionManager() { return mCurrentMode->GetSelectionManager(); }

	void SetGizmo(ImGuizmo::OPERATION mode);

	// Switches the current mode to the given new one.
	void ChangeMode();
};
