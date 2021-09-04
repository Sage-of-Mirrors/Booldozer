#include "DOM.hpp"
#include "../lib/libgctools/include/compression.h"
#include <fstream>
#include "GenUtil.hpp"
#include "ResUtil.hpp"
#include "Options.hpp"

std::string const LEntityFileNames[LEntityType_Max] = {
	"characterinfo",
	"enemyinfo",
	"eventinfo",
	"furnitureinfo",
	"generatorinfo",
	"groupinfo",
	"itemappeartable",
	"itemfishingtable",
	"iteminfotable",
	"itemtable",
	"iyapootable",
	"keyinfo",
	"objinfo",
	"observerinfo",
	"polygoninfo",
	"railinfo",
	"roominfo",
	"soundgroupinfo",
	"soundpolygoninfo",
	"teidencharacterinfo",
	"teidenenemyinfo",
	"teidenkeyinfo",
	"teidenobserverinfo",
	"telesa",
	"treasuretable"
};

LMapDOMNode::LMapDOMNode() : Super("map")
{
	mType = EDOMNodeType::Map;

	GCerror err;
	if ((err = gcInitContext(&mArcLoaderContext)) != GC_ERROR_SUCCESS) {
		printf("Error initing arc loader context: %s\n", gcGetErrorMessage(err));
	}
}

