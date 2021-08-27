#pragma once

#include "../lib/bStream/bstream.h"
#include <filesystem>

constexpr size_t DOL_SECTION_COUNT = 18;

class DOL
{
	uint32_t mTextDataOffsets[DOL_SECTION_COUNT];
	uint32_t mTextDataRAMAddresses[DOL_SECTION_COUNT];

	bStream::CFileStream* mFileStream;

public:
	DOL();
	~DOL();

	// Returns whether this DOL object has an open file stream.
	bool IsOpen() { return mFileStream != nullptr && mFileStream->getStream().is_open(); }

	bStream::CFileStream* GetFileStream() { return mFileStream; }

	// Attempts to load a DOL from the given file path.
	bool LoadDOLFile(std::filesystem::path dolPath);

	// Uses the DOL header data to convert the given RAM address to an offset in the DOL file.
	uint32_t ConvertAddressToOffset(uint32_t address);
};
