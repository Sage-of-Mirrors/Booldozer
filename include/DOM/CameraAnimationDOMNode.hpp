#pragma once

#include "io/KeyframeIO.hpp"
#include "BGRenderDOMNode.hpp"
#include "scene/Camera.hpp"
#include "scene/EditorScene.hpp"

namespace CameraAnimation {
    void CleanupPreview();
    void RenderPreview();
    void InitPreview();
    void SetPreviewActive();
    void SetPreviewInactive();
    bool GetPreviewActive();
}

class LCameraAnimationDOMNode : public LBGRenderDOMNode
{
private:

    bool mPlaying;
    float mTime { 0.0f };
    int32_t mCurrentFrame { 0 };

    int32_t mStartFrame { 0 };
    int32_t mFrameCount { 0 };

    uint32_t mPrevPosKeyX { 0 };
    uint32_t mPrevPosKeyY { 0 };
    uint32_t mPrevPosKeyZ { 0 };

    uint32_t mNextPosKeyX { 1 };
    uint32_t mNextPosKeyY { 1 };
    uint32_t mNextPosKeyZ { 1 };

    uint32_t mPrevTargetKeyX { 0 };
    uint32_t mPrevTargetKeyY { 0 };
    uint32_t mPrevTargetKeyZ { 0 };

    uint32_t mNextTargetKeyX { 1 };
    uint32_t mNextTargetKeyY { 1 };
    uint32_t mNextTargetKeyZ { 1 };

    uint32_t mPrevFovKey { 0 };
    uint32_t mNextFovKey { 1 };

    LTrackCommon mPosFramesX;
    LTrackCommon mPosFramesY;
    LTrackCommon mPosFramesZ;

    LTrackCommon mTargetFramesX;
    LTrackCommon mTargetFramesY;
    LTrackCommon mTargetFramesZ;

    LTrackCommon mUnkownDataFrames;

    LTrackCommon mFovFrames;

    LTrackCommon mZNearFrames;
    LTrackCommon mZFarFrames;

    float mUnknownFloat;

    bool mCameraPosCollapsed;

public:

	typedef LBGRenderDOMNode Super;

	LCameraAnimationDOMNode(std::string name);

    void Load(bStream::CStream* stream);

    void RenderDetailsUI(float dt, LSceneCamera* camera);

/*=== Type operations ===*/
	// Returns whether this node is of the given type, or derives from a node of that type.
	virtual bool IsNodeType(EDOMNodeType type) const override
	{
		if (type == EDOMNodeType::CameraAnim)
			return true;

		return Super::IsNodeType(type);
	}
};
