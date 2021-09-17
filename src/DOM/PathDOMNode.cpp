#include "DOM/PathDOMNode.hpp"
#include "UIUtil.hpp"
#include "GenUtil.hpp"
#include "../lib/libgctools/include/archive.h"

#include <memory>

LPathPointDOMNode::LPathPointDOMNode(std::string name) : Super(name)
{
	mType = EDOMNodeType::PathPoint;
}

void LPathPointDOMNode::Serialize(LJmpIO* JmpIO, uint32_t entry_index) const
{
	JmpIO->SetFloat(entry_index, "pnt0_x", X.Value);
	JmpIO->SetFloat(entry_index, "pnt1_x", X.InTangent);
	JmpIO->SetFloat(entry_index, "pnt2_x", X.OutTangent);

	JmpIO->SetFloat(entry_index, "pnt0_y", Y.Value);
	JmpIO->SetFloat(entry_index, "pnt1_y", Y.InTangent);
	JmpIO->SetFloat(entry_index, "pnt2_y", Y.OutTangent);

	JmpIO->SetFloat(entry_index, "pnt0_z", Z.Value);
	JmpIO->SetFloat(entry_index, "pnt1_z", Z.InTangent);
	JmpIO->SetFloat(entry_index, "pnt2_z", Z.OutTangent);

	JmpIO->SetSignedInt(entry_index, "next_room", NextRoomNumber);
	JmpIO->SetSignedInt(entry_index, 0x809B31, UnkInt1);
	JmpIO->SetSignedInt(entry_index, 0x61DCB2, UnkInt2);
	JmpIO->SetSignedInt(entry_index, 0xD9B7C6, UnkInt3);
}

void LPathPointDOMNode::Deserialize(LJmpIO* JmpIO, uint32_t entry_index)
{
	X.Value = JmpIO->GetFloat(entry_index, "pnt0_x");
	X.InTangent = JmpIO->GetFloat(entry_index, "pnt1_x");
	X.OutTangent = JmpIO->GetFloat(entry_index, "pnt2_x");

	Y.Value = JmpIO->GetFloat(entry_index, "pnt0_y");
	Y.InTangent = JmpIO->GetFloat(entry_index, "pnt1_y");
	Y.OutTangent = JmpIO->GetFloat(entry_index, "pnt2_y");

	Z.Value = JmpIO->GetFloat(entry_index, "pnt0_z");
	Z.InTangent = JmpIO->GetFloat(entry_index, "pnt1_z");
	Z.OutTangent = JmpIO->GetFloat(entry_index, "pnt2_z");

	NextRoomNumber = JmpIO->GetSignedInt(entry_index, "next_room");
	UnkInt1 = JmpIO->GetSignedInt(entry_index, 0x809B31);
	UnkInt2 = JmpIO->GetSignedInt(entry_index, 0x61DCB2);
	UnkInt3 = JmpIO->GetSignedInt(entry_index, 0xD9B7C6);
}

LPathDOMNode::LPathDOMNode(std::string name) : Super(name)
{
	mType = EDOMNodeType::Path;
}

void LPathDOMNode::RenderDetailsUI(float dt)
{
	/*LUIUtility::RenderTransformUI(mTransform.get(), mPosition, mRotation, mScale);

	// Strings
	LUIUtility::RenderTextInput("Character Name", &mName);
	LUIUtility::RenderTooltip("What character this entity is.");

	LUIUtility::RenderTextInput("Spawn Group", &mCreateName);
	LUIUtility::RenderTooltip("What Spawn Group this character is in. Set this to ---- to spawn when Luigi enters the room. This is also known as create_name.");

	LUIUtility::RenderTextInput("Path Name", &mPathName);
	LUIUtility::RenderTooltip("The name of a path file that this character will use. Set this to (null) for no path.");

	LUIUtility::RenderTextInput("Script Name", &mCodeName);
	LUIUtility::RenderTooltip("The name that can be used to reference this character in an event.");

	// Integers
	ImGui::InputInt("Spawn Flag", &mSpawnFlag);
	LUIUtility::RenderTooltip("The flag that must be set before this character will begin spawning.");
	ImGui::InputInt("Despawn Flag", &mDespawnFlag);
	LUIUtility::RenderTooltip("If this flag is set, this character will no longer spawn.");

	ImGui::InputInt("event_set_no", &mEventSetNumber);
	LUIUtility::RenderTooltip("Don't quite know what this does. It's called event_set_no by the game, but it seems useless?");

	if (mName == "luige")
	{
		ImGui::InputInt("Luigi Spawn ID", &mSpawnPointID);
		LUIUtility::RenderTooltip("The ID of this Luigi spawn point. Luigi will spawn into a map by default at ID 0; any other IDs must be accessed via events.");
	}

	ImGui::InputInt("GameBoy Horror Scan ID", &mGBHScanID);
	LUIUtility::RenderTooltip("The ID that determines what message is displayed by a Portrait Ghost when its heart is scanned by the GameBoy Horror.");

	// Comboboxes
	auto mapNode = GetParentOfType<LMapDOMNode>(EDOMNodeType::Map);
	LUIUtility::RenderNodeReferenceCombo<LItemAppearDOMNode>("Defeated Item Table", EDOMNodeType::ItemAppear, mapNode, mItemTableRef);
	LUIUtility::RenderTooltip("The Item Appear entry to use to spawn items when this character is defeated.");

	LUIUtility::RenderComboEnum<EConditionType>("Spawn Condition", mCondType);
	LUIUtility::RenderTooltip("The condition governing whether the character is allowed to spawn.");

	// This may be unused/useless?
	//LUIUtility::RenderComboEnum<EAppearType>("Spawn Type", mAppearType);
	//LUIUtility::RenderTooltip("The way in which the character will spawn. Using the second option WILL crash the game.");

	LUIUtility::RenderComboEnum<EAttackType>("Behavior Type", mAttackType);
	LUIUtility::RenderTooltip("How this character will behave.");

	// Bools
	LUIUtility::RenderCheckBox("Visible?", &mIsVisible);
	LUIUtility::RenderTooltip("Whether this character should be visible.");

	LUIUtility::RenderCheckBox("Stay on path between rooms?", &mStay);
	LUIUtility::RenderTooltip("Whether this character should remain on a path when Luigi moves between rooms.");*/
}

