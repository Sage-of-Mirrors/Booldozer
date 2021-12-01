#pragma once

#include <stdint.h>
#include "map_file.h"

// The data that defines a map and its rooms.
typedef struct LMapData
{
  /* 0x00 */ uint8_t mRoomCount;  // Total number of rooms in this map
  /* 0x01 */ uint8_t mRoomCount2; // Duplicate of room count; purpose unknown
  /* 0x02 */ uint16_t mPadding;   // Assumed to be padding, might not be?
  
  /* 0x04 */ char** mRoomResPaths;                // Paths to room resources, either archives or raw BINs
  /* 0x08 */ uint32_t** mRoomAdjacencyLists;      // Lists of rooms that should be loaded while in a specific room
  /* 0x0C */ LAltResourceData* mAltRoomModelDefs; // Structs with model paths intended to replace a room's main model. Only used by Guest Room (room28)?
  /* 0x10 */ uint32_t mPaddingInt;                // Assumed to be padding, might not be?
  
  /* 0x14 */ LRoomData* mRoomDefs; // Structs defining the map's rooms
  /* 0x18 */ LDoorData* mDoorDefs; // Structs defining the doors within the map
} LMapData;

// Attempts to load the external data file; returns whether it succeeded.
uint32_t TryLoadMapFile(uint16_t MapID);

// Updates the offsets in the map file to pointers.
void UpdateMapFileOffsets();

// Our new function to load the room data from file.
void InitMap_External(uint16_t MapID);
// Our new function to delete the file data on map destruction.
void FreeMap_External();

// The address to which our extracted data is loaded at map init.
extern struct LMapFile* FileBuffer;
// The array of LMapData* that define what maps the game can load.
// There are 14 slots, not all of which are used.
extern struct LMapData* MapDataPtrs[14];

char* TryLoadMirrorFile(uint16_t MapID);
// Our new function to load mirrors from file.
void InitMirrors_External();

extern int OpenMapID;
extern char OpenMapMirrorCount;
extern struct LRuntimeMirror* OpenMapMirrors;
extern void* MirrorConstructor;
