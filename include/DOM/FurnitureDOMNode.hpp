#pragma once

#include "EntityDOMNode.hpp"

class LFurnitureDOMNode : public LEntityDOMNode
{
	// Furniture's name in the JMP file. Usually just the string "furniture".
	std::string mInternalName;
	// Name of the *.bin file containing this piece of furniture's model, located in its room's *.arc file.
	std::string mModelName;
	// Name that allows enemies to link to and hide within this piece of furniture.
	std::string mAccessName;

	// Vertical offset of the item spawned when this piece of furniture is interacted with, if present.
	float mVerticalItemSpawnOffset;

	// Index of an entry into the itemappeartable that determines what item comes out of this piece of furniture when interacted with.
	// TODO: Make this a pointer when the itemappeartable type is implemented.
	int32_t mItemTableIndex;
	// Determines the amount of money spawned when mItemTableIndex specifies the 'money' type.
	int32_t mGenerateNumber;
	// How likely a Boo is to spawn inside this piece of furniture.
	int32_t mBooHideChance;
	// How intensely this piece of furniture will shake when interacted with.
	int32_t mShakeIntensity;

	// Float arguments used depending on this piece of furniture's behavior type.
	glm::vec3 mVecArgs;

	// A flag determining when this piece of furniture can begin spawning.
	int32_t mSpawnFlag;
	// A flag determining when this piece of furniture will stop spawning.
	int32_t mDespawnFlag;

	// The lengths of each side of this piece of furniture's hitbox.
	glm::ivec3 mHitboxExtents;

	// An ID that determines what Luigi will say when scanning this piece of furniture with the GameBoy Horror.
	int32_t mGBHScanID;
	// How this piece of furniture reacts to be interacted with.
	uint32_t mBehaviorType;
	// The sound effect played when this piece of furniture is interacted with.
	uint32_t mSoundID;
	// Whether a sheet is covering this piece of furniture, and how it behaves.
	uint32_t mSheetBehavior;
	// Determines what kind of money is spawned when mItemTableIndex specifies the 'money' type.
	uint32_t mMoneyType;
	// The texture used for the sheet covering this piece of furniture, if present.
	uint32_t mSheetTexture;

	// Determines if this piece of furniture is always rendered (true) or fades away when Luigi walks behind it (false).
	bool mShouldCutaway;
	// If true, Luigi can vaccuum up the sheet covering this piece of furniture, if present. Otherwise, the sheet is a trap.
	bool mCanSheetBeVaccuumed;
	
	// Unused.
	bool mBooAppear;

public:
	typedef LEntityDOMNode Super;

	LFurnitureDOMNode(std::string name);

	// Writes the data this JMP node into the given LJmpIO instance at the specified entry.
	virtual void Serialize(LJmpIO* JmpIO, uint32_t entry_index) const override;
	// Reads the data from the specified entry in the given LJmpIO instance into this JMP node.
	virtual void Deserialize(LJmpIO* JmpIO, uint32_t entry_index) override;
};
