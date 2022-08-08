#include "ExtMapPatch.hpp"
#include "mirrors.h"
#include "util.h"

#include <stddef.h>

LMapFile* FileBuffer = NULL;

uint32_t ExtMapPatch::TryLoadMapFile(uint16_t MapID)
{
  // Generate the path to the map file.
  char FormattedPath[32];
  snprintf(FormattedPath, 32, "/Iwamoto/map%d/rooms.map", MapID);
  
  // Query the DVD to check if the map file exists; if not, early out.
  uint32_t DoesFileExist = DVDConvertPathToEntrynum(FormattedPath);
  if (DoesFileExist == -1)
    return 0;
  
  // Grab the info about the map file from the DVD.
  DVDFileInfo FileInfo;
  DVDOpen(FormattedPath, &FileInfo);
  
  // Allocate memory to hold the map file.
  uint32_t FileSize = OSRoundUp32B(FileInfo.mLength);
  FileBuffer = (LMapFile*)JKRHeap_alloc(FileSize, 32, NULL);
  
  // Read the map file from the DVD.
  DVDReadPrio(&FileInfo, (char*)FileBuffer, FileSize, NULL, 2);
  DVDClose(&FileInfo);
  
  return 1;
}

void ExtMapPatch::UpdateMapFileOffsets()
{
  // Header offsets
  FileBuffer->mRoomData = offset_to_ptr(FileBuffer, (uint32_t)FileBuffer->mRoomData);
  FileBuffer->mRoomResourcePaths = offset_to_ptr(FileBuffer, (uint32_t)FileBuffer->mRoomResourcePaths);
  FileBuffer->mAltResourceData = offset_to_ptr(FileBuffer, (uint32_t)FileBuffer->mAltResourceData);
  FileBuffer->mDoorData = offset_to_ptr(FileBuffer, (uint32_t)FileBuffer->mDoorData);
  FileBuffer->mDoorListData = offset_to_ptr(FileBuffer, (uint32_t)FileBuffer->mDoorListData);
  FileBuffer->mRoomAdjacencyList = offset_to_ptr(FileBuffer, (uint32_t)FileBuffer->mRoomAdjacencyList);
  
  // Room doors
  for (uint32_t i = 0; i < FileBuffer->mRoomCount; i++)
    FileBuffer->mRoomData[i].mDoorListRef = offset_to_ptr(FileBuffer->mDoorListData, (uint32_t)FileBuffer->mRoomData[i].mDoorListRef);
  
  // Room resources
  for (uint32_t i = 0; i < FileBuffer->mRoomCount; i++)
    FileBuffer->mRoomResourcePaths[i] = offset_to_ptr(FileBuffer->mRoomResourcePaths, (uint32_t)FileBuffer->mRoomResourcePaths[i]);
  
  // Alt room resources
  for (uint32_t i = 0; i < FileBuffer->mAltResourceCount; i++)
    FileBuffer->mAltResourceData[i].mPath = offset_to_ptr(FileBuffer->mAltResourceData, (uint32_t)FileBuffer->mAltResourceData[i].mPath);
  
  // Room adjacency list
  for (uint32_t i = 0; i < FileBuffer->mRoomAdjacencyListCount; i++)
    FileBuffer->mRoomAdjacencyList[i] = offset_to_ptr(FileBuffer->mRoomAdjacencyList, (uint32_t)FileBuffer->mRoomAdjacencyList[i]);
}

void ExtMapPatch::InitExtMapData(uint16_t MapID)
{
  FileBuffer = NULL;
  
  if (TryLoadMapFile(MapID))
  {
    UpdateMapFileOffsets();
    
    LMapData* Map = MapDataPtrs[MapID];
    
    Map->mRoomCount = FileBuffer->mRoomCount;
    Map->mRoomCount2 = FileBuffer->mRoomCount;
    
    Map->mRoomDefs = FileBuffer->mRoomData;
    Map->mRoomResPaths = FileBuffer->mRoomResourcePaths;
    Map->mAltRoomModelDefs = FileBuffer->mAltResourceData;
        
    Map->mRoomAdjacencyLists = FileBuffer->mRoomAdjacencyList;
    Map->mDoorDefs = FileBuffer->mDoorData;
  }
    
  // Continue with native map initializing.
  InitMap(MapID);
}

