#include "DOM.hpp"
#include "../lib/libgctools/include/compression.h"

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
	GCarchive mapArc;
	if (!LoadArchive(file_path.string().c_str(), &mapArc))
	{
		return false;
	}

	// We're going to try to get the JMP file "roominfo." This will help us load the room archives from Iwamoto/<our map>.
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

	// We'll call ourselves whatever the root directory of the archive is
	mName = std::string(mapArc.dirs[0].name);

	// Load the room nodes using roominfo
	bStream::CMemoryStream roomMemStream(roominfoData, roomInfoSize, bStream::Endianess::Big, bStream::OpenMode::In);
	JmpIOManagers[LEntityType_Rooms].Load(&roomMemStream);

	for (int32_t i = 0; i < JmpIOManagers[LEntityType_Rooms].GetEntryCount(); i++)
	{
		char roomName[16];
		snprintf(roomName, 16, "room_%02d", i);
		std::shared_ptr<LRoomDOMNode> newRoom = std::shared_ptr<LRoomDOMNode>(new LRoomDOMNode(std::string(roomName)));

		newRoom->LoadJmpInfo(i, &JmpIOManagers[LEntityType_Rooms]);
		Children.push_back(static_cast<std::shared_ptr<LDOMNodeBase>>(newRoom));
	}

	// Now load the other data from the archive.
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

	// To finish loading the map we're going to delegate grabbing room data to the rooms themselves,
	// where they'll also set up things like models for furniture
	auto rooms = GetChildrenOfType<LRoomDOMNode>(EDOMNodeType::Room);
	for (std::shared_ptr<LRoomDOMNode> r : rooms)
	{
		std::filesystem::path parentDir = file_path.parent_path().parent_path();
		std::filesystem::path roomArcPath = std::filesystem::path(parentDir / "Iwamoto" / mName / r->GetName() );
		roomArcPath.replace_extension(".arc");

		GCarchive roomArc;
		if (!LoadArchive(roomArcPath.string().c_str(), &roomArc))
		{
			continue;
		}

		r->CompleteLoad(&roomArc);
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
				newNode = std::shared_ptr<LFurnitureDOMNode>(new LFurnitureDOMNode("furniture_" + i));
				break;
			case LEntityType_Observers:
				newNode = std::shared_ptr<LObserverDOMNode>(new LObserverDOMNode("observer_" + i));
				break;
			case LEntityType_Enemies:
				newNode = std::shared_ptr<LEnemyDOMNode>(new LEnemyDOMNode("enemy_" + i));
				break;
			case LEntityType_Events:
				newNode = std::shared_ptr<LEventDOMNode>(new LEventDOMNode("event_" + i));
				break;
			case LEntityType_Characters:
				newNode = std::shared_ptr<LCharacterDOMNode>(new LCharacterDOMNode("character_" + i));
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
				Children.push_back(newNode);
				continue;
			}

			std::shared_ptr<LRoomDOMNode> entityRoom = GetRoomByNumber(newNode->GetRoomNumber());
			if (entityRoom != nullptr)
				entityRoom->Children.push_back(newNode);
		}
	}

	return true;
}

bool LMapDOMNode::LoadArchive(const char* path, GCarchive* archive)
{
	GCerror err;
	if ((err = gcInitContext(&mArcLoaderContext)) != GC_ERROR_SUCCESS) {
		printf("Error initing arc loader context: %s\n", gcGetErrorMessage(err));
		return false;
	}

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
