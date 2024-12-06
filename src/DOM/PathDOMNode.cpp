#include "DOM/PathDOMNode.hpp"
#include "DOM/RoomDOMNode.hpp"
#include "UIUtil.hpp"
#include "GenUtil.hpp"

#include <memory>

std::map<std::string, std::string> InterpolationTypes = {
	{ "(null)", "None" },
	{ "Linear", "Linear" },
	{ "Bezier", "Bezier" },
};

LPathPointDOMNode::LPathPointDOMNode(std::string name) : Super(name)
{
	mType = EDOMNodeType::PathPoint;
}

void LPathPointDOMNode::Serialize(LJmpIO* JmpIO, uint32_t entry_index) const
{
	JmpIO->SetFloat(entry_index, "pnt0_x", Z.Value);
	JmpIO->SetFloat(entry_index, "pnt1_x", Z.InTangent);
	JmpIO->SetFloat(entry_index, "pnt2_x", Z.OutTangent);

	JmpIO->SetFloat(entry_index, "pnt0_y", Y.Value);
	JmpIO->SetFloat(entry_index, "pnt1_y", Y.InTangent);
	JmpIO->SetFloat(entry_index, "pnt2_y", Y.OutTangent);

	JmpIO->SetFloat(entry_index, "pnt0_z", X.Value);
	JmpIO->SetFloat(entry_index, "pnt1_z", X.InTangent);
	JmpIO->SetFloat(entry_index, "pnt2_z", X.OutTangent);

	JmpIO->SetSignedInt(entry_index, "next_room", mNextRoomNumber);
	JmpIO->SetSignedInt(entry_index, 0x809B31, mUnkInt1);
	JmpIO->SetSignedInt(entry_index, 0x61DCB2, mUnkInt2);
	JmpIO->SetSignedInt(entry_index, 0xD9B7C6, mUnkInt3);
}

void LPathPointDOMNode::Deserialize(LJmpIO* JmpIO, uint32_t entry_index)
{
	Z.Value = JmpIO->GetFloat(entry_index, "pnt0_x");
	Z.InTangent = JmpIO->GetFloat(entry_index, "pnt1_x");
	Z.OutTangent = JmpIO->GetFloat(entry_index, "pnt2_x");

	Y.Value = JmpIO->GetFloat(entry_index, "pnt0_y");
	Y.InTangent = JmpIO->GetFloat(entry_index, "pnt1_y");
	Y.OutTangent = JmpIO->GetFloat(entry_index, "pnt2_y");

	X.Value = JmpIO->GetFloat(entry_index, "pnt0_z");
	X.InTangent = JmpIO->GetFloat(entry_index, "pnt1_z");
	X.OutTangent = JmpIO->GetFloat(entry_index, "pnt2_z");

	mNextRoomNumber = JmpIO->GetSignedInt(entry_index, "next_room");
	mUnkInt1 = JmpIO->GetSignedInt(entry_index, 0x809B31);
	mUnkInt2 = JmpIO->GetSignedInt(entry_index, 0x61DCB2);
	mUnkInt3 = JmpIO->GetSignedInt(entry_index, 0xD9B7C6);
}

void LPathPointDOMNode::PostProcess()
{
	mPosition.x = X.Value;
	mPosition.y = Y.Value;
	mPosition.z = Z.Value;

	auto mapNode = GetParentOfType<LMapDOMNode>(EDOMNodeType::Map);
	if (auto mapNodeLocked = mapNode.lock())
		mRoomRef = mapNodeLocked->GetRoomByNumber(mNextRoomNumber);
}

void LPathPointDOMNode::PreProcess()
{
	X.Value = mPosition.x;
	Y.Value = mPosition.y;
	Z.Value = mPosition.z;

	if (mRoomRef.expired())
	{
		mRoomNumber = 0;
		return;
	}

	mNextRoomNumber = mRoomRef.lock()->GetRoomNumber();
}

void LPathPointDOMNode::RenderDetailsUI(float dt)
{
	auto mapNode = GetParentOfType<LMapDOMNode>(EDOMNodeType::Map);
	LUIUtility::RenderTransformUI(mTransform.get(), mPosition, mRotation, mScale);
	LUIUtility::RenderNodeReferenceCombo<LRoomDOMNode>("Next Room", EDOMNodeType::Room, mapNode, mRoomRef);
	LUIUtility::RenderTooltip("What room a Boo using this path to escape from Luigi should end up in.");

	ImGui::InputInt("Unknown 1", &mUnkInt1);
	LUIUtility::RenderTooltip("???");
	ImGui::InputInt("Unknown 2", &mUnkInt2);
	LUIUtility::RenderTooltip("???");
	ImGui::InputInt("Unknown 3", &mUnkInt3);
	LUIUtility::RenderTooltip("???");
}

LPathDOMNode::LPathDOMNode(std::string name) : Super(name)
{
	mType = EDOMNodeType::Path;
	mRoomNumber = -1;
}

