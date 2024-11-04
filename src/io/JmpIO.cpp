#include "io/JmpIO.hpp"
#include "DOM/EntityDOMNode.hpp"
#include "GenUtil.hpp"
#include <map>
#include <format>

LJmpIO::LJmpIO()
{

}

LJmpIO::~LJmpIO()
{

}

bool LJmpIO::Load(bStream::CMemoryStream* stream)
{
	mEntryCount = stream->readInt32();
	mFieldCount = stream->readInt32();
	mEntryStartOffset = stream->readUInt32();
	mEntrySize = stream->readUInt32();

	if (mEntrySize == 0 || mEntryStartOffset + mEntrySize * mEntryCount > stream->getSize())
		return false;


	mFields.clear();
	mFields.reserve(mFieldCount);

	for (int32_t i = 0; i < mFieldCount; i++)
	{
		LJmpFieldInfo newField;
		newField.Hash = stream->readUInt32();
		newField.Bitmask = stream->readUInt32();
		newField.Start = stream->readUInt16();
		newField.Shift = stream->readUInt8();
		newField.Type = (EJmpFieldType)stream->readUInt8();

		mFields.push_back(newField);
	}

	mData.clear();
	mData.reserve(mEntryCount);

	uint32_t stringTableOffset = mEntryStartOffset + mEntrySize * mEntryCount;

	for(int32_t i = 0; i < mEntryCount; i++)
	{
		LJmpEntry entry;
		for (const LJmpFieldInfo& f : mFields)
		{
			LJmpValue value;

			stream->seek(mEntryStartOffset + (mEntrySize * i) + f.Start);
			switch(f.Type){
				case EJmpFieldType::Integer:
					std::get<uint32_t>(value) = (stream->readUInt32() & f.Bitmask) >> f.Shift;
					break;
				case EJmpFieldType::Float:
					std::get<float>(value) = stream->readFloat();
					break;
				case EJmpFieldType::String:
					std::get<std::string>(value) = stream->readString(mStringSize);
			}

			entry.insert({f.Hash, value});
		}
		mData.push_back(entry);
	}

	return true;
}

bool LJmpIO::Load(nlohmann::json jsonTemplate, size_t count)
{
	nlohmann::json fieldsArray = jsonTemplate["fields"];

	mFields.clear();
	mFields.reserve(fieldsArray.size());

	for (auto field : fieldsArray)
	{
		LJmpFieldInfo newField;
		newField.Hash = field["hash"];
		newField.Bitmask = field["mask"];
		newField.Start = field["start"];
		newField.Shift = field["shift"];
		newField.Type = (EJmpFieldType)field["type"];

		mFields.push_back(newField);

		mFieldCount++;
	}

	mEntryCount = count;
	mFieldCount = static_cast<int32_t>(mFields.size());
	mEntryStartOffset = JMP_HEADER_SIZE + (mFieldCount * JMP_FIELD_DEF_SIZE);
	mEntrySize = CalculateNewEntrySize();

	if (mEntrySize == 0)
		return false;

	mData.clear();
	mData.reserve(mEntryCount);

	return true;
}


uint32_t LJmpIO::HashFieldName(std::string name) const
{
	uint32_t hash = 0;

	for (char c : name)
	{
		hash = ((hash << 8) + c) % JMP_HASH_PRIME;
	}

	return hash;
}

const LJmpFieldInfo* LJmpIO::FetchJmpFieldInfo(std::string name)
{
	const LJmpFieldInfo* field = nullptr;
	uint32_t nameHash = HashFieldName(name);

	for (const LJmpFieldInfo& f : mFields)
	{
		if (nameHash == f.Hash)
		{
			field = &f;
			break;
		}
	}

	return field;
}

const LJmpFieldInfo* LJmpIO::FetchJmpFieldInfo(uint32_t hash)
{
	const LJmpFieldInfo* field = nullptr;

	for (const LJmpFieldInfo& f : mFields)
	{
		if (hash == f.Hash)
		{
			field = &f;
			break;
		}
	}

	return field;
}

