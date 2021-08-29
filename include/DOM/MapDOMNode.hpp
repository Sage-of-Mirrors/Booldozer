#pragma once

#include "DOMNodeBase.hpp"
#include "EntityDOMNode.hpp"
#include "io/JmpIO.hpp"
#include "../lib/libgctools/include/archive.h"

#include <filesystem>

extern std::string const LEntityFileNames[];

class LRoomDOMNode;

// DOM node representing an entire map, including rooms and objects.
class LMapDOMNode : public LDOMNodeBase
{
	GCcontext mArcLoaderContext;
	LJmpIO JmpIOManagers[LEntityType_Max];

	bool LoadArchive(const char* path, GCarchive* archive);
	bool LoadEntityNodes(LJmpIO* jmp_io, LEntityType type);

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
