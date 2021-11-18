#include "io/StaticMapDataIO.hpp"
#include "DOM/MapDOMNode.hpp"
#include "DOM/RoomDataDOMNode.hpp"
#include "DOM/DoorDOMNode.hpp"
#include "DOM/RoomDOMNode.hpp"
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

bool LStaticMapDataIO::GetRoomData(const uint32_t& index, LStaticRoomData& data) const
{
	if (index < 0 || index >= mRoomCount)
		return false;

	for (size_t i = 0; i < mRoomCount; i++)
	{
		uint32_t offset = i * ROOM_DATA_SIZE;
		void* rawPtr = mData + mRoomDataOffset + offset - FILE_HEADER_SIZE;

		LStaticRoomData d = *static_cast<LStaticRoomData*>(rawPtr);

		// Swap endianness of fields larger than a byte
		SwapStaticRoomDataEndianness(d);

		if (d.mRoomID == index)
		{
			data = d;
			break;
		}
	}

	return true;
}

bool LStaticMapDataIO::GetRoomResourcePath(const uint32_t& index, std::string& data) const
{
 	if (index < 0 || index >= mRoomCount)
		return false;

	uint32_t offset = mRoomResourcePathOffset + (index * sizeof(uint32_t)) - FILE_HEADER_SIZE;
	void* rawOffsetPtr = mData + offset;

	uint32_t offsetToString = LGenUtility::SwapEndian(*static_cast<uint32_t*>(rawOffsetPtr));

	char strBuffer[RES_STRING_SIZE + 1];
	memcpy(strBuffer, mData + mRoomResourcePathOffset + offsetToString - FILE_HEADER_SIZE, RES_STRING_SIZE);
	strBuffer[RES_STRING_SIZE] = 0;

	data = std::string(strBuffer);

	return true;
}

bool LStaticMapDataIO::GetDoorData(const uint32_t& index, LStaticDoorData& data) const
{
	if (index < 0 || index >= mDoorCount)
		return false;

	uint32_t offset = index * DOOR_DATA_SIZE;
	void* rawPtr = mData + mDoorDataOffset + offset - FILE_HEADER_SIZE;

	data = *static_cast<LStaticDoorData*>(rawPtr);
	SwapStaticDoorDataEndianness(data);

	return true;
}

bool LStaticMapDataIO::GetAltResourceData(const uint32_t& index, LStaticAltRoomResourceData& data) const
{
	if (index < 0 || index >= mAltResourceCount)
		return false;

	uint32_t offset = index * ALT_RES_DATA_SIZE;
	void* rawPtr = mData + mAltResourceDataOffset + offset - FILE_HEADER_SIZE;

	data = *static_cast<LStaticAltRoomResourceData*>(rawPtr);
	SwapStaticAltResDataEndianness(data);

	return true;
}

bool LStaticMapDataIO::GetDoorListData(const uint32_t& starting_offset, std::vector<uint16_t>& data) const
{
	data = std::vector<uint16_t>();

	size_t off = starting_offset;

	while(true)
	{
		void* rawPtr = mData + mDoorListDataOffset + off - FILE_HEADER_SIZE;

		uint16_t temp = *static_cast<uint16_t*>(rawPtr);
		if (temp == 0xFFFF)
			break;

		data.push_back(LGenUtility::SwapEndian(temp));

		off += 2;
	}

	return true;
}

bool LStaticMapDataIO::GetAdjacentRoomListData(const uint32_t& index, std::vector<uint16_t>& data) const
{
	if (index < 0 || index >= mRoomAdjacencyListCount)
		return false;

	uint32_t offset = mRoomAdjacencyListDataOffset + (index * sizeof(uint32_t)) - FILE_HEADER_SIZE;
	void* rawOffsetPtr = mData + offset;

	uint32_t offsetToList = LGenUtility::SwapEndian(*static_cast<uint32_t*>(rawOffsetPtr));

	while (true)
	{
		void* rawPtr = mData + mRoomAdjacencyListDataOffset + offsetToList - FILE_HEADER_SIZE;

		uint16_t temp = *static_cast<uint16_t*>(rawPtr);
		if (temp == 0xFFFF)
			break;

		data.push_back(LGenUtility::SwapEndian(temp));

		offsetToList += 2;
	}
}

void LStaticMapDataIO::SwapStaticRoomDataEndianness(LStaticRoomData& data) const
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

