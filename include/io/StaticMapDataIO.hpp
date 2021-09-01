#pragma once

#include "DOL.hpp"
#include "../lib/bStream/bstream.h"
#include "glm/glm.hpp"

#include <filesystem>
#include <vector>

constexpr size_t FILE_HEADER_SIZE = 64;
constexpr size_t ROOM_DATA_SIZE = 48;
constexpr size_t MAP_DATA_SIZE = 28;
constexpr size_t RES_STRING_SIZE = 28;
constexpr size_t DOOR_DATA_SIZE = 28;
constexpr size_t ALT_RES_DATA_SIZE = 8;

struct LStaticMapData
{
	uint8_t mRoomCount;
	uint8_t mRoomCount2;
	uint16_t mPadding;

	uint32_t mRoomResTableAddress;
	uint32_t mRoomAdjacencyListAddress;
	uint32_t mAltResDataAddress;
	uint32_t mPadding2;

	uint32_t mRoomDataAddress;
	uint32_t mDoorDataAddress;
};

struct LStaticRoomData
{
	uint8_t mCameraPositionIndex;
	uint8_t mFloor;
	uint8_t mDoorZone;
	uint8_t mRoomID;

	uint32_t mCameraBehavior;

	glm::vec<3, int> mBoundingBoxMin;
	glm::vec<3, int> mBoundingBoxMax;

	uint32_t mUnknown1;
	uint32_t mUnknown2;

	uint32_t mDoorListIndex;

	glm::vec<4, char> mDarkColor;
};

/*
enum class EDoorOrientation : uint32_t
{
	Invisible,
	Front_Facing,
	Side_Facing
};

enum class EDoorType : uint32_t
{
	Door,
	Viewport,
	Window
};

enum class EDoorModel : uint32_t
{
	Square_Mansion_Door = 1,
	Round_Topped_Mansion_Door,
	Parlor_Double_Door,
	Anteroom_Double_Door,
	Lab_Door,
	Gallery_Door,
	Nursery_Door,
	Twins_Door,
	Wooden_Door,
	Basement_Hallway_Door,
	Hearts_Double_Door,
	Clubs_Door,
	Diamonds_Door,
	Spades_Door
};
*/

struct LStaticDoorData
{
	uint8_t mOrientation;
	uint8_t mType;
	uint16_t mPadding;

	uint16_t mJmpID;

	uint8_t mModel;
	uint8_t mEntryIndex;

	glm::vec<3, int> mPosition;
	glm::vec<3, short> mViewportSize;

	uint8_t mNextEscape;
	uint8_t mCurrentEscape;
};

struct LStaticAltRoomResourceData
{
	uint8_t mRoomNumber;
	uint8_t mUnknown1;
	uint16_t mPadding;

	uint32_t mPathOffset;

	//std::string mPath;

	LStaticAltRoomResourceData() : mRoomNumber(0), mUnknown1(0), mPadding(0), mPathOffset(0) { }
};

class LStaticMapDataIO
{
	// Total size of the file, used for bounds checking.
	size_t mFileSize;

	// Stuff for all the rooms in the map.
	size_t mRoomCount;
	uint32_t mRoomDataOffset;
	uint32_t mRoomResourcePathOffset;

	// Some rooms can have "alternate" resources. Method of activating them is currently unknown.
	size_t mAltResourceCount;
	uint32_t mAltResourceDataOffset;

	// Stuff for all the doors in the map.
	size_t mDoorCount;
	uint32_t mDoorDataOffset;

	// This list connects rooms to doors. Entries are shorts and lists are terminated with -1/0xFFFF.
	size_t mDoorListCount;
	uint32_t mDoorListDataOffset;

	// This list tells the game what rooms should be loaded when you're in a specific room.
	size_t mRoomAdjacencyListCount;
	uint32_t mRoomAdjacencyListDataOffset;

	// Blob of data used to read stuff from.
	uint8_t* mData;

	void SwapStaticRoomDataEndianness(LStaticRoomData& data);
	void SwapStaticDoorDataEndianness(LStaticDoorData& data);
	void SwapStaticAltResDataEndianness(LStaticAltRoomResourceData& data);

	std::vector<LStaticRoomData> GetRoomDataFromDOL(bStream::CFileStream* stream, uint32_t count, uint32_t offset);
	std::vector<LStaticDoorData> GetDoorDataFromDOL(bStream::CFileStream* stream, uint32_t offset);
	std::vector<std::string> GetResDataFromDOL(bStream::CFileStream* stream, const DOL& dol, uint32_t count, uint32_t offset);
	std::vector<LStaticAltRoomResourceData> GetAltResDataFromDOL(bStream::CFileStream* stream, const DOL& dol, uint32_t offset);
	std::vector<std::vector<uint16_t>> GetRoomAdjDataFromDOL(bStream::CFileStream* stream, const DOL& dol, uint32_t count, uint32_t offset);
	std::vector<std::string> GetAltResPathsFromDOL(bStream::CFileStream* stream, const DOL& dol, const std::vector<LStaticAltRoomResourceData>& altRes);
	std::vector<std::vector<uint16_t>> GetRoomDoorListsFromDOL(bStream::CFileStream* stream, const DOL& dol, const std::vector<LStaticRoomData>& rooms);

	void WriteResStrings(bStream::CDynamicMemoryStream& stream, const std::vector<std::string>& resStrings);
	void WriteAdjacencyLists(bStream::CDynamicMemoryStream& stream, const std::vector<std::vector<uint16_t>>& adjacencyLists);
	void WriteAltResData(bStream::CDynamicMemoryStream& stream, std::vector<LStaticAltRoomResourceData>& altResData, const std::vector<std::string>& altResPaths);
	void WriteDoorData(bStream::CDynamicMemoryStream& stream, const std::vector<LStaticDoorData>& doors);
	void WriteRoomAndDoorListData(bStream::CDynamicMemoryStream& stream, std::vector<LStaticRoomData>& rooms, const std::vector<std::vector<uint16_t>>& doorLists);

public:
	LStaticMapDataIO();

	bool Load(bStream::CMemoryStream* stream);

	bool GetRoomData(const uint32_t& index, LStaticRoomData& data);
	bool GetRoomResourcePath(const uint32_t& index, std::string& data);
	bool GetDoorData(const uint32_t& index, LStaticDoorData& data);
	bool GetAltResourceData(const uint32_t& index, LStaticAltRoomResourceData& data);
	bool GetDoorListData(const uint32_t& starting_index, size_t& count, uint16_t*& data);

	bool Save(bStream::CFileStream& stream);

	bool SetRoomData(const uint32_t& index, LStaticRoomData data);
	bool SetRoomResourcePath(const uint32_t& index, std::string data);
	bool SetDoorData(const uint32_t& index, LStaticDoorData data);
	bool SetAltResourceData(const uint32_t& index, LStaticAltRoomResourceData data);
	bool SetDoorListData(const uint32_t& starting_index, const size_t& count, uint16_t* data);

	bool RipStaticDataFromExecutable(const DOL& dol, std::filesystem::path dest_path, std::string map, std::string region);
};