void LPathDOMNode::Serialize(LJmpIO* JmpIO, uint32_t entry_index) const
{
	JmpIO->SetString(entry_index, "name", mName);
	JmpIO->SetString(entry_index, "type", mInterpolationType);
	JmpIO->SetString(entry_index, "next", mNextPathName);

	JmpIO->SetSignedInt(entry_index, "no", mOrganizationNumber);
	JmpIO->SetSignedInt(entry_index, "num_pnt", mNumPoints);
	JmpIO->SetSignedInt(entry_index, "room_no", mRoomNumber);
	JmpIO->SetSignedInt(entry_index, "arg0", mArg0);

	JmpIO->SetUnsignedInt(entry_index, "do_type", mDoType);

	JmpIO->SetBoolean(entry_index, "closed", mIsClosed);
	JmpIO->SetBoolean(entry_index, "use", mUse);
}

void LPathDOMNode::Deserialize(LJmpIO* JmpIO, uint32_t entry_index)
{
	mName = JmpIO->GetString(entry_index, "name");
	mInterpolationType = JmpIO->GetString(entry_index, "type");
	mNextPathName = JmpIO->GetString(entry_index, "next");

	mOrganizationNumber = JmpIO->GetSignedInt(entry_index, "no");
	mNumPoints = JmpIO->GetSignedInt(entry_index, "num_pnt");
	mRoomNumber = JmpIO->GetSignedInt(entry_index, "room_no");
	mArg0 = JmpIO->GetSignedInt(entry_index, "arg0");

	mDoType = JmpIO->GetUnsignedInt(entry_index, "do_type");

	mIsClosed = JmpIO->GetBoolean(entry_index, "closed");
	mUse = JmpIO->GetBoolean(entry_index, "use");
}

void LPathDOMNode::PostProcess()
{
	// On the off chance that the parent is invalid, don't try to do anything.
	if (Parent.expired())
		return;

	// Grab a temporary shared_ptr for the parent.
	//auto parentShared = Parent.lock();
}

void LPathDOMNode::PreProcess()
{
	// On the off chance that the parent is invalid, don't try to do anything.
	if (Parent.expired())
		return;

	// Grab a temporary shared_ptr for the parent.
	//auto parentShared = Parent.lock();
}

void LPathDOMNode::PostProcess(const GCarchive& mapArchive)
{
	bStream::CMemoryStream fileMemStream;

	// Find the correct path file in the archive
	for (uint32_t i = 0; i < mapArchive.filenum; i++)
	{
		if (strcmp(mapArchive.files[i].name, mName.c_str()) == 0)
		{
			fileMemStream = bStream::CMemoryStream(static_cast<uint8_t*>(mapArchive.files[i].data), mapArchive.files[i].size, bStream::Endianess::Big, bStream::OpenMode::In);
			break;
		}
	}

	// If the file wasn't found, emit an error and return
	if (fileMemStream.getSize() == 0)
	{
		std::cout << "Unable to load path file " << mName << " !" << std::endl;
		return;
	}

	LJmpIO pathLoader = LJmpIO();
	pathLoader.Load(&fileMemStream);

	for (size_t i = 0; i < mNumPoints; i++)
	{
		std::shared_ptr<LPathPointDOMNode> newPoint = std::make_shared<LPathPointDOMNode>("Path Point");
		newPoint->Deserialize(&pathLoader, i);

		AddChild(newPoint);
	}
}

void LPathDOMNode::PreProcess(LJmpIO& pathJmp, bStream::CMemoryStream& pathStream)
{
	pathJmp.Save(GetChildrenOfType<LEntityDOMNode>(EDOMNodeType::PathPoint), pathStream);
}