void LStaticMapDataIO::SwapStaticDoorDataEndianness(LStaticDoorData& data) const
{
	data.mJmpID = LGenUtility::SwapEndian(data.mJmpID);

	data.mPosition.x = LGenUtility::SwapEndian(data.mPosition.x);
	data.mPosition.y = LGenUtility::SwapEndian(data.mPosition.y);
	data.mPosition.z = LGenUtility::SwapEndian(data.mPosition.z);

	data.mViewportSize.x = LGenUtility::SwapEndian(data.mViewportSize.x);
	data.mViewportSize.y = LGenUtility::SwapEndian(data.mViewportSize.y);
	data.mViewportSize.z = LGenUtility::SwapEndian(data.mViewportSize.z);
}

void LStaticMapDataIO::SwapStaticAltResDataEndianness(LStaticAltRoomResourceData& data) const
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

	bStream::CMemoryStream memStream = bStream::CMemoryStream(256, bStream::Endianess::Big, bStream::OpenMode::Out);

	// Room data
	WriteRoomAndDoorListData(memStream, roomData, doorLists);
	// Res data
	WriteResStrings(memStream, roomResData);
	// Alt res data
	WriteAltResData(memStream, altResData, altResPathData);
	// Doors
	WriteDoorData(memStream, doorData);
	// Adjacency data
	WriteAdjacencyLists(memStream, roomAdjacencyData);

	mFileSize = memStream.tell() + FILE_HEADER_SIZE;

	if (mData)
		delete[] mData;

	mData = memStream.getBuffer();

	bStream::CFileStream f = bStream::CFileStream(dest_path.u8string(), bStream::Endianess::Big, bStream::OpenMode::Out);
	Save(f);

	return true;
}

