#include "DOM/ItemAppearDOMNode.hpp"
#include "DOM/ItemInfoDOMNode.hpp"
#include "UIUtil.hpp"
#include "GenUtil.hpp"
#include <algorithm>
#include <map>

LItemAppearDOMNode::LItemAppearDOMNode(std::string name) : Super(name)
{
    std::fill_n(mItemNames, 20, "nothing");
    std::fill_n(mItemInfoRefs, 20, std::weak_ptr<LItemInfoDOMNode>());

    mType = EDOMNodeType::ItemAppear;
    mRoomNumber = -1;
}

std::string LItemAppearDOMNode::GetName()
{
    std::map<std::string, uint32_t> stats;

    for (auto reference : mItemInfoRefs)
    {
        if (reference.expired())
        {
            stats.try_emplace("nothing", 0);
            stats["nothing"] += 1;

            continue;
        }

        auto refLocked = reference.lock();

        stats.try_emplace(refLocked->GetName(), 0);
        stats[refLocked->GetName()] += 1;
    }

    std::string result = "";

    for (auto [name, count] : stats)
    {
        if (result != "")
            result += ", ";

        result = fmt::format("{0}{1} (x{2})", result, name, count);
    }

    return result;
}

void LItemAppearDOMNode::RenderDetailsUI(float dt)
{
    for (uint32_t i = 0; i < 20; i++)
    {
        ImGui::PushID(i);
        LUIUtility::RenderNodeReferenceCombo<LItemInfoDOMNode>(fmt::format("Item Slot {0}", i), EDOMNodeType::ItemInfo, Parent, mItemInfoRefs[i]);
        ImGui::PopID();
    }
}

void LItemAppearDOMNode::Serialize(LJmpIO* JmpIO, uint32_t entry_index) const
{
    for (uint32_t i = 0; i < 20; i++)
    {
        JmpIO->SetString(entry_index, fmt::format("item{0}", i), mItemNames[i]);
    }
}

void LItemAppearDOMNode::Deserialize(LJmpIO* JmpIO, uint32_t entry_index)
{
    for (uint32_t i = 0; i < 20; i++)
    {
        mItemNames[i] = JmpIO->GetString(entry_index, fmt::format("item{0}", i));
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
