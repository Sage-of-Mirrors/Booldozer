#include "DOM.hpp"
#include <fstream>
#include "GenUtil.hpp"
#include "ResUtil.hpp"
#include "Options.hpp"
#include <cstring>

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

LMapDOMNode::~LMapDOMNode(){}


bool LMapDOMNode::LoadMap(std::filesystem::path file_path)
{

	mMapArchive = Archive::Rarc::Create();

	// Make sure file path is valid
	if (!std::filesystem::exists(file_path))
	{
		printf("File \"%s\" does not exist\n", file_path.string().c_str());
		return false;
	}

	bStream::CFileStream mMapArchiveStream(file_path.string(), bStream::Endianess::Big, bStream::OpenMode::In);
	// Attempt to load archive
	if (!mMapArchive->Load(&mMapArchiveStream))
	{
		printf("Archive Load Failed\n");
		return false;
	}

	// We'll call ourselves whatever the root directory of the archive is
	mName = file_path.stem().string();

	std::filesystem::path eventPath = std::filesystem::path(OPTIONS.mRootPath) / "files" / "Event";
	if(std::filesystem::exists(eventPath))
	{
		for (auto& archive : std::filesystem::directory_iterator(eventPath))
		{

			std::shared_ptr<Archive::Rarc> eventArc = Archive::Rarc::Create();

			//Exclude cvs subdir
			if(archive.is_regular_file()){
				bStream::CFileStream eventStream(archive.path(), bStream::Endianess::Big, bStream::OpenMode::In);
				if(!eventArc->Load(&eventStream)) continue;

				//Name of the event file were loading, ex 'event48'
				std::string eventName = archive.path().stem().string();
				std::string csvName = eventName;

				csvName.replace(0, 5, "message");
				std::shared_ptr<LEventDataDOMNode> eventData =  std::make_shared<LEventDataDOMNode>(eventName);
				
				eventData->LoadEventArchive(eventArc, archive.path(), eventName, csvName);
				
				AddChild(eventData);
			}
		}
		
	}

	std::filesystem::path roomsMap = LResUtility::GetStaticMapDataPath(mName);
	if (!std::filesystem::exists(roomsMap))
	{
		DOL dol;
		dol.LoadDOLFile(std::filesystem::path(OPTIONS.mRootPath) / "sys" / "main.dol");

		std::cout << "Ripping static data for map " << mName << std::endl;
		if (!mStaticMapIO.RipStaticDataFromExecutable(dol, roomsMap, mName, "GLME01"))
		{
			std::cout << fmt::format("Failed to rip static map data to {0}", roomsMap.string()) << std::endl;
			return false;
		}
	}

	if (!ReadStaticData(roomsMap))
	{
		std::cout << fmt::format("Failed to open static data from {0}", roomsMap.string()) << std::endl;
		return false;
	}

	// Attempt to load roominfo data. Doesn't necessarily exist!
	std::shared_ptr<Archive::File> roomInfoFile = mMapArchive->GetFile("/jmp/roominfo");

	if (roomInfoFile != nullptr)
	{
		// Prep the roominfo data to be read from
		bStream::CMemoryStream roomMemStream(roomInfoFile->GetData(), roomInfoFile->GetSize(), bStream::Endianess::Big, bStream::OpenMode::In);
		JmpIOManagers[LEntityType_Rooms].Load(&roomMemStream);
	}

	// Grab the friendly room names for this map
	nlohmann::json roomNames = LResUtility::GetNameMap(fmt::format("{0}_rooms", mName));

	// Use the static room data to know how many rooms to load
	for (size_t i = 0; i < mStaticMapIO.GetRoomCount(); i++)
	{
		std::string roomName = fmt::format("room_{:02}", i);
		if (roomNames.find(roomName) != roomNames.end())
			roomName = roomNames[roomName];

		std::shared_ptr<LRoomDOMNode> newRoom = std::make_shared<LRoomDOMNode>(roomName);

		// roominfo might not have this room defined in it, so only try to read if the index is within the bounds.
		if (i < JmpIOManagers[LEntityType_Rooms].GetEntryCount())
			newRoom->Deserialize(&JmpIOManagers[LEntityType_Rooms], static_cast<uint32_t>(i));
		else
			newRoom->SetRoomNumber(static_cast<int32_t>(i));

		AddChild(newRoom);
	}

	auto rooms = GetChildrenOfType<LRoomDOMNode>(EDOMNodeType::Room);

	// Complete loading the basic map data
	LoadStaticData(rooms);

	// Load Collision

	std::shared_ptr<LMapCollisionDOMNode> collision = std::make_shared<LMapCollisionDOMNode>("Collision");

	std::shared_ptr<Archive::File> collisionFile = mMapArchive->GetFile("col.mp");

	if (collisionFile != nullptr)
	{
		bStream::CMemoryStream collisionMemStream(collisionFile->GetData(), collisionFile->GetSize(), bStream::Endianess::Big, bStream::OpenMode::In);
		collision->Load(&collisionMemStream);
		AddChild(collision);
	}


	// Now load the entity data from the map's archive.
	for (int32_t entityType = 0; entityType < LEntityType_Max; entityType++)
	{
		// Skip roominfo because we already loaded it
		if (entityType == LEntityType_Rooms)
			continue;

		std::shared_ptr<Archive::File> jmpFile = mMapArchive->GetFile(std::filesystem::path("jmp") / LEntityFileNames[entityType]);

		// This JMP file doesn't exist in the arc we loaded.
		if (jmpFile == nullptr)
			continue;

		// Some JMP files have strings that aren't 32 bytes, so this is a way to handle that.
		if (entityType == LEntityType_ItemInfoTable || entityType == LEntityType_ItemAppear || entityType == LEntityType_ItemFishing || entityType == LEntityType_TreasureTable)
			JmpIOManagers[entityType].SetStringSize(16);

		bStream::CMemoryStream fileReader = bStream::CMemoryStream(jmpFile->GetData(), jmpFile->GetSize(), bStream::Endianess::Big, bStream::OpenMode::In);
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
	if (!std::filesystem::exists(mirrorsPath))
	{
		if (mName == "map2")
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
	}
	else
	{
		bStream::CFileStream mirrorFile = bStream::CFileStream(mirrorsPath.string(), bStream::Big);
		
		uint32_t mirrorCount = mirrorFile.readUInt32();
		mirrorFile.skip(4);

		for (uint32_t i = 0; i < mirrorCount; i++)
		{
			std::shared_ptr<LMirrorDOMNode> newNode = std::make_shared<LMirrorDOMNode>("Mirror");
			newNode->Load(&mirrorFile);
			AddChild(newNode);
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
	bStream::CFileStream mapFileReader = bStream::CFileStream(filePath.string(), bStream::Endianess::Big, bStream::OpenMode::In);
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
	for (uint32_t i = 0; i < mStaticMapIO.GetDoorCount(); i++)
	{
		LStaticDoorData d;
		mStaticMapIO.GetDoorData(i, d);

		std::shared_ptr<LDoorDOMNode> doorNode = std::make_shared<LDoorDOMNode>(fmt::format("Door {0}", i));
		doorNode->Load(d);
		AddChild(doorNode);
	}

	for (uint32_t i = 0; i < rooms.size(); i++)
	{
		std::shared_ptr<LRoomDataDOMNode> roomData = std::make_shared<LRoomDataDOMNode>(fmt::format("room data {0}", i));
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

		std::shared_ptr<Archive::File> jmpFile = mMapArchive->GetFile(std::filesystem::path("jmp") / LEntityFileNames[entityType]);
		
		if(jmpFile == nullptr){
			jmpFile = Archive::File::Create(); // [v]: if this jmp file doesnt exist, make it
			jmpFile->SetName(LEntityFileNames[entityType]);
			mMapArchive->GetFolder("jmp")->AddFile(jmpFile);
		}
		
		jmpFile->SetData(memWriter.getBuffer(), newFileSize);
	}

	for (auto pathNode : GetChildrenOfType<LPathDOMNode>(EDOMNodeType::Path))
	{
		nlohmann::json jmpTemplate = LResUtility::DeserializeJSON(RES_BASE_PATH / "jmp_templates" / "path.json");

		if (jmpTemplate.find("fields") == jmpTemplate.end())
		{
			std::cout << fmt::format("Failed to read JSON at {0}", (RES_BASE_PATH / "jmp_templates" / "path.json").string());
			return false;
		}

		LJmpIO pathJmp;
		pathJmp.Load(jmpTemplate, pathNode->GetNumPoints());

		size_t pathSize = pathJmp.CalculateNewFileSize(pathNode->GetNumPoints());

		bStream::CMemoryStream pathJmpFile = bStream::CMemoryStream(pathSize, bStream::Endianess::Big, bStream::OpenMode::Out);
		pathNode->PreProcess(pathJmp, pathJmpFile);

		std::shared_ptr<Archive::File> pathFile = mMapArchive->GetFile(std::filesystem::path("path") / pathNode->GetName());

		if(pathFile == nullptr){
			pathFile = Archive::File::Create();
			pathFile->SetName(pathNode->GetName());

			mMapArchive->GetFolder("path")->AddFile(pathFile);
		}

		pathFile->SetData(pathJmpFile.getBuffer(), pathJmpFile.getSize());

	}


	mMapArchive->SaveToFile(file_path, Compression::Format::YAY0);


	mStaticMapIO.SaveMapFile(LResUtility::GetStaticMapDataPath(mName), GetSharedPtr<LMapDOMNode>(EDOMNodeType::Map));

	SaveMirrorData();

	return true;
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
			std::cout << fmt::format("Failed to read JSON at {0}", (RES_BASE_PATH / "jmp_templates" / "path.json").string());
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

	SaveMirrorData();

	return true;
}

bool LMapDOMNode::SaveMirrorData()
{
	std::filesystem::path mirrorsPath = LResUtility::GetMirrorDataPath(mName);

	std::vector<std::shared_ptr<LMirrorDOMNode>> MirrorNodes = GetChildrenOfType<LMirrorDOMNode>(EDOMNodeType::Mirror);

	if (MirrorNodes.size() == 0)
	{
		if (std::filesystem::exists(mirrorsPath))
		{
			std::filesystem::remove(mirrorsPath);
		}

		return true;
	}

	bStream::CMemoryStream memStream = bStream::CMemoryStream(8 + (MirrorNodes.size() * 0x38), bStream::Big, bStream::Out);
	memStream.writeUInt32(MirrorNodes.size());
	memStream.writeUInt32(0);

	for (int i = 0; i < MirrorNodes.size(); i++)
	{
		MirrorNodes[i]->Save(&memStream);
	}

	bStream::CFileStream fileStream = bStream::CFileStream(mirrorsPath.string(), bStream::Big, bStream::Out);
	fileStream.writeBytes(memStream.getBuffer(), memStream.getSize());

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
				newNode = std::make_shared<LFurnitureDOMNode>(fmt::format("Furniture {0}", i));
				break;
			case LEntityType_Observers:
				newNode = std::make_shared<LObserverDOMNode>(fmt::format("Observer {0}", i));
				break;
			case LEntityType_Enemies:
				newNode = std::make_shared<LEnemyDOMNode>(fmt::format("Enemy {0}", i));
				break;
			case LEntityType_Events:
				newNode = std::make_shared<LEventDOMNode>(fmt::format("Event {0}", i));
				break;
			case LEntityType_Characters:
				newNode = std::make_shared<LCharacterDOMNode>(fmt::format("Character {0}", i));
				break;
			case LEntityType_ItemInfoTable:
				newNode = std::make_shared<LItemInfoDOMNode>(fmt::format("Item Info {0}", i));
				break;
			case LEntityType_ItemAppear:
				newNode = std::make_shared<LItemAppearDOMNode>(fmt::format("Item Drop Group {0}", i));
				break;
			case LEntityType_ItemFishing:
				newNode = std::make_shared<LItemFishingDOMNode>(fmt::format("Capture Item Group {0}", i));
				break;
			case LEntityType_TreasureTable:
				newNode = std::make_shared<LTreasureTableDOMNode>(fmt::format("Treasure Table {0}", i));
				break;
			case LEntityType_Generators:
				newNode = std::make_shared<LGeneratorDOMNode>("generator");
				break;
			case LEntityType_Objects:
				newNode = std::make_shared<LObjectDOMNode>(fmt::format("Object {0}", i));
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