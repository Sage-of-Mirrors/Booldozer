#pragma once

#include "../lib/bStream/bstream.h"

enum class EJmpFieldType
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
	uint32_t Start;
	// The shift required to compensate for the bitmask.
	uint32_t Shift;
	// The underlying data type of this field.
	EJmpFieldType Type;
};

// Handles reading and writing data from the map JMP files.
class LJmpIO
{
	// Number of individual entries in this JMP file.
	int32_t EntryCount;
	// Number of fields defined in this JMP file.
	int32_t Fieldcount;
	// Offset of the first entry in this JMP file.
	uint32_t EntryStartOffset;
	// Size of an entry in this JMP file.
	uint32_t EntrySize;

	// Pointer to the data blob containing the entries in this JMP file.
	uint8_t* mData;

public:
	LJmpIO(bStream::CMemoryStream* stream);

/*== Input ==*/
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
	// Writes an unsigned int to the given field in the specified JMP entry,
	// packing into a bitfield if required.
	void SetUnsignedInt(uint32_t entry_index, std::string field_name, uint32_t value);

	// Writes a signed int to the given field in the specified JMP entry.
	void SetSignedInt(uint32_t entry_index, std::string field_name, int32_t value);

	// Writes a float to the given field in the specified JMP entry.
	void SetFloat(uint32_t entry_index, std::string field_name, float value);

	// Writes a boolean to the given field in the specified JMP entry,
	// packing into a bitfield if required.
	void SetBoolean(uint32_t entry_index, std::string field_name, bool value);

	// Writes a string to the given field in the specified JMP entry; pads the string
	// to 32 bytes in length.
	void SetString(uint32_t entry_index, std::string field_name, std::string value);
};