bool LMapDOMNode::LoadMap(std::filesystem::path file_path)
{
	// Make sure file path is valid
	if (!std::filesystem::exists(file_path))
	{
		printf("File \"%s\" does not exist\n", file_path.string().c_str());
		return false;
	}

	// Attempt to load archive
	GCarchive mapArc;
	if (!LoadArchive(file_path.string().c_str(), &mapArc))
	{
		return false;
	}

	// We'll call ourselves whatever the root directory of the archive is
	mName = std::string(mapArc.dirs[0].name);

	std::filesystem::path roomsMap = LResUtility::GetStaticMapDataPath(mName);
	if (!std::filesystem::exists(roomsMap))
	{
		DOL dol;
		dol.LoadDOLFile(std::filesystem::path(OPTIONS.mRootPath) / "sys" / "main.dol");

		if (!mStaticMapIO.RipStaticDataFromExecutable(dol, roomsMap, mName, "GLME01"))
		{
			std::cout << LGenUtility::Format("Failed to rip static map data to ", roomsMap) << std::endl;
			return false;
		}
	}

	if (!ReadStaticData(roomsMap))
	{
		std::cout << LGenUtility::Format("Failed to open static data from ", roomsMap) << std::endl;
		return false;
	}

	// We're going to try to get the JMP file "roominfo." This will help us load the room room data.
	// If we can't find a roominfo file, we're going to assume this archive isn't from the Map/ directory and error out
	uint8_t* roominfoData = nullptr;
	size_t roomInfoSize = 0;
	for (uint32_t i = 0; i < mapArc.filenum; i++)
	{
		if (strcmp(mapArc.files[i].name, "roominfo") == 0)
		{
			roominfoData = (uint8_t*)mapArc.files[i].data;
			roomInfoSize = (size_t)mapArc.files[i].size;
			break;
		}
	}

	// No roominfo, no map load
	if (roominfoData == nullptr)
	{
		printf("Archive \"%s\" is not a valid map archive.\nPlease open a map from the Map folder.\n", file_path.string().c_str());
		return false;
	}

	// Grab the friendly room names for this map
	nlohmann::json roomNames = LResUtility::GetMapRoomNames(mName);

	// Prep the roominfo data to be read from
	bStream::CMemoryStream roomMemStream(roominfoData, roomInfoSize, bStream::Endianess::Big, bStream::OpenMode::In);
	JmpIOManagers[LEntityType_Rooms].Load(&roomMemStream);

	// As it turns out, roominfo might not list all of the rooms in the map.
	// Therefore, we're going to use the static data (extracted into rooms.map) as ground truth for room count.
	for (size_t i = 0; i < mStaticMapIO.GetRoomCount(); i++)
	{
		std::string roomName = LGenUtility::Format("room_", std::setfill('0'), std::setw(2), i);
		if (roomNames.find(roomName) != roomNames.end())
			roomName = roomNames[roomName];

		std::shared_ptr<LRoomDOMNode> newRoom = std::make_shared<LRoomDOMNode>(roomName);

		// roominfo might not have this room defined in it, so only try to read if the index is within the bounds.
		if (i < JmpIOManagers[LEntityType_Rooms].GetEntryCount())
			newRoom->LoadJmpInfo(i, &JmpIOManagers[LEntityType_Rooms]);

		AddChild(newRoom);
	}

	auto rooms = GetChildrenOfType<LRoomDOMNode>(EDOMNodeType::Room);

	// Complete loading the basic map data
	LoadStaticData(rooms);

	// Now load the entity data from the map's archive.
	for (int32_t entityType = 0; entityType < LEntityType_Max; entityType++)
	{
		// Skip roominfo because we already loaded it
		if (entityType == LEntityType_Rooms)
			continue;

		uint8_t* fileData = nullptr;
		size_t fileSize = 0;
		for (uint32_t i = 0; i < mapArc.filenum; i++)
		{
			if (strcmp(mapArc.files[i].name, LEntityFileNames[entityType].c_str()) == 0)
			{
				fileData = (uint8_t*)mapArc.files[i].data;
				fileSize = mapArc.files[i].size;
				break;
			}
		}

		// This JMP file doesn't exist in the arc we loaded.
		if (fileData == nullptr)
			continue;

		// Some JMP files have strings that aren't 32 bytes, so this is a way to handle that.
		if (entityType == LEntityType_ItemInfoTable || entityType == LEntityType_ItemAppear || entityType == LEntityType_ItemFishing || entityType == LEntityType_TreasureTable)
			JmpIOManagers[entityType].SetStringSize(16);

		bStream::CMemoryStream fileReader = bStream::CMemoryStream(fileData, fileSize, bStream::Endianess::Big, bStream::OpenMode::In);
		if (!JmpIOManagers[entityType].Load(&fileReader))
		{
			printf("Error loading JMP data from \"%s\"\n", LEntityFileNames[entityType].c_str());
			continue;
		}

		if (!LoadEntityNodes(&JmpIOManagers[entityType], (LEntityType)entityType))
		{
			printf("Error loading entities from \"%s\"\n", LEntityFileNames[entityType].c_str());
			continue;
		}
	}

	// Shore up things like entity references now that all of the entity data has been loaded
	std::vector<std::shared_ptr<LEntityDOMNode>> loadedNodes = GetChildrenOfType<LEntityDOMNode>(EDOMNodeType::Entity);
	for (auto loadedNode : loadedNodes)
		loadedNode->PostProcess();

	// To finish loading the map we're going to delegate grabbing room data to the rooms themselves,
	// where they'll also set up things like models for furniture
	for (std::shared_ptr<LRoomDOMNode> r : rooms)
		r->CompleteLoad();

	return true;
}

bool LMapDOMNode::ReadStaticData(std::filesystem::path filePath)
{
	bStream::CFileStream mapFileReader = bStream::CFileStream(filePath.u8string(), bStream::Endianess::Big, bStream::OpenMode::In);
	size_t mapFileSize = mapFileReader.getSize();

	uint8_t* mapBuf = new uint8_t[mapFileSize];
	mapFileReader.readBytesTo(mapBuf, mapFileSize);

	bStream::CMemoryStream mapStream = bStream::CMemoryStream(mapBuf, mapFileSize, bStream::Endianess::Big, bStream::OpenMode::In);
	bool result = mStaticMapIO.Load(&mapStream);

	delete[] mapBuf;
	return result;
}

