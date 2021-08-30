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

	mRoomAdjacencyListCount = stream->readUInt32();
	mRoomAdjacencyListDataOffset = stream->readUInt32();

	// Padding
	stream->skip(16);

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

bool LStaticMapDataIO::RipStaticDataFromExecutable(const DOL& dol, std::filesystem::path dest_path, std::string map, std::string region)
{
	if (!dol.IsOpen() || dest_path.empty() || map.empty() || region.empty())
		return false;

	uint32_t mapDataOffset = LResUtility::GetStaticMapDataOffset(map, region);
	if (mapDataOffset == 0)
		return false;

	bStream::CFileStream* stream = dol.GetFileStream();
	stream->seek(mapDataOffset);

	alignas(LStaticMapData) char mapDataBuffer[sizeof(LStaticMapData)];
	stream->readBytesTo((uint8_t*)mapDataBuffer, MAP_DATA_SIZE);

	LStaticMapData* mapData = static_cast<LStaticMapData*>((void*)mapDataBuffer);

	mRoomCount = mapData->mRoomCount;

	// Have to swap endianness to get the correct values
	mapData->mRoomResTableAddress = LGenUtility::SwapEndian(mapData->mRoomResTableAddress);
	mapData->mRoomAdjacencyListAddress = LGenUtility::SwapEndian(mapData->mRoomAdjacencyListAddress);
	mapData->mAltResDataAddress = LGenUtility::SwapEndian(mapData->mAltResDataAddress);

	mapData->mRoomDataAddress = LGenUtility::SwapEndian(mapData->mRoomDataAddress);
	mapData->mDoorDataAddress = LGenUtility::SwapEndian(mapData->mDoorDataAddress);

	std::vector<std::string> roomResData = GetResDataFromDOL(stream, dol, mapData->mRoomCount, dol.ConvertAddressToOffset(mapData->mRoomResTableAddress));
	std::vector<std::vector<uint16_t>> roomAdjacencyData = GetRoomAdjDataFromDOL(stream, dol, mapData->mRoomCount, dol.ConvertAddressToOffset(mapData->mRoomAdjacencyListAddress));
	std::vector<LStaticRoomData> roomData = GetRoomDataFromDOL(stream, mapData->mRoomCount, dol.ConvertAddressToOffset(mapData->mRoomDataAddress));
	
	std::vector<LStaticAltRoomResourceData> altResData = GetAltResDataFromDOL(stream, dol, dol.ConvertAddressToOffset(mapData->mAltResDataAddress));
	std::vector<std::string> altResPathData = GetAltResPathsFromDOL(stream, dol, altResData);

	std::vector<LStaticDoorData> doorData = GetDoorDataFromDOL(stream, dol.ConvertAddressToOffset(mapData->mDoorDataAddress));
	std::vector<std::vector<uint16_t>> doorLists = GetRoomDoorListsFromDOL(stream, dol, roomData);

	// Write the part of the file for res strings; they're in their own section
	bStream::CMemoryStream resBlob;
	WriteResStrings(resBlob, roomResData);

	// Write room adjacency lists; they're in their own section
	bStream::CMemoryStream adjBlob;
	WriteAdjacencyLists(adjBlob, roomAdjacencyData);

	// Write door data (they're in their own section)
	bStream::CMemoryStream doorBlob;
	WriteDoorData(doorBlob, doorData);

	// Write alternate resource data;
	// putting the data and the paths in the same section because the data references the paths via address.
	bStream::CMemoryStream altResBlob;
	WriteAltResData(altResBlob, altResData, altResPathData);

	// Write room and door list data;
	// In this file format they're technically two different sections, but the room entries have addresses
	// for their door lists despite the door lists having their own address in the higher-level map data.
	bStream::CMemoryStream roomBlob;
	bStream::CMemoryStream doorListBlob;
	WriteRoomAndDoorListData(roomBlob, doorListBlob, roomData, doorLists);

	mFileSize = FILE_HEADER_SIZE + resBlob.getSize() + adjBlob.getSize() + doorBlob.getSize() + altResBlob.getSize() + roomBlob.getSize() + doorListBlob.getSize();

	size_t currentOffset = FILE_HEADER_SIZE;
	size_t offsetStore = 0;
	bStream::CMemoryStream* finalStream = new bStream::CMemoryStream(mFileSize, bStream::Endianess::Big, bStream::OpenMode::Out);

	finalStream->writeUInt32(mFileSize);

	finalStream->writeUInt32(mRoomCount);
	finalStream->writeUInt32(currentOffset);

	offsetStore = finalStream->tell();
	finalStream->seek(currentOffset);
	finalStream->writeBytes((char*)roomBlob.getBuffer(), roomBlob.getSize());
	currentOffset += roomBlob.getSize();

	finalStream->seek(offsetStore);
	finalStream->writeUInt32(currentOffset);

	offsetStore = finalStream->tell();
	finalStream->seek(currentOffset);
	finalStream->writeBytes((char*)resBlob.getBuffer(), resBlob.getSize());
	currentOffset += resBlob.getSize();

	finalStream->seek(offsetStore);
	finalStream->writeUInt32(altResData.size());
	finalStream->writeUInt32(currentOffset);

	offsetStore = finalStream->tell();
	finalStream->seek(currentOffset);
	finalStream->writeBytes((char*)altResBlob.getBuffer(), altResBlob.getSize());
	currentOffset += altResBlob.getSize();

	finalStream->seek(offsetStore);
	finalStream->writeUInt32(doorData.size());
	finalStream->writeUInt32(currentOffset);

	offsetStore = finalStream->tell();
	finalStream->seek(currentOffset);
	finalStream->writeBytes((char*)doorBlob.getBuffer(), doorBlob.getSize());
	currentOffset += doorBlob.getSize();

	finalStream->seek(offsetStore);
	finalStream->writeUInt32(0);
	finalStream->writeUInt32(currentOffset);

	offsetStore = finalStream->tell();
	finalStream->seek(currentOffset);
	finalStream->writeBytes((char*)doorListBlob.getBuffer(), doorListBlob.getSize());
	currentOffset += doorListBlob.getSize();

	finalStream->seek(offsetStore);
	finalStream->writeUInt32(0);
	finalStream->writeUInt32(currentOffset);

	offsetStore = finalStream->tell();
	finalStream->seek(currentOffset);
	finalStream->writeBytes((char*)adjBlob.getBuffer(), adjBlob.getSize());
	currentOffset += adjBlob.getSize();

	std::ofstream outf = std::ofstream(dest_path);
	outf.write((const char*)finalStream->getBuffer(), finalStream->getSize());

	delete finalStream;
	return true;
}

