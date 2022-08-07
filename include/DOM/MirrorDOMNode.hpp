#pragma once

#include "BGRenderDOMNode.hpp"
#include <nlohmann/json.hpp>

class LMirrorDOMNode : public LBGRenderDOMNode
{
	
	int32_t mCameraHeightOffset;
	
	int32_t mResolutionWidth;
	int32_t mResolutionHeight;
	
	float mZoom;

	bool mGBHOnly; //u32

	float mCameraDistance;

public:
	typedef LBGRenderDOMNode Super;

	LMirrorDOMNode(std::string name);

	virtual std::string GetName() override;
	virtual void RenderDetailsUI(float dt) override;

	/*
	bool Load(const LStaticDoorData& source);
	bool Save(LStaticDoorData& dest);
	*/
	
	bool Load(const nlohmann::ordered_json& jsonEntry);

	void PostProcess();
	void PreProcess();


/*=== Type operations ===*/
	// Returns whether this node is of the given type, or derives from a node of that type.
	virtual bool IsNodeType(EDOMNodeType type) const override
	{
		if (type == EDOMNodeType::Mirror)
			return true;

		return Super::IsNodeType(type);
	}
};
