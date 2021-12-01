#pragma once

#include <stdint.h>

typedef struct LRuntimeMirror
{
  char mUnkBlock1[0x40];
  
  uint32_t mRoomBitfield;
  
  float mPositionX;
  float mPositionY;
  float mPositionZ;
  
  float mScaleX;
  float mScaleY;
  float mScaleZ;
  
  int32_t mRotationX;
  int32_t mRotationY;
  int32_t mRotationZ;
  
  char mUnkBlock2[0x0C];
  
  int32_t mRenderCameraVerticalOffset;
  
  char mUnkBlock3[0x158];
  
  float mUnkField_1D0;
  
  char mUnkBlock4[0x3C];
  
  uint16_t mImageBaseWidth;
  uint16_t mImageBaseHeight;
  
  char mUnkBlock5[0x24];
  
  float mRenderCameraZoom;
  
  float mRenderCameraRotationX;
  float mRenderCameraRotationY;
  float mRenderCameraRotationZ;
  
  char mUnkChar1;
  char mGBHRenderOnly;
  char mUnkChar2;
  char mUnkChar3;
} LRuntimeMirror;

typedef struct LMirrorFileDef
{
  float mPositionX;
  float mPositionY;
  float mPositionZ;
  
  float mScaleX;
  float mScaleY;
  float mScaleZ;
  
  int32_t mRotationX;
  int32_t mRotationY;
  int32_t mRotationZ;
  
  int32_t mRenderCameraVerticalOffset;
  
  float mUnkField_1D0;
  
  uint16_t mImageBaseWidth;
  uint16_t mImageBaseHeight;
  
  float mRenderCameraZoom;
  
  uint32_t mGBHRenderOnly;
} LMirrorFileDef;

typedef struct LMirrorFile
{
  uint32_t mMirrorCount;
  LMirrorFileDef* mMirrorDefinitions;
} LMirrorFile;
