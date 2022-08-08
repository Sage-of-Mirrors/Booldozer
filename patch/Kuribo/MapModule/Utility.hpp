#pragma once

#define OSRoundUp32B(n) ((unsigned int) (((unsigned int)(n) + 31) & ~31))

#include <stdint.h>

// This has a lot of components and isn't necessary for our use case,
// so we're dummying it out with a char array of appropriate size.
struct DVDCommandBlock
{
  uint8_t mDummy[0x30];
};

// Represents the data used during DVD IO operations.
struct DVDFileInfo
{
  DVDCommandBlock mCommandBlock;
  uint32_t mStartAddress;
  uint32_t mLength;
  void* mCallback;
};

// Utility function for calculating pointers from a base + an offset.
void* offset_to_ptr(void* base, uint32_t ofs) {
  if (ofs == 0)
    return base;

  return (uint8_t *)base + ofs;
}
