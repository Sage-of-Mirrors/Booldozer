#pragma once

#include "../lib/bStream/bstream.h"

#include <string>

class LModel
{
	std::string mName;

public:
	LModel();

	bool LoadBIN(bStream::CMemoryStream* stream);
	bool LoadMDL(bStream::CMemoryStream* stream);

	void Render();
};