uint32_t LJmpIO::CalculateNewEntrySize()
{
	uint32_t newSize = 0;

	for (const LJmpFieldInfo f : mFields)
	{
		uint32_t tempNewSize = f.Start;

		if (f.Type == EJmpFieldType::String)
			tempNewSize += mStringSize;
		else
			tempNewSize += sizeof(uint32_t);

		newSize = std::max(newSize, tempNewSize);
	}

	return newSize;
}


uint32_t LJmpIO::GetUnsignedInt(uint32_t entry_index, std::string field_name){
	const LJmpFieldInfo* field = FetchJmpFieldInfo(field_name);

	if(entry_index < mData.size() && field != nullptr){
		return std::get<uint32_t>(mData.at(entry_index).at(field->Hash));
	} else {
		return 0;
	}
}


int32_t LJmpIO::GetSignedInt(uint32_t entry_index, std::string field_name){
	const LJmpFieldInfo* field = FetchJmpFieldInfo(field_name);

	if(entry_index < mData.size() && field != nullptr){
		return (int32_t)std::get<uint32_t>(mData.at(entry_index).at(HashFieldName(field_name)));
	} else {
		return 0;
	}
}

int32_t LJmpIO::GetSignedInt(uint32_t entry_index, uint32_t field_hash){
	const LJmpFieldInfo* field = FetchJmpFieldInfo(field_hash);

	if(entry_index < mData.size() && field != nullptr){
		return (int32_t)std::get<uint32_t>(mData.at(entry_index).at(field_hash));
	} else {
		return 0;
	}
}

float LJmpIO::GetFloat(uint32_t entry_index, std::string field_name){
	const LJmpFieldInfo* field = FetchJmpFieldInfo(field_name);

	if(entry_index < mData.size() && field != nullptr){
		return std::get<float>(mData.at(entry_index).at(field->Hash));
	} else {
		return 0.0f;
	}
}

bool LJmpIO::GetBoolean(uint32_t entry_index, std::string field_name){
	const LJmpFieldInfo* field = FetchJmpFieldInfo(field_name);

	if(entry_index < mData.size() && field != nullptr){
		return std::get<uint32_t>(mData.at(entry_index).at(field->Hash)) > 0;
	} else {
		return false;
	}
}

std::string LJmpIO::GetString(uint32_t entry_index, std::string field_name){
	const LJmpFieldInfo* field = FetchJmpFieldInfo(field_name);

	if(entry_index < mData.size() && field != nullptr){
		return LGenUtility::SjisToUtf8(std::get<std::string>(mData.at(entry_index).at(field->Hash)));
	} else {
		return "";
	}
}

bool LJmpIO::SetUnsignedInt(uint32_t entry_index, std::string field_name, uint32_t value){
	const LJmpFieldInfo* field = FetchJmpFieldInfo(field_name);

	if(entry_index < mData.size() && field != nullptr){
		std::get<uint32_t>(mData.at(entry_index).at(field->Hash)) = value;
		return true;
	} else {
		return false;
	}
}

bool LJmpIO::SetSignedInt(uint32_t entry_index, std::string field_name, int32_t value){
	const LJmpFieldInfo* field = FetchJmpFieldInfo(field_name);

	if(entry_index < mData.size() && field != nullptr){
		std::get<uint32_t>(mData.at(entry_index).at(field->Hash)) = (uint32_t)value;
		return true;
	} else {
		return false;
	}
}

bool LJmpIO::SetSignedInt(uint32_t entry_index, uint32_t field_hash, int32_t value){
	const LJmpFieldInfo* field = FetchJmpFieldInfo(field_hash);

	if(entry_index < mData.size() && field != nullptr){
		std::get<uint32_t>(mData.at(entry_index).at(field->Hash)) = (uint32_t)value;
		return true;
	} else {
		return false;
	}
}

