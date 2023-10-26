#include "DOM/ItemFishingDOMNode.hpp"
#include "DOM/ItemInfoDOMNode.hpp"
#include "DOM/MapDOMNode.hpp"
#include "GenUtil.hpp"
#include "UIUtil.hpp"

LItemFishingDOMNode::LItemFishingDOMNode(std::string name) : Super(name)
{
	std::fill_n(mItemNames, 60, "-");
	std::fill_n(mItemInfoRefs, 60, std::weak_ptr<LItemInfoDOMNode>());

	mType = EDOMNodeType::ItemFishing;
	mRoomNumber = -1;
}

void LItemFishingDOMNode::RenderDetailsUI(float dt)
{
    for (uint32_t i = 0; i < 60; i++)
    {
        ImGui::PushID(i);
        LUIUtility::RenderNodeReferenceCombo<LItemInfoDOMNode>(fmt::format("Item Slot {0}", i), EDOMNodeType::Furniture, Parent, mItemInfoRefs[i]);
        ImGui::PopID();
    }
}

void LItemFishingDOMNode::Serialize(LJmpIO* JmpIO, uint32_t entry_index) const
{
    for (uint32_t i = 0; i < 60; i++)
    {
        JmpIO->SetString(entry_index, fmt::format("item{0}", i), mItemNames[i]);
    }
}

void LItemFishingDOMNode::Deserialize(LJmpIO* JmpIO, uint32_t entry_index)
{
    for (uint32_t i = 0; i < 60; i++)
    {
        mItemNames[i] = JmpIO->GetString(entry_index, fmt::format("item{0}", i));
    }
}

void LItemFishingDOMNode::PostProcess()
{
    // On the off chance that the parent is invalid, don't try to do anything.
    if (Parent.expired())
        return;

    // Grab a temporary shared_ptr for the parent.
    auto parentShared = Parent.lock();

    // Grab item info data from the map
    auto mapNode = GetParentOfType<LMapDOMNode>(EDOMNodeType::Map);
    if (auto mapNodeLocked = mapNode.lock())
    {
        auto itemInfos = mapNodeLocked->GetChildrenOfType<LItemInfoDOMNode>(EDOMNodeType::ItemInfo);

        for (uint32_t i = 0; i < 60; i++)
        {
            for (auto info : itemInfos)
            {
                if (info->GetName() == mItemNames[i])
                {
                    mItemInfoRefs[i] = info;
                    break;
                }
            }
        }
    }
}

void LItemFishingDOMNode::PreProcess()
{
    // On the off chance that the parent is invalid, don't try to do anything.
    if (Parent.expired())
        return;

    // Grab a temporary shared_ptr for the parent.
    auto parentShared = Parent.lock();

    for (uint32_t i = 0; i < 60; i++)
    {
        if (auto info = mItemInfoRefs[i].lock())
            mItemNames[i] = info->GetName();
        else
            mItemNames[i] = "-";
    }
}
