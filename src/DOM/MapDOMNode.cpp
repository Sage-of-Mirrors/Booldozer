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
	if (!GCResourceManager.LoadArchive(file_path.string().c_str(), &mMapArchive))
	{
		return false;
	}

	// We'll call ourselves whatever the root directory of the archive is
	mName = std::string(file_path.stem().string().c_str());

	std::filesystem::path eventPath = std::filesystem::path(OPTIONS.mRootPath) / "files" / "Event";
	if(std::filesystem::exists(eventPath)){
		for (auto& archive : std::filesystem::directory_iterator(eventPath)){
			GCarchive eventArc;
			//Exclude cvs subdir
			if(archive.is_regular_file() && GCResourceManager.LoadArchive(archive.path().generic_u8string().c_str(), &eventArc)){
				//Name of the event file were loading, ex 'event48'
				std::string eventName = archive.path().stem().string();
				std::string csvName = eventName;
				csvName.replace(0, 5, "message");
				std::cout << csvName << std::endl;
				std::shared_ptr<LEventDataDOMNode> eventData =  std::make_shared<LEventDataDOMNode>(eventName);
				for (size_t i = 0; i < eventArc.filenum; i++){
					if((eventName + ".txt").compare(eventArc.files[i].name) == 0){
						eventData->mEventScript = LGenUtility::SjisToUtf8(std::string((char*)eventArc.files[i].data));
					}
					else if((csvName + ".csv").compare(eventArc.files[i].name) == 0){
						std::stringstream csv_data((char*)eventArc.files[i].data);
						std::string curline;
						while (std::getline(csv_data, curline)){
							eventData->mEventText.push_back(LGenUtility::SjisToUtf8(curline));
						}
						
					}
				}
				
				AddChild(eventData);
			}
		}
		
	}

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

	// Attempt to load roominfo data. Doesn't necessarily exist!
	uint8_t* roominfoData = nullptr;
	size_t roomInfoSize = 0;
	for (uint32_t i = 0; i < mMapArchive.filenum; i++)
	{
		if (strcmp(mMapArchive.files[i].name, "roominfo") == 0)
		{
			roominfoData = (uint8_t*)mMapArchive.files[i].data;
			roomInfoSize = (size_t)mMapArchive.files[i].size;
			break;
		}
	}

	if (roominfoData != nullptr)
	{
		// Prep the roominfo data to be read from
		bStream::CMemoryStream roomMemStream(roominfoData, roomInfoSize, bStream::Endianess::Big, bStream::OpenMode::In);
		JmpIOManagers[LEntityType_Rooms].Load(&roomMemStream);
	}

	// Grab the friendly room names for this map
	nlohmann::json roomNames = LResUtility::GetNameMap(LGenUtility::Format(mName, "_rooms"));

	// Use the static room data to know how many rooms to load
	for (size_t i = 0; i < mStaticMapIO.GetRoomCount(); i++)
	{
		std::string roomName = LGenUtility::Format("room_", std::setfill('0'), std::setw(2), i);
		if (roomNames.find(roomName) != roomNames.end())
			roomName = roomNames[roomName];

		std::shared_ptr<LRoomDOMNode> newRoom = std::make_shared<LRoomDOMNode>(roomName);

		// roominfo might not have this room defined in it, so only try to read if the index is within the bounds.
		if (i < JmpIOManagers[LEntityType_Rooms].GetEntryCount())
			newRoom->Deserialize(&JmpIOManagers[LEntityType_Rooms], i);
		else
			newRoom->SetRoomNumber(i);

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
		for (uint32_t i = 0; i < mMapArchive.filenum; i++)
		{
			if (strcmp(mMapArchive.files[i].name, LEntityFileNames[entityType].c_str()) == 0)
			{
				fileData = (uint8_t*)mMapArchive.files[i].data;
				fileSize = mMapArchive.files[i].size;
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

	// Get the path the mirrors file should be at.
	std::filesystem::path mirrorsPath = LResUtility::GetMirrorDataPath(mName);
	// If the file doesn't exist, and we're in the main mansion (map2), try to load the JSON template.
	if (!std::filesystem::exists(mirrorsPath) && mName == "map2")
	{
		nlohmann::ordered_json mansionMirrorTemplate = LResUtility::GetMirrorTemplate("mirrors_map2");

		if (!mansionMirrorTemplate.empty())
		{
			for (auto j : mansionMirrorTemplate)
			{
				std::shared_ptr<LMirrorDOMNode> newNode = std::make_shared<LMirrorDOMNode>("Mirror");
				newNode->Load(j);
				AddChild(newNode);
			}
		}
	}

	// Shore up things like entity references now that all of the entity data has been loaded
	for (auto loadedNode : GetChildrenOfType<LEntityDOMNode>(EDOMNodeType::Entity))
		loadedNode->PostProcess();

	// Load the path files into the path nodes
	for (auto pathNode : GetChildrenOfType<LPathDOMNode>(EDOMNodeType::Path))
		pathNode->PostProcess(mMapArchive);

	for (auto doorNode : GetChildrenOfType<LDoorDOMNode>(EDOMNodeType::Door))
		doorNode->PostProcess();

	for (auto mirrorNode : GetChildrenOfType<LMirrorDOMNode>(EDOMNodeType::Mirror))
		mirrorNode->PostProcess();

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

bool LMapDOMNode::SaveMapToArchive(std::filesystem::path file_path)
{
	auto isNotBlackoutFilter = [](auto node) { return std::static_pointer_cast<LBlackoutDOMNode>(node)->IsActiveDuringBlackout() == false; };
	auto isBlackoutFilter = [](auto node) { return std::static_pointer_cast<LBlackoutDOMNode>(node)->IsActiveDuringBlackout() == true; };

	for (int32_t entityType = 0; entityType < LEntityType_Max; entityType++)
	{
		std::vector<std::shared_ptr<LEntityDOMNode>> entitiesOfType;
		
		if (entityType == LEntityType_Characters || entityType == LEntityType_Enemies || entityType == LEntityType_Observers || entityType == LEntityType_Keys)
			entitiesOfType = GetChildrenOfType<LEntityDOMNode>(LEntityDOMNode::EntityTypeToDOMNodeType((LEntityType)entityType), isNotBlackoutFilter);
		else if (entityType == LEntityType_BlackoutCharacters || entityType == LEntityType_BlackoutEnemies || entityType == LEntityType_BlackoutObservers || entityType == LEntityType_BlackoutKeys)
			entitiesOfType = GetChildrenOfType<LEntityDOMNode>(LEntityDOMNode::EntityTypeToDOMNodeType((LEntityType)entityType), isBlackoutFilter);
		else
		{
			entitiesOfType = GetChildrenOfType<LEntityDOMNode>(LEntityDOMNode::EntityTypeToDOMNodeType((LEntityType)entityType));

			if (entitiesOfType.size() == 0)
				continue;
		}

		for (auto ent : entitiesOfType)
			ent->PreProcess();

		// Calculate the size of the required buffer. Header size + number of fields * field size + number of entities * entity size
		size_t newFileSize = JmpIOManagers[entityType].CalculateNewFileSize(entitiesOfType.size());
		bStream::CMemoryStream memWriter = bStream::CMemoryStream(newFileSize, bStream::Endianess::Big, bStream::OpenMode::Out);

		JmpIOManagers[entityType].Save(entitiesOfType, memWriter);

		for (uint32_t i = 0; i < mMapArchive.filenum; i++)
		{
			if (strcmp(mMapArchive.files[i].name, LEntityFileNames[entityType].c_str()) == 0)
			{

				if (!GCResourceManager.ReplaceArchiveFileData(&mMapArchive.files[i], memWriter.getBuffer(), newFileSize))
				{
					std::cout << "Error replacing file " << mMapArchive.files[i].name << std::endl;
					return false;
				}

				break;
			}
		}
	}

	if (!GCResourceManager.SaveArchiveCompressed(file_path.string().c_str(), &mMapArchive))
	{
		std::cout << "Error saving archive " << file_path << std::endl;
		return false;
	}


	mStaticMapIO.SaveMapFile(LResUtility::GetStaticMapDataPath(mName), GetSharedPtr<LMapDOMNode>(EDOMNodeType::Map));

	

}

bool LMapDOMNode::SaveMapToFiles(std::filesystem::path folder_path)
{
	auto isNotBlackoutFilter = [](auto node) { return std::static_pointer_cast<LBlackoutDOMNode>(node)->IsActiveDuringBlackout() == false; };
	auto isBlackoutFilter = [](auto node) { return std::static_pointer_cast<LBlackoutDOMNode>(node)->IsActiveDuringBlackout() == true; };

	std::filesystem::path jmpPath = folder_path / mName / "jmp";
	if (!std::filesystem::exists(jmpPath))
		std::filesystem::create_directories(jmpPath);

	for (int32_t entityType = 0; entityType < LEntityType_Max; entityType++)
	{
		std::filesystem::path entityFilePath = jmpPath / LEntityFileNames[entityType];
		std::vector<std::shared_ptr<LEntityDOMNode>> entitiesOfType;
		
		if (entityType == LEntityType_Characters || entityType == LEntityType_Enemies || entityType == LEntityType_Observers || entityType == LEntityType_Keys)
			entitiesOfType = GetChildrenOfType<LEntityDOMNode>(LEntityDOMNode::EntityTypeToDOMNodeType((LEntityType)entityType), isNotBlackoutFilter);
		else if (entityType == LEntityType_BlackoutCharacters || entityType == LEntityType_BlackoutEnemies || entityType == LEntityType_BlackoutObservers || entityType == LEntityType_BlackoutKeys)
			entitiesOfType = GetChildrenOfType<LEntityDOMNode>(LEntityDOMNode::EntityTypeToDOMNodeType((LEntityType)entityType), isBlackoutFilter);
		else
		{
			entitiesOfType = GetChildrenOfType<LEntityDOMNode>(LEntityDOMNode::EntityTypeToDOMNodeType((LEntityType)entityType));

			if (entitiesOfType.size() == 0)
				continue;
		}

		for (auto ent : entitiesOfType)
			ent->PreProcess();

		// Calculate the size of the required buffer. Header size + number of fields * field size + number of entities * entity size
		size_t newFileSize = JmpIOManagers[entityType].CalculateNewFileSize(entitiesOfType.size());
		bStream::CMemoryStream memWriter = bStream::CMemoryStream(newFileSize, bStream::Endianess::Big, bStream::OpenMode::Out);

		JmpIOManagers[entityType].Save(entitiesOfType, memWriter);

		std::ofstream fileStream;
		fileStream.open(entityFilePath.c_str(), std::ios::binary | std::ios::out);
		fileStream.write((const char*)memWriter.getBuffer(), newFileSize);
		fileStream.close();
	}

	std::filesystem::path pathFolder = folder_path / mName / "path";
	if (!std::filesystem::exists(pathFolder))
		std::filesystem::create_directories(pathFolder);

	for (auto pathNode : GetChildrenOfType<LPathDOMNode>(EDOMNodeType::Path))
	{
		nlohmann::json jmpTemplate = LResUtility::DeserializeJSON(RES_BASE_PATH / "jmp_templates" / "path.json");

		if (jmpTemplate.find("fields") == jmpTemplate.end())
		{
			std::cout << LGenUtility::Format("Failed to read JSON at ", RES_BASE_PATH / "jmp_templates" / "path.json");
			return false;
		}

		LJmpIO pathJmp;
		pathJmp.Load(jmpTemplate, pathNode->GetNumPoints());

		size_t pathSize = pathJmp.CalculateNewFileSize(pathNode->GetNumPoints());

		bStream::CMemoryStream pathFile = bStream::CMemoryStream(pathSize, bStream::Endianess::Big, bStream::OpenMode::Out);
		pathNode->PreProcess(pathJmp, pathFile);

		std::filesystem::path pathPath = pathFolder / pathNode->GetName();
		std::ofstream fileStream;
		fileStream.open(pathPath.c_str(), std::ios::binary | std::ios::out);
		fileStream.write((const char*)pathFile.getBuffer(), pathSize);
		fileStream.close();
	}

	mStaticMapIO.SaveMapFile(LResUtility::GetStaticMapDataPath(mName), GetSharedPtr<LMapDOMNode>(EDOMNodeType::Map));

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
			case LEntityType_BlackoutCharacters:
				newNode = std::make_shared<LCharacterDOMNode>("Character", true);
				break;
			case LEntityType_BlackoutObservers:
				newNode = std::make_shared<LObserverDOMNode>("Observer", true);
				break;
			case LEntityType_BlackoutKeys:
				newNode = std::make_shared<LKeyDOMNode>("key01", true);
				break;
			case LEntityType_Paths:
				newNode = std::make_shared<LPathDOMNode>("path");
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

std::shared_ptr<LRoomDOMNode> LMapDOMNode::GetRoomByID(int32_t id)
{
	std::shared_ptr<LRoomDOMNode> room = nullptr;

	auto roomVec = GetChildrenOfType<LRoomDOMNode>(EDOMNodeType::Room);
	for (std::shared_ptr<LRoomDOMNode> r : roomVec)
	{
		auto roomData = r->GetChildrenOfType<LRoomDataDOMNode>(EDOMNodeType::RoomData)[0];

		if (roomData->GetRoomID() == id)
		{
			room = r;
			break;
		}
	}

	return room;
}

std::shared_ptr<LRoomDOMNode> LMapDOMNode::GetRoomByIndex(int32_t index)
{
	std::shared_ptr<LRoomDOMNode> room = nullptr;

	auto roomVec = GetChildrenOfType<LRoomDOMNode>(EDOMNodeType::Room);
	for (std::shared_ptr<LRoomDOMNode> r : roomVec)
	{
		auto roomData = r->GetChildrenOfType<LRoomDataDOMNode>(EDOMNodeType::RoomData)[0];

		if (roomData->GetRoomIndex() == index)
		{
			room = r;
			break;
		}
	}

	return room;
}
