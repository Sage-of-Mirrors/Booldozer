#include "io/StaticMapDataIO.hpp"
#include "GenUtil.hpp"

LStaticMapDataIO::LStaticMapDataIO() :
	mFileSize(0), mRoomCount(0), mRoomDataOffset(0), mRoomResourcePathOffset(0), mAltResourceCount(0),
	mAltResourceDataOffset(0), mDoorCount(0), mDoorDataOffset(0), mDoorListCount(0), mDoorListDataOffset(0), mData(nullptr)
{

}

bool LStaticMapDataIO::Load(bStream::CMemoryStream* stream)
{
	if (!stream)
		return false;

	mFileSize = stream->readUInt32();
	
	mRoomCount = stream->readUInt32();
	mRoomDataOffset = stream->readUInt32();
	mRoomResourcePathOffset = stream->readUInt32();

	mAltResourceCount = stream->readUInt32();
	mAltResourceDataOffset = stream->readUInt32();

	mDoorCount = stream->readUInt32();
	mDoorDataOffset = stream->readUInt32();

	mDoorListCount = stream->readUInt32();
	mDoorListDataOffset = stream->readUInt32();

	// Padding
	stream->skip(4);

	mData = new uint8_t[mFileSize - FILE_HEADER_SIZE];
	memcpy(mData, stream->getBuffer() + FILE_HEADER_SIZE, mFileSize - FILE_HEADER_SIZE);

	return true;
}

bool LStaticMapDataIO::GetRoomData(const uint32_t& index, LStaticRoomData& data)
{
	if (index < 0 || index >= mRoomCount)
		return false;

	uint32_t offset = index * ROOM_DATA_SIZE;
	void* rawPtr = mData + mRoomDataOffset + offset;

	data = *static_cast<LStaticRoomData*>(rawPtr);

	// Swap endianness of fields larger than a byte
	data.mCameraBehavior = LGenUtility::SwapEndian(data.mCameraBehavior);

	data.mBoundingBoxMin.x = LGenUtility::SwapEndian(data.mBoundingBoxMin.x);
	data.mBoundingBoxMin.y = LGenUtility::SwapEndian(data.mBoundingBoxMin.y);
	data.mBoundingBoxMin.z = LGenUtility::SwapEndian(data.mBoundingBoxMin.z);

	data.mBoundingBoxMax.x = LGenUtility::SwapEndian(data.mBoundingBoxMax.x);
	data.mBoundingBoxMax.y = LGenUtility::SwapEndian(data.mBoundingBoxMax.y);
	data.mBoundingBoxMax.z = LGenUtility::SwapEndian(data.mBoundingBoxMax.z);

	data.mDoorListIndex = LGenUtility::SwapEndian(data.mDoorListIndex);

	return true;
}

bool LStaticMapDataIO::GetRoomResourcePath(const uint32_t& index, std::string& data)
{
	if (index < 0 || index >= mRoomCount)
		return false;

	uint32_t offset = mRoomResourcePathOffset + (index * RES_STRING_SIZE);

	char strBuffer[RES_STRING_SIZE + 1];
	memcpy(strBuffer, mData + offset, RES_STRING_SIZE);
	strBuffer[RES_STRING_SIZE] = 0;

	data = std::string(strBuffer);
}

bool LStaticMapDataIO::GetDoorData(const uint32_t& index, LStaticDoorData& data)
{
	if (index < 0 || index >= mDoorCount)
		return false;

	uint32_t offset = index * DOOR_DATA_SIZE;
	void* rawPtr = mData + mDoorDataOffset + offset;

	data = *static_cast<LStaticDoorData*>(rawPtr);

	// Swap endianness of fields larger than a byte
	data.mJmpID = LGenUtility::SwapEndian(data.mJmpID);

	data.mPosition.x = LGenUtility::SwapEndian(data.mPosition.x);
	data.mPosition.y = LGenUtility::SwapEndian(data.mPosition.y);
	data.mPosition.z = LGenUtility::SwapEndian(data.mPosition.z);

	data.mViewportSize.x = LGenUtility::SwapEndian(data.mViewportSize.x);
	data.mViewportSize.y = LGenUtility::SwapEndian(data.mViewportSize.y);
	data.mViewportSize.z = LGenUtility::SwapEndian(data.mViewportSize.z);

	return true;
}

bool LStaticMapDataIO::GetAltResourceData(const uint32_t& index, LStaticAltRoomResourceData& data)
{
	if (index < 0 || index >= mAltResourceCount)
		return false;

	uint32_t offset = index * ALT_RES_DATA_SIZE;
	void* rawPtr = mData + mAltResourceDataOffset + offset;

	data = *static_cast<LStaticAltRoomResourceData*>(rawPtr);

	// Swap endianness of fields larger than a byte
	data.mPathOffset = LGenUtility::SwapEndian(data.mPathOffset);

	return true;
}

bool LStaticMapDataIO::GetDoorListData(const uint32_t& starting_index, size_t& count, uint16_t*& data)
{
	if (starting_index < 0 || starting_index + count > mDoorListCount)
		return false;

	uint32_t offset = starting_index * 2;
	data = new uint16_t[count];

	for (uint32_t i = 0; i < count; i++)
	{
		void* rawPtr = mData + mDoorListDataOffset + offset;

		uint16_t temp = *static_cast<uint16_t*>(rawPtr);
		data[i] = LGenUtility::SwapEndian(temp);

		offset += 2;
	}

	return true;
}
