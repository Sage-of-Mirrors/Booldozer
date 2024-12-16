#pragma once

#include "DOM.hpp"
#include "modes/EditorModeBase.hpp"
#include "modes/ActorMode.hpp"
#include "modes/EnemyMode.hpp"
#include "modes/ItemMode.hpp"
#include "modes/PathMode.hpp"
#include "modes/DoorMode.hpp"
#include "modes/EventMode.hpp"
#include "modes/BooMode.hpp"
#include "io/PrmIO.hpp"
#include "ImGuizmo.h"

#include <filesystem>
#include <memory>
#include <json.hpp>

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
	LItemMode mItemMode;
	LPathMode mPathMode;
	LDoorMode mDoorMode;
	LEventMode mEventMode;
	LBooMode mBooMode;

	// UI Windows
	bool bInitialized { false };

	bool mOnStartPopup { false };
	bool mSaveMapClicked { false };
	bool mClickedMapSelect { false };
	bool mClickedMapClear { false };
	bool mSaveConfigsClicked { false };
	int32_t mSelectedMap { -1 };
	int32_t mMapNameDialogEditingNameIdx { -1 };
	std::string mMapNameDialogEditingNameStr { "" };

	uint32_t mMainDockSpaceID { 0 };
	uint32_t mDockNodeLeftID { 0 };
	uint32_t mDockNodeRightID { 0 };
	uint32_t mDockNodeUpLeftID { 0 };
	uint32_t mDockNodeDownLeftID { 0 };

	// Rendering surface
	uint32_t mFbo, mRbo, mViewTex, mPickTex;

	float mPrevWinWidth { -1.0f };
	float mPrevWinHeight { -1.0f };

	void SaveMap(std::string path);
	void LoadMap(std::string path, LEditorScene* scene);

	void OpenMap(std::string file_path);
	void AppendMap(std::string map_path);
	void SaveMapToFiles(std::string folder_path);
	void SaveMapToArchive(std::string file_path);

	void RenderNoRootPopup();
public:
	LBooldozerEditor();
	~LBooldozerEditor();
	
	// Param files for this root
	LPrmIO mGhostConfigs;

	EEditorMode CurrentMode;

	void Render(float dt, LEditorScene* renderer_scene);

	// Callback for when the user requests to open a map folder.
	void onOpenMapCB();

	void onAppendMapCB();

	void onClearMapCB();

	// Callback for when the user requests to open one or more room arcs.
	void onOpenRoomsCB();

	// Callback for when the user requests to save a map
	void onSaveMapCB();
	void onSaveMapArchiveCB();
	void onGCMExportCB();

	void SaveActorConfigs();
	void onPlaytestCB();

	LEditorSelection* GetSelectionManager() { return mCurrentMode->GetSelectionManager(); }

	void SetGizmo(ImGuizmo::OPERATION mode);

	// Switches the current mode to the given new one.
	void ChangeMode();

	bool mOpenActorEditor { false };
	bool mOpenControlsDialog { false };
	bool mOpenBannerEditor { false };
	bool mOpenProjectManager { false };
	bool mOpenMenuEditor { false };
};
