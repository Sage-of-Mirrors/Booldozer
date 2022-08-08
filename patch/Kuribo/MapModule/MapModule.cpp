#include "ExtMapPatch.hpp"

#include <sdk/kuribo_sdk.h>

pp::DefineModule("Map Module", "Booldozer Modules", "1.0");

// Init
pp:PatchBL(0x80012738, InitExtMapData);
pp:PatchBL(0x80010DA0, InitExtMirrorData);

// Free
pp::PatchBL(0x8000BF50, FreeExtMapData);