std::vector<LStaticRoomData> LStaticMapDataIO::GetRoomDataFromDOL(bStream::CFileStream* stream, uint32_t count, uint32_t offset)
{
	std::vector<LStaticRoomData> rooms;

	if (offset == 0)
		return rooms;

	stream->seek(offset);
	alignas(LStaticRoomData) char buffer[sizeof(LStaticRoomData)];

	for (uint32_t i = 0; i < count; i++)
	{
		stream->readBytesTo((uint8_t*)buffer, sizeof(LStaticRoomData));

		LStaticRoomData newRoom = *static_cast<LStaticRoomData*>((void*)buffer);

		SwapStaticRoomDataEndianness(newRoom);
		rooms.push_back(newRoom);
	}

	return rooms;
}

std::vector<LStaticDoorData> LStaticMapDataIO::GetDoorDataFromDOL(bStream::CFileStream* stream, uint32_t offset)
{
	std::vector<LStaticDoorData> doors;

	if (offset == 0)
		return doors;

	stream->seek(offset);
	alignas(LStaticDoorData) char buffer[sizeof(LStaticDoorData)];

	while (true)
	{
		stream->readBytesTo((uint8_t*)buffer, sizeof(LStaticDoorData));

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

std::vector<std::string> LStaticMapDataIO::GetResDataFromDOL(bStream::CFileStream* stream, const DOL& dol, uint32_t count, uint32_t offset)
{
	std::vector<std::string> resPaths;

	if (offset == 0)
		return resPaths;

	stream->seek(offset);
	char buffer[RES_STRING_SIZE];

	for (uint32_t i = 0; i < count; i++)
	{
		uint32_t resAddress = stream->readUInt32();
		
		uint32_t curOffset = stream->getStream().tellg();
		stream->seek(dol.ConvertAddressToOffset(resAddress));

		stream->readBytesTo((uint8_t*)buffer, RES_STRING_SIZE);
		resPaths.push_back(std::string((char*)buffer));

		stream->seek(curOffset);
	}

	return resPaths;
}

std::vector<LStaticAltRoomResourceData> LStaticMapDataIO::GetAltResDataFromDOL(bStream::CFileStream* stream, const DOL& dol, uint32_t offset)
{
	std::vector<LStaticAltRoomResourceData> altRes;

	if (offset == 0)
		return altRes;

	stream->seek(offset);
	alignas(LStaticAltRoomResourceData) char buffer[sizeof(LStaticAltRoomResourceData)];

	while (true)
	{
		stream->readBytesTo((uint8_t*)buffer, sizeof(LStaticAltRoomResourceData));

		LStaticAltRoomResourceData newAlt = *static_cast<LStaticAltRoomResourceData*>((void*)buffer);

		// The list appears to be terminated with an entry with room number == 255.
		if (newAlt.mRoomNumber == 255)
			break;

		SwapStaticAltResDataEndianness(newAlt);

		altRes.push_back(newAlt);
	}

	return altRes;
}

std::vector<std::vector<uint16_t>> LStaticMapDataIO::GetRoomAdjDataFromDOL(bStream::CFileStream* stream, const DOL& dol, uint32_t count, uint32_t offset)
{
	std::vector<std::vector<uint16_t>> adjData;

	if (offset == 0)
		return adjData;

	stream->seek(offset);

	for (uint32_t i = 0; i < count; i++)
	{
		uint32_t adjAddress = stream->readUInt32();

		uint32_t curOffset = stream->getStream().tellg();
		stream->seek(dol.ConvertAddressToOffset(adjAddress));

		std::vector<uint16_t> curAdjData;

		while (true)
		{
			uint16_t adj = stream->readUInt16();
			if (adj == 0xFFFF)
				break;

			curAdjData.push_back(adj);
		}

		adjData.push_back(curAdjData);

		stream->seek(curOffset);
	}

	return adjData;
}

std::vector<std::string> LStaticMapDataIO::GetAltResPathsFromDOL(bStream::CFileStream* stream, const DOL& dol, const std::vector<LStaticAltRoomResourceData>& altRes)
{
	std::vector<std::string> altPaths;
	
	char buffer[RES_STRING_SIZE];

	for (auto ar : altRes)
	{
		uint32_t curOffset = stream->getStream().tellg();
		stream->seek(dol.ConvertAddressToOffset(ar.mPathOffset));

		stream->readBytesTo((uint8_t*)buffer, RES_STRING_SIZE);
		altPaths.push_back(std::string((char*)buffer));

		stream->seek(curOffset);
	}

	return altPaths;
}

std::vector<std::vector<uint16_t>> LStaticMapDataIO::GetRoomDoorListsFromDOL(bStream::CFileStream* stream, const DOL& dol, const std::vector<LStaticRoomData>& rooms)
{
	std::vector<std::vector<uint16_t>> doorListData;

	for (const LStaticRoomData& room : rooms)
	{
		uint32_t curOffset = stream->getStream().tellg();
		stream->seek(dol.ConvertAddressToOffset(room.mDoorListIndex));

		std::vector<uint16_t> curDoorList;

		while (true)
		{
			uint16_t door = stream->readUInt16();
			if (door == 0xFFFF)
				break;

			curDoorList.push_back(door);
		}

		doorListData.push_back(curDoorList);

		stream->seek(curOffset);
	}

	return doorListData;
}

void LStaticMapDataIO::WriteResStrings(bStream::CMemoryStream& stream, const std::vector<std::string>& resStrings)
{
	if (resStrings.size() == 0)
		return;

	size_t tableSize = resStrings.size() * 4;
	// Size = offset table (number of res strings * 4) + res strings themselves (fixed-length strings, 28 chars)
	size_t streamSize = tableSize + resStrings.size() * RES_STRING_SIZE;

	stream = bStream::CMemoryStream(streamSize, bStream::Endianess::Big, bStream::OpenMode::Out);

	// Write offset table
	for (uint32_t i = 0; i < resStrings.size(); i++)
	{
		stream.writeUInt32(tableSize + i * RES_STRING_SIZE);
	}

	// Now write the strings
	for (const std::string& str : resStrings)
	{
		stream.writeBytes((char*)str.c_str(), RES_STRING_SIZE);
	}
}

void LStaticMapDataIO::WriteAdjacencyLists(bStream::CMemoryStream& stream, const std::vector<std::vector<uint16_t>>& adjacencyLists)
{
	if (adjacencyLists.size() == 0)
		return;

	// Offset table that will point to these lists in the game
	size_t tableSize = adjacencyLists.size() * sizeof(uint32_t);

	// Size of the actual list data
	size_t dataSize = 0;
	// Add 1 to the size because of the extra -1 terminator uint16
	for (auto t : adjacencyLists)
		dataSize += (t.size() + 1) * sizeof(uint16_t);

	// Size = offset table (number of res strings * 4) + res strings themselves (fixed-length strings, 28 chars)
	size_t streamSize = tableSize + dataSize;
	stream = bStream::CMemoryStream(streamSize, bStream::Endianess::Big, bStream::OpenMode::Out);

	size_t runningOffset = tableSize;

	// Write offset table
	for (uint32_t i = 0; i < adjacencyLists.size(); i++)
	{
		stream.writeUInt32(runningOffset);
		runningOffset += (adjacencyLists[i].size() + 1) * sizeof(uint16_t);
	}

	// Now write the adjacency lists
	for (auto t : adjacencyLists)
	{
		for (auto u : t)
			stream.writeUInt16(u);

		// List terminator
		stream.writeUInt16(0xFFFF);
	}
}

void LStaticMapDataIO::WriteAltResData(bStream::CMemoryStream& stream, std::vector<LStaticAltRoomResourceData>& altResData, const std::vector<std::string>& altResPaths)
{
	if (altResData.size() == 0 || altResPaths.size() == 0)
		return;

	size_t dataSize = altResData.size() * sizeof(LStaticAltRoomResourceData);

	size_t streamSize = dataSize + altResPaths.size() * RES_STRING_SIZE;
	stream = bStream::CMemoryStream(streamSize, bStream::Endianess::Big, bStream::OpenMode::Out);

	// Write entries
	for (uint32_t i = 0; i < altResData.size(); i++)
	{
		altResData[i].mPathOffset = dataSize + i * RES_STRING_SIZE;
		stream.writeBytes((char*)&altResData[i], sizeof(LStaticAltRoomResourceData));
	}

	// Now write the strings
	for (std::string str : altResPaths)
	{
		stream.writeBytes(str.data(), RES_STRING_SIZE);
	}
}

void LStaticMapDataIO::WriteDoorData(bStream::CMemoryStream& stream, const std::vector<LStaticDoorData>& doors)
{
	if (doors.size() == 0)
		return;

	stream = bStream::CMemoryStream(doors.size() * sizeof(LStaticDoorData), bStream::Endianess::Big, bStream::OpenMode::Out);

	for (LStaticDoorData d : doors)
		stream.writeBytes((char*)&d, sizeof(LStaticDoorData));
}

void LStaticMapDataIO::WriteRoomAndDoorListData(bStream::CMemoryStream& roomDataStream, bStream::CMemoryStream& listDataStream, std::vector<LStaticRoomData>& rooms, const std::vector<std::vector<uint16_t>>& doorLists)
{
	if (rooms.size() == 0 || doorLists.size() == 0)
		return;

	size_t roomDataSize = rooms.size() * sizeof(LStaticRoomData);
	roomDataStream = bStream::CMemoryStream(roomDataSize, bStream::Endianess::Big, bStream::OpenMode::Out);

	size_t runningOffset = 0;

	// Write room data
	for (uint32_t i = 0; i < rooms.size(); i++)
	{
		rooms[i].mDoorListIndex = runningOffset;
		runningOffset += (doorLists[i].size() + 1) * sizeof(uint16_t);

		roomDataStream.writeBytes((char*)&rooms[i], sizeof(LStaticDoorData));
	}

	// Write door list data
	listDataStream = bStream::CMemoryStream(runningOffset, bStream::Endianess::Big, bStream::OpenMode::Out);
	for (const std::vector<uint16_t>& d : doorLists)
	{
		for (const uint16_t& s : d)
			listDataStream.writeUInt16(s);

		// 0xFFFF terminator
		listDataStream.writeUInt16(0xFFFF);
	}
}
