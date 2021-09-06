#pragma once

#include "DOMNodeBase.hpp"
#include "EntityDOMNode.hpp"
#include "io/JmpIO.hpp"
#include "io/StaticMapDataIO.hpp"
#include "../lib/libgctools/include/archive.h"
#include "ResUtil.hpp"

#include <filesystem>

extern std::string const LEntityFileNames[];

class LRoomDOMNode;

// DOM node representing an entire map, including rooms and objects.
class LMapDOMNode : public LDOMNodeBase
{
	LJmpIO JmpIOManagers[LEntityType_Max];
	LStaticMapDataIO mStaticMapIO;

	bool LoadEntityNodes(LJmpIO* jmp_io, LEntityType type);

	bool ReadStaticData(std::filesystem::path filePath);
	bool LoadStaticData(std::vector<std::shared_ptr<LRoomDOMNode>> rooms);

public:
	typedef LDOMNodeBase Super;

	LMapDOMNode();

	bool LoadMap(std::filesystem::path file_path);
	bool SaveMapToFiles(std::filesystem::path folder_path);
	std::shared_ptr<LRoomDOMNode> GetRoomByNumber(int32_t number);

/*=== Type operations ===*/
	// Returns whether this node is of the given type, or derives from a node of that type.
	virtual bool IsNodeType(EDOMNodeType type) const override
	{
		if (type == EDOMNodeType::Map)
			return true;

		return Super::IsNodeType(type);
	}
};
