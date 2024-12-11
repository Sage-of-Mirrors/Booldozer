#include "DOL.hpp"
#include "GenUtil.hpp"
#include "Options.hpp"
#include "picosha2.h"
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


	// SHA256 the DOL so we can see if its clean or needs to be patched
	std::ifstream f(dolPath.string(), std::ios::binary);
	std::vector<unsigned char> s(picosha2::k_digest_size);
	picosha2::hash256(f, s.begin(), s.end());

	mChecksum = picosha2::hash256_hex_string(s);

	LGenUtility::Log << "[DOL]: SHA256 of executable is " << mChecksum << std::endl;
	if(mChecksum == "4e233ab2509e055894dbfef9cf4c5c07668b312ee2c2c44362b7d952308ce95a"){
		LGenUtility::Log << "[DOL]: Executable is clean" << std::endl;
		mPatched = false;
		OPTIONS.mIsDOLPatched = false;
	} else {
		mPatched = true;
		OPTIONS.mIsDOLPatched = true;
	}

	// Open DOL file and ensure it's valid
	mFileStream = new bStream::CFileStream(dolPath.string(), bStream::Endianess::Big);

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

uint32_t DOL::ConvertAddressToOffset(uint32_t address) const
{
	if (address == 0)
		return 0;

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
