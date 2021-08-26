#include "DOM\ItemAppearDOMNode.hpp"
#include "DOM\ItemInfoDOMNode.hpp"
#include "UIUtil.hpp"
#include "GenUtil.hpp"
#include <algorithm>

LItemAppearDOMNode::LItemAppearDOMNode(std::string name) : Super(name)
{
    std::fill_n(mItemNames, 20, "nothing");
    std::fill_n(mItemInfoRefs, 20, std::weak_ptr<LItemInfoDOMNode>());

    mType = EDOMNodeType::ItemAppear;
    mRoomNumber = -1;
}

void LItemAppearDOMNode::RenderDetailsUI(float dt)
{
    for (uint32_t i = 0; i < 20; i++)
    {
        ImGui::PushID(i);
        LUIUtility::RenderNodeReferenceCombo<LItemInfoDOMNode>(LGenUtility::Format("Item Slot ", i), EDOMNodeType::Furniture, Parent, mItemInfoRefs[i]);
        ImGui::PopID();
    }
}

void LItemAppearDOMNode::Serialize(LJmpIO* JmpIO, uint32_t entry_index) const
{
    for (uint32_t i = 0; i < 20; i++)
    {
        JmpIO->SetString(entry_index, LGenUtility::Format("item", i), mItemNames[i]);
    }
}

void LItemAppearDOMNode::Deserialize(LJmpIO* JmpIO, uint32_t entry_index)
{
    for (uint32_t i = 0; i < 20; i++)
    {
        mItemNames[i] = JmpIO->GetString(entry_index, LGenUtility::Format("item", i));
    }
}

void LItemAppearDOMNode::PostProcess()
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

        for (uint32_t i = 0; i < 20; i++)
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

void LItemAppearDOMNode::PreProcess()
{
    // On the off chance that the parent is invalid, don't try to do anything.
    if (Parent.expired())
        return;

    // Grab a temporary shared_ptr for the parent.
    auto parentShared = Parent.lock();

    for (uint32_t i = 0; i < 20; i++)
    {
        if (auto info = mItemInfoRefs[i].lock())
            mItemNames[i] = info->GetName();
        else
            mItemNames[i] = "nothing";
    }
}
