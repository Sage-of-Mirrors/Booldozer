#include <sdk/kuribo_sdk.h>
#include "ExtMapPatch.hpp"

pp::DefineModule("Map Module", "Booldozer Team", "1.0");

// Init
pp::PatchBL(0x80012738, ExtMapPatch::InitExtMapData);
pp::PatchBL(0x80010DA0, ExtMapPatch::InitExtMirrorData);

// Free
pp::PatchBL(0x8000BF50, ExtMapPatch::FreeExtMapData);
