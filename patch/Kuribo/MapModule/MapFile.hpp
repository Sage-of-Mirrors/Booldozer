#pragma once

#include <stdint.h>

struct LBoundingBox
{
  int32_t Min_X;
  int32_t Min_Y;
  int32_t Min_Z;
  
  int32_t Max_X;
  int32_t Max_Y;
  int32_t Max_Z;
};

// The data that makes up each room in a map.
struct LRoomData
{
  uint8_t mCameraPositionIndex;
  uint8_t mFloor;
  uint8_t mDoorZone;
  uint8_t mRoomID;

  uint32_t mCameraBehavior;
  
  LBoundingBox mRoomBounds;

  // The number of bounding boxes that override the room bounds for actor spawning.
  uint32_t mRoomBoundOverrideCount;
  LBoundingBox* mRoomBoundOverrides;

  // This is updated to point to the correct location at runtime.
  uint16_t* mDoorListRef;

  uint8_t mDarkColor_R;
  uint8_t mDarkColor_G;
  uint8_t mDarkColor_B;
  uint8_t mDarkColor_A;
};

// The data that defines alternate resources a room in a map can use;
// Used to make Guest Room upside-down.
struct LAltResourceData
{
  uint8_t mRoomNumber;
  uint8_t mUnknown1;
  uint16_t mPadding;

  // This is updated to point to the correct location at runtime.
  char* mPath;
};

// The data that makes up each door within a map.
struct LDoorData
{
  uint8_t mOrientation;
  uint8_t mType;
  uint16_t mPadding;

  uint16_t mJmpID;

  uint8_t mModel;
  uint8_t mEntryIndex;

  uint32_t mPosition_X;
  uint32_t mPosition_Y;
  uint32_t mPosition_Z;
  
  uint16_t mViewportSize_X;
  uint16_t mViewportSize_Y;
  uint16_t mViewportSize_Z;

  uint8_t mNextEscape;
  uint8_t mCurrentEscape;
};

// Layout for the header of the *.map file containing the extracted map data.
struct LMapFile
{
  // Total size of the file, used for bounds checking.
  uint32_t mFileSize;

  // Stuff for all the rooms in the map.
  uint32_t mRoomCount;
  struct LRoomData* mRoomData;
  
  // This is a char* array of size mRoomCount.
  // The pointers are updated to point to the correct locations at runtime.
  char** mRoomResourcePaths;

  // Some rooms can have "alternate" resources. Method of activating them is currently unknown.
  uint32_t mAltResourceCount;
  struct LAltResourceData* mAltResourceData;

  // Stuff for all the doors in the map.
  uint32_t mDoorCount;
  struct LDoorData* mDoorData;

  // This list connects rooms to doors. Entries are shorts and lists are terminated with -1/0xFFFF.
  uint32_t mDoorListCount;
  uint16_t* mDoorListData;

  // This list tells the game what rooms should be loaded when you're in a specific room.
  uint32_t mRoomAdjacencyListCount;
  
  // This is a uint16_t* array of size mRoomCount.
  // The pointers are updated to point to the correct locations at runtime.
  uint32_t* mRoomAdjacencyList;
};
