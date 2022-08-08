#pragma once

#include "MapFile.hpp"
#include <stdint.h>

namespace ExtMapPatch
{
  // The data that defines a map and its rooms.
  struct LMapData
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
  };

/* === Functions === */
  // Our new function to load the room data from file.
  void InitExtMapData(uint16_t MapID);
  // Our new function to delete the file data on map destruction.
  void FreeExtMapData();
  
  // Our new function to load mirrors from file.
  void InitExtMirrorData();
  
  // Attempts to load the external map data file; returns whether it succeeded.
  bool TryLoadMapFile(uint16_t MapID);
  // Updates the offsets in the map file to pointers.
  void UpdateMapFileOffsets();

  // Attempts to load the external mirror file; returns the mirror data if it succeeded, or nullptr if not.
  char* TryLoadMirrorFile(uint16_t MapID);

/* === Custom variables === */
  // The address to which our extracted data is loaded at map init.
  extern LMapFile* FileBuffer;

/* === Native variables === */
  // The array of LMapData* that define what maps the game can load.
  // There are 14 slots, not all of which are used.
  extern LMapData* MapDataPtrs[14];

  // The ID of the currently open map.
  extern int OpenMapID;
  // The number of mirrors currently in memory.
  extern char OpenMapMirrorCount;
  // The starting address of the array of existing mirrors.
  extern LRuntimeMirror* OpenMapMirrors;
  // Constructor for the LRuntimeMirror struct.
  extern void* MirrorConstructor; 
}