#pragma once

#include "EntityDOMNode.hpp"

class LItemAppearDOMNode;

enum class EMoneyType : uint32_t
{
	None,
	Coins,
	Bills,
	Coins_and_Bills,
	Sapphires,
	Rubies,
	Emeralds,
	Gold_Bars,
	Red_Diamonds
};

enum class ESheetTexture : uint32_t
{
	Tablecloth_Gold,
	Towel,
	T_Shirt,
	Poster_Ghost_Musician,
	Tablecloth_Floral,
	Bedsheets_Striped,
	Poster_Boo_Danger,
	Bedsheets_Quilt,
	Tablecloth_Parlor,
	Sheet_Holes,
	Projection_Screen,
	Sheet_Foyer_Mirror,
	Poster_Western,
	Poster_Boo_Leave,
	Poster_Monsters,
	Poster_Daisy
};

enum class ESheetBehavior : uint32_t
{
	None,

	Tablecloth,
	Tablecloth_Parlor,
	Tablecloth_2,
	Tablecloth_Parlor_2,

	Cloth_Overhang_Front,
	Cloth_Overhang_Right,
	Cloth_Overhang_Left,
	Cloth_Overhang_Back,

	Poster_Front,
	Poster_Right,
	Poster_Left,
	Poster_Back,

	Bedsheet_Master_Bedroom,
	Bedsheet_Unknown_1,
	Bedsheet_Unknown_2,
	Bedsheet_Unknown_3,
	Bedsheet_GuestRoom,
	Bedhseet_Unknown_4,
	Bedhseet_Unknown_5,
	Bedsheet_Master_Bedroom_2,

	Poster_Front_2,
	Poster_Right_2,
	Poster_Left_2,
	Poster_Back_2,

	Cloth_Flutter_Back
};

enum class EFurnitureSound : uint32_t
{
	Door_Opening,
	Wood_Knocking,
	Door_Opening_2,
	Wood_Tapping,
	Wood_Rattling_Slow,
	Wood_Rattling_Fast,

	Metal_Tapping_Thick,
	Metal_Tapping_Thin,
	Metal_Slamming,

	Plastic_Tapping,
	Leaves_Rustling,

	Heavy_Fall,
	Heavy_Break,

	Drawer_Opening,

	Harp,
	Cello,
	Marimba,
	Timpani_1,
	Timpani_2,
	Timpani_3,
	Saxophone,

	Unknown_1,

	Metal_Clanging,
	Metal_Crunching,

	Glass_Tapping_Thin,
	Glass_Tapping_Thick,

	Metal_Tapping_Muffled_Low,
	Metal_Tapping_Muffled_High,
	Metal_Tapping_Muffled_Mid
};

enum class EMoveType : uint32_t
{
	Heavy_No_Open,
	Spinning_With_Vaccuum_Spawns_Item,

	Normal_No_Open,
	Normal_No_Open_2,
	Normal_No_Open_3,

	Rocking_Sets_Flag_80,
	Musical_Instrument_Sets_Flag_90,

	Chandelier,

	Spinning_Sets_Flag_88,
	Fade_Toggles_With_Flag_50,
	Fade_Out_Heavy_With_Flag_50,
	Shining_Light_Sets_Flag_88,
	Moves_With_Flag41,
	Fade_Out_Chandelier_With_Flag_50,
	Shakes_With_Flag_94,

	Normal_Open,

	Knocking_Sets_Flag_arg0,

	Spins_With_Vaccum,
	Knocking_Opens,
	Knocking_Moves,
	Chandelier_Drops_While_Chauncey_Alive,
	CooCoo_Clock_ClockworkSoldiers,
	Mirror_Warp,
	Mouse_Hole_Warp,
	Knocking_Causes_Disappearence,
	Bogmire_Tombstone,

	Knocking_Causes_Damage_While_In_Front,
	Knight_Statue_With_Mace,
	Knight_Statue_With_Staff,

	Walking_Over_Flips_Gravity,
	Spins_When_Whirlindas_Active,

	Moves_With_Flag_arg0,
	Mouse_Hole_Warp_Lit_Only,

	Button_Sets_Flag_arg0,
	Watering_Spawns_Item,
	Burning_Spawns_Item,

	Knocking_Sets_Flag_114,
	Scan_Hitbox_Only,

	Interacting_Opens_Door,
	Zoom_And_Effects_With_Flag_arg0,
	Spins_With_Flag_arg0,
	Flips_With_Flag_115,
	Walking_On_Spawns_Item,

	Angle_Statue_Knocking_Sets_Flag_arg0
};

class LFurnitureDOMNode : public LEntityDOMNode
{
/*=== JMP properties ===*/
	// Name of the *.bin file containing this piece of furniture's model, located in its room's *.arc file.
	std::string mModelName;
	// Name that allows enemies to link to and hide within this piece of furniture.
	std::string mAccessName;

	// Vertical offset of the item spawned when this piece of furniture is interacted with, if present.
	float mVerticalItemSpawnOffset;

	// Index of an entry into the itemappeartable that determines what item comes out of this piece of furniture when interacted with.
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
	EMoveType mBehaviorType;
	// The sound effect played when this piece of furniture is interacted with.
	EFurnitureSound mSoundID;
	// Whether a sheet is covering this piece of furniture, and how it behaves.
	ESheetBehavior mSheetBehavior;
	// Determines what kind of money is spawned when mItemTableIndex specifies the 'money' type.
	EMoneyType mMoneyType;
	// The texture used for the sheet covering this piece of furniture, if present.
	ESheetTexture mSheetTexture;

	// Determines if this piece of furniture is always rendered (true) or fades away when Luigi walks behind it (false).
	bool mShouldCutaway;
	// If true, Luigi can vaccuum up the sheet covering this piece of furniture, if present. Otherwise, the sheet is a trap.
	bool mCanSheetBeVaccuumed;
	
	// Unused.
	bool mBooAppear;

	std::weak_ptr<LItemAppearDOMNode> mItemTableRef;

public:
	typedef LEntityDOMNode Super;

	LFurnitureDOMNode(std::string name);
	~LFurnitureDOMNode() { /*LGenUtility::Log << "cleaned up furniture node " << mModelName << std::endl;*/ }

	virtual std::string GetName() override { return mAccessName == "(null)" ? mModelName : mModelName + " [" + mAccessName + "]"; }
	std::string GetModelName() { return mModelName; }
	void SetModelName(std::string name) { mModelName = name; }

	glm::vec3 GetScanbox() { return mHitboxExtents; }

	std::string GetAccessName() const { return mAccessName; }
	void SetAccessName(std::string newAccessName) { mAccessName = newAccessName; }

	void CopyTo(LFurnitureDOMNode* other);

	virtual void RenderDetailsUI(float dt) override;

	// Writes the data this JMP node into the given LJmpIO instance at the specified entry.
	virtual void Serialize(LJmpIO* JmpIO, uint32_t entry_index) const override;
	// Reads the data from the specified entry in the given LJmpIO instance into this JMP node.
	virtual void Deserialize(LJmpIO* JmpIO, uint32_t entry_index) override;

	virtual void PostProcess() override;
	virtual void PreProcess() override;

/*=== Type operations ===*/
	// Returns whether this node is of the given type, or derives from a node of that type.
	virtual bool IsNodeType(EDOMNodeType type) const override
	{
		if (type == EDOMNodeType::Furniture)
			return true;

		return Super::IsNodeType(type);
	}
};
