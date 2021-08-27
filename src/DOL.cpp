#include "DOL.hpp"
#include "GenUtil.hpp"

#include <iostream>

DOL::DOL()
{
	std::fill_n(mTextDataOffsets, DOL_SECTION_COUNT, 0);
	std::fill_n(mTextDataRAMAddresses, DOL_SECTION_COUNT, 0);

	mFileStream = nullptr;
}

DOL::~DOL()
{
	if (mFileStream)
		delete mFileStream;
}

bool DOL::LoadDOLFile(std::filesystem::path dolPath)
{
	if (dolPath.empty() || !std::filesystem::exists(dolPath))
		return false;

	// Open DOL file and ensure it's valid
	mFileStream = new bStream::CFileStream(dolPath.u8string(), bStream::Endianess::Big);
	if (!mFileStream || !mFileStream->getStream().is_open())
		return false;

	// Read in file offsets for the text and data sections
	for (uint32_t i = 0; i < DOL_SECTION_COUNT; i++)
		mTextDataOffsets[i] = mFileStream->readUInt32();

	// Read in RAM addresses for the text and data sections
	for (uint32_t i = 0; i < DOL_SECTION_COUNT; i++)
		mTextDataRAMAddresses[i] = mFileStream->readUInt32();

	return true;
}

uint32_t DOL::ConvertAddressToOffset(uint32_t address)
{
	for (uint32_t i = 0; i < DOL_SECTION_COUNT; i++)
	{
		if (mTextDataRAMAddresses[i] == 0)
			continue;

		if (mTextDataRAMAddresses[i] >= address)
		{
			uint32_t j = i - 1;
			if (j >= 0 && j < DOL_SECTION_COUNT)
			{
				size_t ramOffset = address - mTextDataRAMAddresses[j];
				return (uint32_t)(mTextDataOffsets[j] + ramOffset);
			}
		}
	}

	return 0;
}