void LPathDOMNode::RenderDetailsUI(float dt)
{

	LUIUtility::RenderTextInput("Path File", &mName);
	ImGui::InputInt("Room No", &mRoomNumber);
	
	// Integers
	ImGui::InputInt("ID", &mOrganizationNumber);
	LUIUtility::RenderTooltip("An ID to organize paths by floor. Seems to be unused by the game?");
	ImGui::InputInt("Arg 0", &mArg0);
	LUIUtility::RenderTooltip("Mostly sets the speed at which an actor traverses the path. May mean other things to other actors.");

	// Comboboxes
	auto mapNode = GetParentOfType<LMapDOMNode>(EDOMNodeType::Map);
	LUIUtility::RenderNodeReferenceCombo<LPathDOMNode>("Next Path", EDOMNodeType::Path, mapNode, mNextPathRef);
	LUIUtility::RenderTooltip("The next path in a sequence.");

	LUIUtility::RenderComboBox("Interpolation Type", InterpolationTypes, mInterpolationType);
	LUIUtility::RenderTooltip("What kind of interpolation this path uses. Ladders use Linear. Not sure what the other types do.");

	LUIUtility::RenderComboEnum<EPathDoType>("Do Type", mDoType);
	LUIUtility::RenderTooltip("This value allows the actor following this path to do something when getting to the end of it.");

	// Bools
	LUIUtility::RenderCheckBox("Is Path Closed", &mIsClosed);
	LUIUtility::RenderTooltip("Doesn't seem to be used; it probably would have marked this path as being a closed loop (last point -> first point).");

	LUIUtility::RenderCheckBox("Is Ladder", &mUse);
	LUIUtility::RenderTooltip("Whether this path can be used as a ladder when Luigi presses A next to it.");
}

void LPathDOMNode::Serialize(LJmpIO* JmpIO, uint32_t entry_index) const
{
	JmpIO->SetString(entry_index, "name", mName);
	JmpIO->SetString(entry_index, "type", mInterpolationType);
	JmpIO->SetString(entry_index, "next", mNextPathName);

	JmpIO->SetUnsignedInt(entry_index, "no", mOrganizationNumber);
	JmpIO->SetUnsignedInt(entry_index, "num_pnt", (int32_t)Children.size());
	JmpIO->SetUnsignedInt(entry_index, "room_no", mRoomNumber);
	JmpIO->SetUnsignedInt(entry_index, "arg0", mArg0);

	JmpIO->SetUnsignedInt(entry_index, "do_type", (uint32_t)mDoType);

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

	mDoType = (EPathDoType)JmpIO->GetUnsignedInt(entry_index, "do_type");

	mIsClosed = JmpIO->GetBoolean(entry_index, "closed");
	mUse = JmpIO->GetBoolean(entry_index, "use");
}

void LPathDOMNode::PostProcess()
{
	auto mapNode = GetParentOfType<LMapDOMNode>(EDOMNodeType::Map);
	if (auto mapNodeLocked = mapNode.lock())
	{
		for (auto p : mapNodeLocked->GetChildrenOfType<LPathDOMNode>(EDOMNodeType::Path))
		{
			if (p->GetName() == mNextPathName)
			{
				mNextPathRef = p;
				break;
			}
		}
	}
}


void LPathDOMNode::RenderPath(CPathRenderer* renderer){
}

void LPathDOMNode::PreProcess()
{
	if (auto nextPathLocked = mNextPathRef.lock())
		mNextPathName = nextPathLocked->GetName();
	else
		mNextPathName = "(null)";
}

void LPathDOMNode::PostProcess(std::shared_ptr<Archive::Rarc> mapArchive)
{

	// Find the correct path file in the archive

	std::shared_ptr<Archive::File> pathFile = mapArchive->GetFile(std::filesystem::path("path") / mName);

	
	if(pathFile == nullptr){
		LGenUtility::Log << "[PathDOMNode]: Unable to load path file " << mName << " !" << std::endl;
		return;
	}

	bStream::CMemoryStream fileMemStream(pathFile->GetData(), pathFile->GetSize(), bStream::Endianess::Big, bStream::OpenMode::In);

	LJmpIO pathLoader;
	pathLoader.Load(&fileMemStream);

	float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	float g = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	float b = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

	mPathColor = {r, g, b, 1.0f};

	for (size_t i = 0; i < mNumPoints; i++)
	{
		std::shared_ptr<LPathPointDOMNode> newPoint = std::make_shared<LPathPointDOMNode>("Path Point");
		newPoint->Deserialize(&pathLoader, i);
		

		AddChild(newPoint);

		newPoint->PostProcess();
		mPathRenderable.push_back({newPoint->GetPosition(), mPathColor, 12800});
	}

	// Boo escape paths are defined outside of the rooms they're for; skip sorting them.
	if (mName.find("escape") != std::string::npos)
		return;

	auto mapNode = GetParentOfType<LMapDOMNode>(EDOMNodeType::Map);
	if (auto mapNodeLocked = mapNode.lock())
	{
		auto rooms = mapNodeLocked->GetChildrenOfType<LRoomDOMNode>(EDOMNodeType::Room);
		for (auto r : rooms)
		{
			auto roomData = r->GetChildrenOfType<LRoomDataDOMNode>(EDOMNodeType::RoomData)[0];

			if(Children.empty() || Children[0].get() == nullptr || Children[0]->GetNodeType() != EDOMNodeType::PathPoint) continue;
			if (roomData->CheckPointInBounds(std::static_pointer_cast<LPathPointDOMNode>(Children[0])->GetPosition()))
			{
				r->AddChild(GetSharedPtr<LPathDOMNode>(EDOMNodeType::Path));
				break;
			}
		}
	}
}

void LPathDOMNode::PreProcess(LJmpIO& pathJmp, bStream::CMemoryStream& pathStream)
{
	auto points = GetChildrenOfType<LEntityDOMNode>(EDOMNodeType::PathPoint);
	for (auto p : points)
		p->PreProcess();

	pathJmp.Save(points, pathStream);
}
