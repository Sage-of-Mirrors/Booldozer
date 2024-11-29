#pragma once

#include "BGRenderDOMNode.hpp"
#include <json.hpp>

namespace bStream
{
	class CFileStream;
	class CMemoryStream;
}

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

	int32_t GetResWidth() { return mResolutionWidth; }
	int32_t GetResHeight() { return mResolutionHeight; }

	virtual std::string GetName() override;
	virtual void RenderDetailsUI(float dt) override;

	bool Load(bStream::CFileStream* stream);
	bool Save(bStream::CMemoryStream* stream);
	
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