void ExtMapPatch::FreeExtMapData()
{
  // Free the memory where the map file was stored, if it was loaded in the first place.
  if (FileBuffer != NULL)
  {
     JKRHeap_free(FileBuffer, NULL);
     FileBuffer = NULL;     
  }
  
  // Continue with native map destruction.
  FreeMap();
}

char* ExtMapPatch::TryLoadMirrorFile(uint16_t MapID)
{
  // Generate the path to the map file.
  char FormattedPath[32];
  snprintf(FormattedPath, 32, "/Iwamoto/map%d/mirrors.bin", MapID);
  
  // Query the DVD to check if the map file exists; if not, early out.
  uint32_t DoesFileExist = DVDConvertPathToEntrynum(FormattedPath);
  if (DoesFileExist == -1)
    return NULL;
  
  // Grab the info about the map file from the DVD.
  DVDFileInfo FileInfo;
  DVDOpen(FormattedPath, &FileInfo);
  
  // Allocate memory to hold the map file.
  uint32_t FileSize = OSRoundUp32B(FileInfo.mLength);
  char* data = (char*)JKRHeap_alloc(FileSize, 32, NULL);
  
  // Read the map file from the DVD.
  DVDReadPrio(&FileInfo, data, FileSize, NULL, 2);
  DVDClose(&FileInfo);
  
  return data;
}

void ExtMapPatch::InitExtMirrorData()
{
  LMirrorFile* data = (LMirrorFile*)TryLoadMirrorFile(OpenMapID);
  if (data != NULL)
  {
    OpenMapMirrorCount = data->mMirrorCount;
    data->mMirrorDefinitions = (LMirrorFileDef*)((char*)data + 8);
    
    LRuntimeMirror* mirrorBuffer = (LRuntimeMirror*)FUN_801ce804(sizeof(LRuntimeMirror) * OpenMapMirrorCount + 8);
    OpenMapMirrors = __construct_array(test, &MirrorConstructor, 0, sizeof(LRuntimeMirror), OpenMapMirrorCount);
    
    for (int i = 0; i < OpenMapMirrorCount; i++)
    {
      FUN_800264e8(&OpenMapMirrors[i]);
      
      OpenMapMirrors[i].mPositionX = data->mMirrorDefinitions[i].mPositionX;
      OpenMapMirrors[i].mPositionY = data->mMirrorDefinitions[i].mPositionY;
      OpenMapMirrors[i].mPositionZ = data->mMirrorDefinitions[i].mPositionZ;
      
      OpenMapMirrors[i].mRoomBitfield = GetRoomFromPosition(&OpenMapMirrors[i].mPositionX, 0);
      
      OpenMapMirrors[i].mScaleX = data->mMirrorDefinitions[i].mScaleX;
      OpenMapMirrors[i].mScaleY = data->mMirrorDefinitions[i].mScaleY;
      OpenMapMirrors[i].mScaleZ = data->mMirrorDefinitions[i].mScaleZ;
      
      OpenMapMirrors[i].mRotationX = data->mMirrorDefinitions[i].mRotationX;
      OpenMapMirrors[i].mRotationY = data->mMirrorDefinitions[i].mRotationY;
      OpenMapMirrors[i].mRotationZ = data->mMirrorDefinitions[i].mRotationZ;
      
      OpenMapMirrors[i].mRenderCameraVerticalOffset = data->mMirrorDefinitions[i].mRenderCameraVerticalOffset;
      
      OpenMapMirrors[i].mRenderCameraDistance = data->mMirrorDefinitions[i].mRenderCameraDistance;
      
      OpenMapMirrors[i].mImageBaseWidth = data->mMirrorDefinitions[i].mImageBaseWidth;
      OpenMapMirrors[i].mImageBaseHeight = data->mMirrorDefinitions[i].mImageBaseHeight;
      
      OpenMapMirrors[i].mRenderCameraZoom = data->mMirrorDefinitions[i].mRenderCameraZoom;
      
      OpenMapMirrors[i].mGBHRenderOnly = data->mMirrorDefinitions[i].mGBHRenderOnly;
      
      FUN_8001054c(&OpenMapMirrors[i].mRenderCameraRotationX, &OpenMapMirrors[i].mRotationX);
    }
    
    JKRHeap_free(data, NULL);
  }
  else
    InitMirrors();
}