bool LMapDOMNode::LoadStaticData(std::vector<std::shared_ptr<LRoomDOMNode>> rooms)
{
	for (size_t i = 0; i < mStaticMapIO.GetDoorCount(); i++)
	{
		LStaticDoorData d;
		mStaticMapIO.GetDoorData(i, d);

		std::shared_ptr<LDoorDOMNode> doorNode = std::make_shared<LDoorDOMNode>(LGenUtility::Format("Door ", i));
		doorNode->Load(d);
		AddChild(doorNode);
	}

	for (size_t i = 0; i < rooms.size(); i++)
	{
		std::shared_ptr<LRoomDataDOMNode> roomData = std::make_shared<LRoomDataDOMNode>(LGenUtility::Format("room data ", i));
		roomData->Load(i, mStaticMapIO, rooms, GetChildrenOfType<LDoorDOMNode>(EDOMNodeType::Door));

		rooms[i]->AddChild(roomData);
	}

	return true;
}

bool LMapDOMNode::SaveMapToFiles(std::filesystem::path folder_path)
{
	for (int32_t entityType = 0; entityType < LEntityType_Max; entityType++)
	{
		std::filesystem::path entityFilePath = folder_path / LEntityFileNames[entityType];
		auto entitiesOfType = GetChildrenOfType<LEntityDOMNode>(LEntityDOMNode::EntityTypeToDOMNodeType((LEntityType)entityType));

		if (entitiesOfType.size() == 0)
			continue;

		for (auto ent : entitiesOfType)
			ent->PreProcess();

		// Calculate the size of the required buffer. Header size + number of fields * field size + number of entities * entity size
		size_t newFileSize = JmpIOManagers[entityType].CalculateNewFileSize(entitiesOfType.size());
		bStream::CMemoryStream memWriter = bStream::CMemoryStream(newFileSize, bStream::Endianess::Big, bStream::OpenMode::Out);;

		JmpIOManagers[entityType].Save(entitiesOfType, memWriter);

		std::ofstream fileStream;
		fileStream.open(entityFilePath.c_str(), std::ios::binary | std::ios::out);
		fileStream.write((const char*)memWriter.getBuffer(), memWriter.getSize());
		fileStream.close();
	}

	return true;
}

bool LMapDOMNode::LoadEntityNodes(LJmpIO* jmp_io, LEntityType type)
{
	for (int32_t i = 0; i < jmp_io->GetEntryCount(); i++)
	{
		std::shared_ptr<LEntityDOMNode> newNode = nullptr;

		// This will be a huge switch, but it will allow us to create and populate every node type
		// that we'll load from JMP.
		switch (type)
		{
			case LEntityType_Furniture:
				newNode = std::make_shared<LFurnitureDOMNode>(LGenUtility::Format("Furniture ", i));
				break;
			case LEntityType_Observers:
				newNode = std::make_shared<LObserverDOMNode>(LGenUtility::Format("Observer ", i));
				break;
			case LEntityType_Enemies:
				newNode = std::make_shared<LEnemyDOMNode>(LGenUtility::Format("Enemy ", i));
				break;
			case LEntityType_Events:
				newNode = std::make_shared<LEventDOMNode>(LGenUtility::Format("Event ", i));
				break;
			case LEntityType_Characters:
				newNode = std::make_shared<LCharacterDOMNode>(LGenUtility::Format("Character ", i));
				break;
			case LEntityType_ItemInfoTable:
				newNode = std::make_shared<LItemInfoDOMNode>(LGenUtility::Format("Item Info ", i));
				break;
			case LEntityType_ItemAppear:
				newNode = std::make_shared<LItemAppearDOMNode>(LGenUtility::Format("Item Drop Group ", i));
				break;
			case LEntityType_ItemFishing:
				newNode = std::make_shared<LItemFishingDOMNode>(LGenUtility::Format("Capture Item Group ", i));
				break;
			case LEntityType_TreasureTable:
				newNode = std::make_shared<LTreasureTableDOMNode>(LGenUtility::Format("Treasure Table ", i));
				break;
			case LEntityType_Generators:
				newNode = std::make_shared<LGeneratorDOMNode>("generator");
				break;
			case LEntityType_Objects:
				newNode = std::make_shared<LObjectDOMNode>(LGenUtility::Format("Object ", i));
				break;
			case LEntityType_Keys:
				newNode = std::make_shared<LKeyDOMNode>("key01");
				break;
			case LEntityType_SpeedySpiritDrops:
				newNode = std::make_shared<LSpeedySpiritDropDOMNode>("iyapoo1");
				break;
			case LEntityType_Boos:
				newNode = std::make_shared<LBooDOMNode>("Boo");
				break;
			case LEntityType_BlackoutEnemies:
				newNode = std::make_shared<LEnemyDOMNode>("Enemy", true);
				break;
			default:
				break;
		}

		if (newNode != nullptr)
		{
			newNode->Deserialize(jmp_io, i);

			// If room number is -1, this entity doesn't actually belong to a specific room.
			// Examples include: events
			if (newNode->GetRoomNumber() == -1)
			{
				AddChild(newNode);
				continue;
			}

			std::shared_ptr<LRoomDOMNode> entityRoom = GetRoomByNumber(newNode->GetRoomNumber());
			if (entityRoom != nullptr)
				entityRoom->AddChild(newNode);
		}
	}

	return true;
}

