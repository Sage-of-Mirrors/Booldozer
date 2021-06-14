#pragma once

#include "DOM.hpp"

// Backend for the main window's UI.
class LBooldozerEditor
{
	// The map that is currently being edited.
	LMapDOMNode mLoadedMap;

public:
	LBooldozerEditor();

	// Callback for when the user requests to open a map folder.
	void onOpenMapCB();
	// Callback for when the user requests to open one or more room arcs.
	void onOpenRoomsCB();
};
