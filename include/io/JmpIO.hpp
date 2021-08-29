#pragma once

#include "../lib/bStream/bstream.h"
#include "GenUtil.hpp"
#include <memory>
#include <vector>

constexpr size_t JMP_HEADER_SIZE = 16;
constexpr size_t JMP_FIELD_DEF_SIZE = 12;
constexpr uint32_t JMP_HASH_PRIME = 33554393;

class LEntityDOMNode;

enum class EJmpFieldType : uint8_t
{
	Integer,
	String,
	Float
};

// Definition of a field making up an entry in the JMP files.
struct LJmpFieldInfo
{
	// The hash of the name of this field.
	uint32_t Hash;
	// The bitmask that isolates this field within a bitfield.
	uint32_t Bitmask;
	// The offset of this field within the JMP entry.
	uint16_t Start;
	// The shift required to compensate for the bitmask.
	uint8_t Shift;
	// The underlying data type of this field.
	EJmpFieldType Type;
};

// Handles reading and writing data from the map JMP files.
class LJmpIO
{
	// Number of individual entries in this JMP file.
	int32_t mEntryCount;
	// Number of fields defined in this JMP file.
	int32_t mFieldCount;
	// Offset of the first entry in this JMP file.
	uint32_t mEntryStartOffset;
	// Size of an entry in this JMP file.
	uint32_t mEntrySize;

	// Size of a string. Can vary between 16 and 32.
	size_t mStringSize;

	// A vector of the fields that define the data within the JMP entries.
	std::vector<LJmpFieldInfo> mFields;

	// Pointer to the data blob containing the entries in this JMP file.
	uint8_t* mData;

	// Hashes the given field name so that the field can be found from the list of loaded instances.
	uint32_t HashFieldName(std::string name) const;

	// Returns a pointer to the field info corresponding to the given name if it exists within this JMP file,
	// or nullptr if it does not exist.
	const LJmpFieldInfo* FetchJmpFieldInfo(std::string name);
	// Retrieves the unsigned integer at the given offset from this JMP file's entry data.
	uint32_t PeekU32(uint32_t offset);
	// Retrieves the signed integer at the given offset from this JMP file's entry data.
	int32_t PeekS32(uint32_t offset);
	// Retrieves the float at the given offset from this JMP file's entry data.
	float PeekF32(uint32_t offset);

	bool PokeU32(uint32_t offset, uint32_t value);
	bool PokeS32(uint32_t offset, int32_t value);
	bool PokeF32(uint32_t offset, float value);

	// Recalculates the size of each entry by examining the fields defining the entry data.
	uint32_t CalculateNewEntrySize();

public:
	LJmpIO();

	int32_t GetEntryCount() const { return mEntryCount; }
	uint32_t GetEntrySize() const { return mEntrySize; }
	int32_t GetFieldCount() const { return mFieldCount; }

	size_t GetStringSize() const { return mStringSize; }
	void SetStringSize(uint32_t newStringSize) { mStringSize = newStringSize; }

	size_t CalculateNewFileSize(size_t entityCount) { return LGenUtility::PadToBoundary(JMP_HEADER_SIZE + (mFieldCount * JMP_FIELD_DEF_SIZE) + (entityCount * mEntrySize), 32); }

/*== Input ==*/
	// Attempts to load a JMP file from the given stream. Returns
	// true if the load succeeded, false if not.
	bool Load(bStream::CMemoryStream* stream);

	// Attempts to return the value of the given field from the given JMP entry
	// as an unsigned int; returns 0 if the field is invalid.
	uint32_t GetUnsignedInt(uint32_t entry_index, std::string field_name);

	// Attempts to return the value of the given field from the given JMP entry
	// as an signed int; returns 0 if the field is invalid.
	int32_t GetSignedInt(uint32_t entry_index, std::string field_name);

	// Attempts to return the value of the given field from the given JMP entry
	// as float; returns 0.0f if the field is invalid.
	float GetFloat(uint32_t entry_index, std::string field_name);

	// Attempts to return the value of the given field from the given JMP entry
	// as a boolean; returns false if the field is invalid.
	bool GetBoolean(uint32_t entry_index, std::string field_name);

	// Attempts to return the value of the given field from the given JMP entry
	// as a string; returns "(null)" if the field is invalid.
	std::string GetString(uint32_t entry_index, std::string field_name);

/*== Output ==*/
	// Saves the current JMP data to the given stream.
	bool Save(std::vector<std::shared_ptr<LEntityDOMNode>> entities, bStream::CMemoryStream& stream);

	// Writes an unsigned int to the given field in the specified JMP entry,
	// packing into a bitfield if required.
	bool SetUnsignedInt(uint32_t entry_index, std::string field_name, uint32_t value);

	// Writes a signed int to the given field in the specified JMP entry.
	bool SetSignedInt(uint32_t entry_index, std::string field_name, int32_t value);

	// Writes a float to the given field in the specified JMP entry.
	bool SetFloat(uint32_t entry_index, std::string field_name, float value);

	// Writes a boolean to the given field in the specified JMP entry,
	// packing into a bitfield if required.
	bool SetBoolean(uint32_t entry_index, std::string field_name, bool value);

	// Writes a string to the given field in the specified JMP entry; pads the string
	// to 32 bytes in length.
	bool SetString(uint32_t entry_index, std::string field_name, std::string value);
};
