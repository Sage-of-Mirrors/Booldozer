#include "DOM/RoomDOMNode.hpp"

LRoomDOMNode::LRoomDOMNode(std::string name) : LBGRenderDOMNode(name)
{
	mType = EDOMNodeType::Room;
}

void LRoomDOMNode::LoadJmpInfo(uint32_t index, LJmpIO* jmp_io)
{
	mInternalName = jmp_io->GetString(index, "name");

	mRoomNumber = jmp_io->GetSignedInt(index, "RoomNo");
	mThunder = jmp_io->GetSignedInt(index, "Thunder");

	mShouldRenderSkybox = jmp_io->GetBoolean(index, "VRbox");

	mDustLevel = jmp_io->GetSignedInt(index, "DustLv");

	mLightColor.r = jmp_io->GetSignedInt(index, "LightColorR");
	mLightColor.g = jmp_io->GetSignedInt(index, "LightColorG");
	mLightColor.b = jmp_io->GetSignedInt(index, "LightColorB");

	mDistance = jmp_io->GetSignedInt(index, "Distance");
	mLv = jmp_io->GetSignedInt(index, "Lv");
	mSoundEchoParameter = jmp_io->GetSignedInt(index, "sound_echo_parameter");
	mSoundRoomCode = jmp_io->GetSignedInt(index, "sound_room_code");

	mSoundRoomSize = jmp_io->GetUnsignedInt(index, "sound_room_size");
}

void LRoomDOMNode::SaveJmpInfo(uint32_t index, LJmpIO* jmp_io)
{
	jmp_io->SetString(index, "name", mInternalName);

	jmp_io->SetSignedInt(index, "RoomNo", mRoomNumber);
	jmp_io->SetSignedInt(index, "Thunder", mThunder);

	jmp_io->SetBoolean(index, "VRbox", mShouldRenderSkybox);

	jmp_io->SetSignedInt(index, "DustLv", mDustLevel);

	jmp_io->SetSignedInt(index, "LightColorR", mLightColor.r);
	jmp_io->SetSignedInt(index, "LightColorG", mLightColor.g);
	jmp_io->SetSignedInt(index, "LightColorB", mLightColor.b);

	jmp_io->SetSignedInt(index, "Distance", mDistance);
	jmp_io->SetSignedInt(index, "Lv", mLv);
	jmp_io->SetSignedInt(index, "sound_echo_parameter", mSoundEchoParameter);
	jmp_io->SetSignedInt(index, "sound_room_code", mSoundRoomCode);

	jmp_io->SetUnsignedInt(index, "sound_room_size", mSoundRoomSize);
}

bool LRoomDOMNode::CompleteLoad(GCarchive* room_arc)
{
	// Load the room models.
	for (int32_t i = 0; i < room_arc->filenum; i++)
	{
		uint8_t* fileData = nullptr;
		size_t fileSize = 0;
		bool isRoomBin = false;
		
		if (strstr(room_arc->files[i].name, ".bin"))
		{
			if (strcmp(room_arc->files[i].name, "room.bin") == 0)
			{
				isRoomBin = true;
			}

			fileData = (uint8_t*)room_arc->files[i].data;
			fileSize = room_arc->files[i].size;
		}

		// This file was determined to not be a BIN
		if (fileData == nullptr)
		{
			continue;
		}

		bStream::CMemoryStream memStrm = bStream::CMemoryStream(fileData, fileSize, bStream::Endianess::Big, bStream::OpenMode::In);

		// Load this file as the room's main model
		if (isRoomBin)
		{
			mRoomModel.LoadBIN(&memStrm);
		}
		// Into the furniture pile you go!
		else
		{
			auto furnModel = std::shared_ptr<LModel>(new LModel());
			furnModel->LoadBIN(&memStrm);

			mFurnitureModels.push_back(furnModel);
		}
	}

	return true;
}