bool LMapDOMNode::LoadArchive(const char* path, GCarchive* archive)
{
	GCerror err;

	FILE* f = fopen(path, "rb");
	if (f == nullptr)
	{
		printf("Error opening file \"%s\"\n", path);
		return false;
	}

	fseek(f, 0L, SEEK_END);
	GCsize size = (GCsize)ftell(f);
	rewind(f);

	void* file = malloc(size);
	if (file == nullptr)
	{
		printf("Error allocating buffer for file \"%s\"\n", path);
		return false;
	}

	fread(file, 1, size, f);
	fclose(f);

	// If the file starts with 'Yay0', it's Yay0 compressed.
	if (*((uint32_t*)file) == 0x30796159)
	{
		GCsize compressedSize = gcDecompressedSize(&mArcLoaderContext, (GCuint8*)file, 0);

		void* decompBuffer = malloc(compressedSize);
		gcYay0Decompress(&mArcLoaderContext, (GCuint8*)file, (GCuint8*)decompBuffer, compressedSize, 0);

		free(file);
		file = decompBuffer;
	}
	// Likewise, if the file starts with 'Yaz0' it's Yaz0 compressed.
	else if (*((uint32_t*)file) == 0x307A6159)
	{
		GCsize compressedSize = gcDecompressedSize(&mArcLoaderContext, (GCuint8*)file, 0);

		void* decompBuffer = malloc(compressedSize);
		gcYaz0Decompress(&mArcLoaderContext, (GCuint8*)file, (GCuint8*)decompBuffer, compressedSize, 0);

		free(file);
		file = decompBuffer;
	}

	gcInitArchive(archive, &mArcLoaderContext);
	if ((err = gcLoadArchive(archive, file, size)) != GC_ERROR_SUCCESS) {
		printf("Error Loading Archive: %s\n", gcGetErrorMessage(err));
		return false;
	}

	return true;
}

std::shared_ptr<LRoomDOMNode> LMapDOMNode::GetRoomByNumber(int32_t number)
{
	std::shared_ptr<LRoomDOMNode> room = nullptr;

	auto roomVec = GetChildrenOfType<LRoomDOMNode>(EDOMNodeType::Room);
	for (std::shared_ptr<LRoomDOMNode> r : roomVec)
	{
		if (r->GetRoomNumber() == number)
		{
			room = r;
			break;
		}
	}

	return room;
}