bool LStaticMapDataIO::SaveMapFile(std::filesystem::path dest_path, std::shared_ptr<LMapDOMNode> map)
{
	if (dest_path.empty() || map == nullptr)
		return false;

	std::vector<std::string> roomResData;
	std::vector<std::vector<uint16_t>> roomAdjacencyData;
	std::vector<std::vector<uint16_t>> doorLists;
	std::vector<LStaticAltRoomResourceData> altResData;
	std::vector<std::string> altResPathData;
	std::vector<LStaticRoomData> roomData = GetRoomDataFromMap(map, roomResData, roomAdjacencyData, doorLists, altResData, altResPathData);

	std::vector<LStaticDoorData> doorData = GetDoorDataFromMap(map);

	bStream::CMemoryStream memStream = bStream::CMemoryStream(256, bStream::Endianess::Big, bStream::OpenMode::Out);

	// Room data
	WriteRoomAndDoorListData(memStream, roomData, doorLists);
	// Res data
	WriteResStrings(memStream, roomResData);
	// Alt res data
	WriteAltResData(memStream, altResData, altResPathData);
	// Doors
	WriteDoorData(memStream, doorData);
	// Adjacency data
	WriteAdjacencyLists(memStream, roomAdjacencyData);

	mFileSize = memStream.tell() + FILE_HEADER_SIZE;

	if (mData)
		delete[] mData;

	mData = memStream.getBuffer();

	bStream::CFileStream f = bStream::CFileStream(dest_path.u8string(), bStream::Endianess::Big, bStream::OpenMode::Out);
	Save(f);

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

std::vector<LStaticRoomData> LStaticMapDataIO::GetRoomDataFromMap(std::shared_ptr<LMapDOMNode> map, std::vector<std::string>& pathData,
	std::vector<std::vector<uint16_t>>& adjacentRoomData, std::vector<std::vector<uint16_t>>& doorListData, std::vector<LStaticAltRoomResourceData>& altResData,
	std::vector<std::string>& altResPathData)
{
	std::vector<LStaticRoomData> roomData;

	auto doors = map->GetChildrenOfType<LDoorDOMNode>(EDOMNodeType::Door);
	auto roomDatas = map->GetChildrenOfType<LRoomDataDOMNode>(EDOMNodeType::RoomData);
	std::sort(roomDatas.begin(), roomDatas.end(),
		[](const std::shared_ptr<LRoomDataDOMNode>& left, const std::shared_ptr<LRoomDataDOMNode>& right) -> bool { return right->GetRoomIndex() > left->GetRoomIndex(); });

	for (auto rd : roomDatas)
	{
		LStaticRoomData staticRoom;
		rd->Save(staticRoom);
		roomData.push_back(staticRoom);

		pathData.push_back(rd->GetResourcePath());

		// Adjacent rooms
		std::vector<uint16_t> currentAdjacents;

		auto adjacentRooms = rd->GetAdjacencyList();
		for (auto adj : adjacentRooms)
		{
			if (auto adjLocked = adj.lock())
				currentAdjacents.push_back(adjLocked->GetRoomID());
			else
				currentAdjacents.push_back(-1);
		}

		adjacentRoomData.push_back(currentAdjacents);

		// Doors
		std::vector<uint16_t> currentDoors;

		std::weak_ptr<LRoomDOMNode> room = rd->GetParentOfType<LRoomDOMNode>(EDOMNodeType::Room);
		if (auto roomLocked = room.lock())
		{
			for (auto door : doors)
			{
				if (door->HasRoomReference(roomLocked))
					currentDoors.push_back(LGenUtility::VectorIndexOf(doors, door));
			}

			doorListData.push_back(currentDoors);

			LAlternateResource altResSrc = roomLocked->GetAlternateResource();
			if (!altResSrc.mAltResourceName.empty())
			{
				LStaticAltRoomResourceData altRes;
				altRes.mRoomNumber = altResSrc.mRoomNumber;
				altRes.mUnknown1 = altResSrc.mUnknown1;

				altResData.push_back(altRes);
				altResPathData.push_back(altResSrc.mAltResourceName);
			}
		}
	}

	return roomData;
}

std::vector<LStaticDoorData> LStaticMapDataIO::GetDoorDataFromMap(std::shared_ptr<LMapDOMNode> map)
{
	std::vector<LStaticDoorData> doorData;

	auto doors = map->GetChildrenOfType<LDoorDOMNode>(EDOMNodeType::Door);
	for (auto d : doors)
	{
		LStaticDoorData staticDoor;
		d->PreProcess();
		d->Save(staticDoor);
		doorData.push_back(staticDoor);
	}

	return doorData;
}

void LStaticMapDataIO::WriteResStrings(bStream::CMemoryStream& stream, const std::vector<std::string>& resStrings)
{
	if (resStrings.size() == 0)
	{
		mRoomResourcePathOffset = 0;

		return;
	}

	mRoomResourcePathOffset = stream.tell() + FILE_HEADER_SIZE;

	size_t tableSize = LGenUtility::PadToBoundary(resStrings.size() * sizeof(uint32_t), 16);

	// Write offset table
	for (uint32_t i = 0; i < resStrings.size(); i++)
	{
		stream.writeUInt32(tableSize + i * RES_STRING_SIZE);
	}

	size_t paddedSize = LGenUtility::PadToBoundary(stream.tell(), 16);
	stream.Reserve(paddedSize);
	stream.seek(paddedSize);

	// Now write the strings
	for (const std::string& str : resStrings)
	{
		stream.writeBytes((char*)str.c_str(), RES_STRING_SIZE);
	}

	paddedSize = LGenUtility::PadToBoundary(stream.tell(), 16);
	stream.Reserve(paddedSize);
	stream.seek(paddedSize);
}

void LStaticMapDataIO::WriteAdjacencyLists(bStream::CMemoryStream& stream, const std::vector<std::vector<uint16_t>>& adjacencyLists)
{
	if (adjacencyLists.size() == 0)
	{
		mRoomAdjacencyListCount = 0;
		mRoomAdjacencyListDataOffset = 0;

		return;
	}

	mRoomAdjacencyListCount = adjacencyLists.size();
	mRoomAdjacencyListDataOffset = stream.tell() + FILE_HEADER_SIZE;

	// Offset table that will point to these lists in the game
	size_t tableSize = LGenUtility::PadToBoundary(adjacencyLists.size() * sizeof(uint32_t), 16);
	size_t runningOffset = tableSize;

	// Write offset table
	for (uint32_t i = 0; i < adjacencyLists.size(); i++)
	{
		stream.writeUInt32(runningOffset);
		runningOffset += (adjacencyLists[i].size() + 1) * sizeof(uint16_t);
	}

	size_t paddedSize = LGenUtility::PadToBoundary(stream.tell(), 16);
	stream.Reserve(paddedSize);
	stream.seek(paddedSize);

	// Now write the adjacency lists
	for (auto t : adjacencyLists)
	{
		for (auto u : t)
			stream.writeUInt16(u);

		// List terminator
		stream.writeUInt16(0xFFFF);
	}

	paddedSize = LGenUtility::PadToBoundary(stream.tell(), 16);
	stream.Reserve(paddedSize);
	stream.seek(paddedSize);
}

void LStaticMapDataIO::WriteAltResData(bStream::CMemoryStream& stream, std::vector<LStaticAltRoomResourceData>& altResData, const std::vector<std::string>& altResPaths)
{
	if (altResData.size() == 0 || altResPaths.size() == 0)
	{
		mAltResourceCount = 0;
		mAltResourceDataOffset = 0;

		return;
	}

	mAltResourceCount = altResData.size();
	mAltResourceDataOffset = stream.tell() + FILE_HEADER_SIZE;

	size_t stringTableOffset = altResData.size() * sizeof(LStaticAltRoomResourceData);

	// Write entries
	for (uint32_t i = 0; i < altResData.size(); i++)
	{
		altResData[i].mPathOffset = stringTableOffset + (i * RES_STRING_SIZE);
		SwapStaticAltResDataEndianness(altResData[i]);

		stream.writeBytes((char*)&altResData[i], sizeof(LStaticAltRoomResourceData));
	}

	// Now write the strings
	for (std::string str : altResPaths)
		stream.writeBytes(str.data(), RES_STRING_SIZE);

	size_t paddedSize = LGenUtility::PadToBoundary(stream.tell(), 16);
	stream.Reserve(paddedSize);
	stream.seek(paddedSize);
}

void LStaticMapDataIO::WriteDoorData(bStream::CMemoryStream& stream, const std::vector<LStaticDoorData>& doors)
{
	if (doors.size() == 0)
	{
		mDoorCount = 0;
		mDoorDataOffset = 0;

		return;
	}

	mDoorCount = doors.size();
	mDoorDataOffset = stream.tell() + FILE_HEADER_SIZE;

	for (LStaticDoorData d : doors)
	{
		SwapStaticDoorDataEndianness(d);
		stream.writeBytes((char*)&d, sizeof(LStaticDoorData));
	}

	// There's an empty door entry to mark the end of the list
	for (uint32_t i = 0; i < sizeof(LStaticDoorData); i++)
		stream.writeInt8(0);

	size_t paddedSize = LGenUtility::PadToBoundary(stream.tell(), 16);
	stream.Reserve(paddedSize);
	stream.seek(paddedSize);
}

void LStaticMapDataIO::WriteRoomAndDoorListData(bStream::CMemoryStream& stream, std::vector<LStaticRoomData>& rooms, const std::vector<std::vector<uint16_t>>& doorLists)
{
	if (rooms.size() == 0 || doorLists.size() == 0)
		return;

	mRoomCount = rooms.size();
	mRoomDataOffset = stream.tell() + FILE_HEADER_SIZE;

	bStream::CMemoryStream doorListStream = bStream::CMemoryStream(256, bStream::Endianess::Big, bStream::OpenMode::Out);

	size_t doorListStartingOffset = rooms.size() * sizeof(LStaticRoomData);
	size_t runningOffset = 0;

	// Write room data
	for (uint32_t i = 0; i < rooms.size(); i++)
	{
		rooms[i].mDoorListIndex = runningOffset;
		runningOffset += (doorLists[i].size() + 1) * sizeof(uint16_t);

		SwapStaticRoomDataEndianness(rooms[i]);

		stream.writeBytes((char*)&rooms[i], sizeof(LStaticRoomData));
	}

	size_t paddedSize = LGenUtility::PadToBoundary(stream.tell(), 16);
	stream.Reserve(paddedSize);
	stream.seek(paddedSize);

	mDoorListDataOffset = stream.tell() + FILE_HEADER_SIZE;

	// Write door list data
	for (const std::vector<uint16_t>& d : doorLists)
	{
		for (const uint16_t& s : d)
			stream.writeUInt16(s);

		// 0xFFFF terminator
		stream.writeUInt16(0xFFFF);
	}

	paddedSize = LGenUtility::PadToBoundary(stream.tell(), 16);
	stream.Reserve(paddedSize);
	stream.seek(paddedSize);
}

bool LStaticMapDataIO::Save(bStream::CFileStream& stream)
{
	stream.writeUInt32(mFileSize);
	
	stream.writeUInt32(mRoomCount);
	stream.writeUInt32(mRoomDataOffset);
	stream.writeUInt32(mRoomResourcePathOffset);

	stream.writeUInt32(mAltResourceCount);
	stream.writeUInt32(mAltResourceDataOffset);

	stream.writeUInt32(mDoorCount);
	stream.writeUInt32(mDoorDataOffset);

	stream.writeUInt32(mDoorListCount);
	stream.writeUInt32(mDoorListDataOffset);

	stream.writeUInt32(mRoomAdjacencyListCount);
	stream.writeUInt32(mRoomAdjacencyListDataOffset);

	size_t headerDiff = FILE_HEADER_SIZE - stream.getSize();
	for (size_t i = 0; i < headerDiff; i++)
		stream.writeInt8(0);
	
	stream.writeBytes((char*)mData, mFileSize - FILE_HEADER_SIZE);

	return true;
}
