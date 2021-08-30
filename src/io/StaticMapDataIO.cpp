#include "io/StaticMapDataIO.hpp"
#include "GenUtil.hpp"
#include "ResUtil.hpp"

#include <vector>

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
	SwapStaticRoomDataEndianness(data);

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

	return true;
}

bool LStaticMapDataIO::GetDoorData(const uint32_t& index, LStaticDoorData& data)
{
	if (index < 0 || index >= mDoorCount)
		return false;

	uint32_t offset = index * DOOR_DATA_SIZE;
	void* rawPtr = mData + mDoorDataOffset + offset;

	data = *static_cast<LStaticDoorData*>(rawPtr);
	SwapStaticDoorDataEndianness(data);

	return true;
}

bool LStaticMapDataIO::GetAltResourceData(const uint32_t& index, LStaticAltRoomResourceData& data)
{
	if (index < 0 || index >= mAltResourceCount)
		return false;

	uint32_t offset = index * ALT_RES_DATA_SIZE;
	void* rawPtr = mData + mAltResourceDataOffset + offset;

	data = *static_cast<LStaticAltRoomResourceData*>(rawPtr);
	SwapStaticAltResDataEndianness(data);

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

void LStaticMapDataIO::SwapStaticRoomDataEndianness(LStaticRoomData& data)
{
	data.mCameraBehavior = LGenUtility::SwapEndian(data.mCameraBehavior);

	data.mBoundingBoxMin.x = LGenUtility::SwapEndian(data.mBoundingBoxMin.x);
	data.mBoundingBoxMin.y = LGenUtility::SwapEndian(data.mBoundingBoxMin.y);
	data.mBoundingBoxMin.z = LGenUtility::SwapEndian(data.mBoundingBoxMin.z);

	data.mBoundingBoxMax.x = LGenUtility::SwapEndian(data.mBoundingBoxMax.x);
	data.mBoundingBoxMax.y = LGenUtility::SwapEndian(data.mBoundingBoxMax.y);
	data.mBoundingBoxMax.z = LGenUtility::SwapEndian(data.mBoundingBoxMax.z);

	data.mUnknown1 = LGenUtility::SwapEndian(data.mUnknown1);
	data.mUnknown2 = LGenUtility::SwapEndian(data.mUnknown2);

	data.mDoorListIndex = LGenUtility::SwapEndian(data.mDoorListIndex);
}

void LStaticMapDataIO::SwapStaticDoorDataEndianness(LStaticDoorData& data)
{
	data.mJmpID = LGenUtility::SwapEndian(data.mJmpID);

	data.mPosition.x = LGenUtility::SwapEndian(data.mPosition.x);
	data.mPosition.y = LGenUtility::SwapEndian(data.mPosition.y);
	data.mPosition.z = LGenUtility::SwapEndian(data.mPosition.z);

	data.mViewportSize.x = LGenUtility::SwapEndian(data.mViewportSize.x);
	data.mViewportSize.y = LGenUtility::SwapEndian(data.mViewportSize.y);
	data.mViewportSize.z = LGenUtility::SwapEndian(data.mViewportSize.z);
}

void LStaticMapDataIO::SwapStaticAltResDataEndianness(LStaticAltRoomResourceData& data)
{
	data.mPathOffset = LGenUtility::SwapEndian(data.mPathOffset);
}

bool LStaticMapDataIO::RipStaticDataFromExecutable(DOL& dol, std::filesystem::path dest_path, std::string map, std::string region)
{
	if (!dol.IsOpen() || dest_path.empty() || map.empty() || region.empty())
		return false;

	uint32_t mapDataOffset = LResUtility::GetStaticMapDataOffset(map, region);
	if (mapDataOffset == 0)
		return false;

	bStream::CFileStream* stream = dol.GetFileStream();
	stream->seek(mapDataOffset);

	uint8_t* mapDataBuffer = new uint8_t[MAP_DATA_SIZE];
	stream->readBytesTo(mapDataBuffer, MAP_DATA_SIZE);

	LStaticMapData* mapData = static_cast<LStaticMapData*>((void*)mapDataBuffer);

	mRoomCount = mapData->mRoomCount;

	// Have to swap endianness to get the correct values
	mapData->mRoomResTableAddress = LGenUtility::SwapEndian(mapData->mRoomResTableAddress);
	mapData->mRoomAdjacencyListAddress = LGenUtility::SwapEndian(mapData->mRoomAdjacencyListAddress);
	mapData->mAltResDataAddress = LGenUtility::SwapEndian(mapData->mAltResDataAddress);

	mapData->mRoomDataAddress = LGenUtility::SwapEndian(mapData->mRoomDataAddress);
	mapData->mDoorDataAddress = LGenUtility::SwapEndian(mapData->mDoorDataAddress);

	std::vector<LStaticRoomData> roomData = GetRoomDataFromDOL(stream, mapData->mRoomCount, dol.ConvertAddressToOffset(mapData->mRoomDataAddress));
	std::vector<LStaticDoorData> doorData = GetDoorDataFromDOL(stream, dol.ConvertAddressToOffset(mapData->mDoorDataAddress));
	//std::vector<std::string> roomResData = GetResDataFromDOL(stream, dol, mapData->mRoomCount, dol.ConvertAddressToOffset(mapData->mRoomResTableAddress));
	std::vector<LStaticAltRoomResourceData> altResData = GetAltResDataFromDOL(stream, dol, dol.ConvertAddressToOffset(mapData->mAltResDataAddress));

	delete mapData;
	return true;
}

std::vector<LStaticRoomData> LStaticMapDataIO::GetRoomDataFromDOL(bStream::CFileStream* stream, uint32_t count, uint32_t offset)
{
	std::vector<LStaticRoomData> rooms;

	stream->seek(offset);
	uint8_t buffer[ROOM_DATA_SIZE];

	for (uint32_t i = 0; i < count; i++)
	{
		stream->readBytesTo(buffer, ROOM_DATA_SIZE);

		LStaticRoomData newRoom = *static_cast<LStaticRoomData*>((void*)buffer);

		SwapStaticRoomDataEndianness(newRoom);
		rooms.push_back(newRoom);
	}

	return rooms;
}

std::vector<LStaticDoorData> LStaticMapDataIO::GetDoorDataFromDOL(bStream::CFileStream* stream, uint32_t offset)
{
	std::vector<LStaticDoorData> doors;

	stream->seek(offset);
	uint8_t buffer[DOOR_DATA_SIZE];

	while (true)
	{
		stream->readBytesTo(buffer, DOOR_DATA_SIZE);

		LStaticDoorData newDoor = *static_cast<LStaticDoorData*>((void*)buffer);

		// The list of doors is terminated by a null entry.
		// Model ID of 0 is invalid, so we can use it
		// to check if we've hit the end of the list.
		if (newDoor.mOrientation == 0)
			break;

		SwapStaticDoorDataEndianness(newDoor);
		doors.push_back(newDoor);
	}

	return doors;
}

std::vector<std::string> LStaticMapDataIO::GetResDataFromDOL(bStream::CFileStream* stream, DOL dol, uint32_t count, uint32_t offset)
{
	std::vector<std::string> resPaths;

	stream->seek(offset);
	uint8_t buffer[RES_STRING_SIZE];

	for (uint32_t i = 0; i < count; i++)
	{
		uint32_t resAddress = stream->readUInt32();
		
		uint32_t curOffset = stream->getStream().tellg();
		stream->seek(dol.ConvertAddressToOffset(resAddress));

		stream->readBytesTo(buffer, RES_STRING_SIZE);
		resPaths.push_back(std::string((char*)buffer));

		stream->seek(curOffset);
	}

	return resPaths;
}

std::vector<LStaticAltRoomResourceData> LStaticMapDataIO::GetAltResDataFromDOL(bStream::CFileStream* stream, DOL dol, uint32_t offset)
{
	std::vector<LStaticAltRoomResourceData> altRes;

	stream->seek(offset);
	uint8_t buffer[ALT_RES_DATA_SIZE];
	uint8_t pathBuffer[RES_STRING_SIZE];

	while (true)
	{
		stream->readBytesTo(buffer, ALT_RES_DATA_SIZE);

		LStaticAltRoomResourceData newAlt = *static_cast<LStaticAltRoomResourceData*>((void*)buffer);

		// The list appears to be terminated with an entry 
		// with room number == 255.
		if (newAlt.mRoomNumber == 255)
			break;

		SwapStaticAltResDataEndianness(newAlt);

		// Unlike normal rooms, the string for the alt res structs are embedded in it.
		// So we're going to grab it.
		uint32_t curOffset = stream->getStream().tellg();
		stream->seek(dol.ConvertAddressToOffset(newAlt.mPathOffset));

		stream->readBytesTo(pathBuffer, RES_STRING_SIZE);
		newAlt.mPath = std::string((char*)buffer);

		stream->seek(curOffset);

		altRes.push_back(newAlt);
	}

	return altRes;
}
