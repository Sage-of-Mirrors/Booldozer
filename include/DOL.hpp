#pragma once

#include "bstream.h"
#include <filesystem>

constexpr size_t DOL_SECTION_COUNT = 18;

class DOL
{
	bool mPatched { false };
	std::string mChecksum { "" };
	uint32_t mTextDataOffsets[DOL_SECTION_COUNT];
	uint32_t mTextDataRAMAddresses[DOL_SECTION_COUNT];

	bStream::CFileStream* mFileStream;

public:
	DOL();
	~DOL();

	// Returns whether this DOL object has an open file stream.
	bool IsOpen() const { return mFileStream != nullptr && mFileStream->getStream().is_open(); }

	bStream::CFileStream* GetFileStream() const { return mFileStream; }

	// Attempts to load a DOL from the given file path.
	bool LoadDOLFile(std::filesystem::path dolPath);

	// Uses the DOL header data to convert the given RAM address to an offset in the DOL file.
	uint32_t ConvertAddressToOffset(uint32_t address) const;
};
