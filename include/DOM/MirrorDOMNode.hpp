#pragma once

#include "BGRenderDOMNode.hpp"

class LMirrorDOMNode : public LBGRenderDOMNode
{
	
	int32_t mCameraHeightOffset;
	
	uint16_t mResolutionWidth;
	uint16_t mResolutionHeight;
	
	float mZoom;

	bool mGBHOnly; //u32

	float mUnkValue1;

public:
	typedef LBGRenderDOMNode Super;

	LMirrorDOMNode(std::string name);

	virtual std::string GetName() override;
	virtual void RenderDetailsUI(float dt) override;

	/*
	bool Load(const LStaticDoorData& source);
	bool Save(LStaticDoorData& dest);
	*/
	
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