bool LJmpIO::SetFloat(uint32_t entry_index, std::string field_name, float value){
	const LJmpFieldInfo* field = FetchJmpFieldInfo(field_name);

	if(entry_index < mData.size() && field != nullptr){
		std::get<float>(mData.at(entry_index).at(field->Hash)) = value;
		return true;
	} else {
		return false;
	}	
}

bool LJmpIO::SetBoolean(uint32_t entry_index, std::string field_name, bool value){
	const LJmpFieldInfo* field = FetchJmpFieldInfo(field_name);

	if(entry_index < mData.size() && field != nullptr){
		return SetUnsignedInt(entry_index, field_name, (uint32_t)value);
	} else {
		return false;
	}	
}


bool LJmpIO::SetString(uint32_t entry_index, std::string field_name, std::string value){
	const LJmpFieldInfo* field = FetchJmpFieldInfo(field_name);

	if(entry_index < mData.size() && field != nullptr){
		std::get<std::string>(mData.at(entry_index).at(field->Hash)) = LGenUtility::Utf8ToSjis(value);
		return true;
	} else {
		return false;
	}	
}

bool LJmpIO::Save(std::vector<std::shared_ptr<LEntityDOMNode>> entities, bStream::CMemoryStream& stream){

	mEntryCount = entities.size();
	mEntrySize = CalculateNewEntrySize();
	mEntryStartOffset = mFieldCount * sizeof(LJmpFieldInfo) + JMP_HEADER_SIZE;

	stream.writeInt32(mEntryCount);
	stream.writeInt32(mFieldCount);
	stream.writeUInt32(mEntryStartOffset);
	stream.writeUInt32(mEntrySize);

	LJmpEntry entry; //empty entry data

	// Write the field info data
	for (const LJmpFieldInfo f : mFields)
	{
		stream.writeUInt32(f.Hash);
		stream.writeUInt32(f.Bitmask);
		stream.writeUInt16(f.Start);
		stream.writeUInt8(f.Shift);
		stream.writeUInt8((uint8_t)f.Type);

		entry.insert({f.Hash, LJmpValue()});
	}

	mData.clear();
	mData.reserve(entities.size());

	for (uint32_t i = 0; i < entities.size(); i++)
	{
		mData.push_back(entry); //Add empty dummy entry
		entities[i]->Serialize(this, i); // set entry data
	}

	size_t fileSize = CalculateNewFileSize(mEntryCount);

	uint8_t* tempBuffer = new uint8_t[fileSize]{};

	bStream::CMemoryStream ReadStream(tempBuffer, fileSize, bStream::Endianess::Big, bStream::OpenMode::In);
	bStream::CMemoryStream WriteStream(tempBuffer, fileSize, bStream::Endianess::Big, bStream::OpenMode::Out);

	for (uint32_t i = 0; i < entities.size(); i++)
	{
		for (const LJmpFieldInfo& f : mFields)
		{
			uint32_t offset = -1;
			ReadStream.seek((mEntrySize * i) + f.Start);
			WriteStream.seek((mEntrySize * i) + f.Start);
			switch(f.Type){

				case EJmpFieldType::Integer:
					{
						uint32_t value = ReadStream.readUInt32(); // Value already in place
						value = (value & ~f.Bitmask) | ((std::get<uint32_t>(mData.at(i).at(f.Hash)) << f.Shift) & f.Bitmask);
						WriteStream.writeUInt32(value);
					}
					break;
				case EJmpFieldType::Float:
					WriteStream.writeFloat(std::get<float>(mData.at(i).at(f.Hash)));
					break;
				case EJmpFieldType::String:
					{
						char buffer[32] = {'\0'};
						std::string str = std::get<std::string>(mData.at(i).at(f.Hash));
						strncpy(buffer, str.c_str(), str.size() > mStringSize ? mStringSize : str.size());
						WriteStream.writeBytes((uint8_t*)buffer, mStringSize);
					}
					break;
			}
		}
	}

	stream.seek(mEntryStartOffset);
	stream.writeBytes(tempBuffer, fileSize);

	delete[] tempBuffer;

	return true;
}