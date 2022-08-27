
#include "DOM/CameraAnimationDOMNode.hpp"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "imgui_neo_internal.h"
#include "imgui_neo_sequencer.h"

LCameraAnimationDOMNode::LCameraAnimationDOMNode(std::string name) : Super(name)
{
	mType = EDOMNodeType::CameraAnim;
}

void LCameraAnimationDOMNode::RenderDetailsUI(float dt, LSceneCamera* camera)
{
    if(ImGui::Button(">"))
    {
        mPlaying = true;
        camera->mCamMode = ANIMATION;

        camera->SetEye(glm::vec3(mPosFramesX.mFrames[mPosFramesX.mKeys[0]].value, mPosFramesY.mFrames[mPosFramesY.mKeys[0]].value, mPosFramesZ.mFrames[mPosFramesZ.mKeys[0]].value));
        camera->SetCenter(glm::vec3(mTargetFramesX.mFrames[mTargetFramesX.mKeys[0]].value, mTargetFramesY.mFrames[mTargetFramesY.mKeys[0]].value, mTargetFramesZ.mFrames[mTargetFramesZ.mKeys[0]].value));
        mCurrentFrame = 0;

        //camera->Fovy = glm::radians(mFovFrames.mFrames[mFovFrames.mKeys[0]].value);

        mNextPosKeyX = 1;
        mNextPosKeyY = 1;
        mNextPosKeyZ = 1;

        mNextTargetKeyX = 1;
        mNextTargetKeyY = 1;
        mNextTargetKeyZ = 1;

        mNextFovKey = 1;
    }

    ImGui::SameLine();

    if(ImGui::Button("||")){
        mPlaying = false;
        camera->mCamMode = FLY;
    }

    if(mPlaying)
    {
        if(camera->mCamMode != ANIMATION)
        {
            camera->mCamMode = ANIMATION;
        }


        if(mCurrentFrame++ > mFrameCount)
        {
            mPlaying = false;
        } 
        else 
        {
            if(mCurrentFrame == mPosFramesX.mKeys[mNextPosKeyX]) 
            {
                mNextPosKeyX++;
            }

            if(mCurrentFrame == mPosFramesY.mKeys[mNextPosKeyY]) 
            {
                mNextPosKeyY++;
            }

            if(mCurrentFrame == mPosFramesZ.mKeys[mNextPosKeyZ]) 
            {
                mNextPosKeyZ++;
            }

            if(mCurrentFrame == mTargetFramesX.mKeys[mNextTargetKeyX]) 
            {
                mNextTargetKeyX++;
            }

            if(mCurrentFrame == mTargetFramesY.mKeys[mNextTargetKeyY]) 
            {
                mNextTargetKeyY++;
            }

            if(mCurrentFrame == mTargetFramesZ.mKeys[mNextTargetKeyZ]) 
            {
                mNextTargetKeyZ++;
            }

            if(mCurrentFrame == mFovFrames.mKeys[mNextFovKey]) 
            {
                mNextFovKey++;
            }

            glm::vec3 eyePos = camera->GetEye();

            if(mNextPosKeyX < mPosFramesX.mKeys.size()){
                eyePos.x = glm::mix(mPosFramesX.mFrames[mPosFramesX.mKeys[mNextPosKeyX - 1]].value, mPosFramesX.mFrames[mPosFramesX.mKeys[mNextPosKeyX]].value, (mCurrentFrame - mPosFramesX.mFrames[mPosFramesX.mKeys[mNextPosKeyX - 1]].frame) / (mPosFramesX.mFrames[mPosFramesX.mKeys[mNextPosKeyX]].frame - mPosFramesX.mFrames[mPosFramesX.mKeys[mNextPosKeyX - 1]].frame));
            }

            if(mNextPosKeyY < mPosFramesY.mKeys.size()){
                eyePos.y = glm::mix(mPosFramesY.mFrames[mPosFramesY.mKeys[mNextPosKeyY - 1]].value, mPosFramesY.mFrames[mPosFramesY.mKeys[mNextPosKeyY]].value, (mCurrentFrame - mPosFramesY.mFrames[mPosFramesY.mKeys[mNextPosKeyY - 1]].frame) / (mPosFramesY.mFrames[mPosFramesY.mKeys[mNextPosKeyY]].frame - mPosFramesY.mFrames[mPosFramesY.mKeys[mNextPosKeyY - 1]].frame));
            }

            if(mNextPosKeyZ < mPosFramesZ.mKeys.size()){
                eyePos.z = glm::mix(mPosFramesZ.mFrames[mPosFramesZ.mKeys[mNextPosKeyZ - 1]].value, mPosFramesZ.mFrames[mPosFramesZ.mKeys[mNextPosKeyZ]].value, (mCurrentFrame - mPosFramesZ.mFrames[mPosFramesZ.mKeys[mNextPosKeyZ - 1]].frame) / (mPosFramesZ.mFrames[mPosFramesZ.mKeys[mNextPosKeyZ]].frame - mPosFramesZ.mFrames[mPosFramesZ.mKeys[mNextPosKeyZ - 1]].frame));
            }

            glm::vec3 targetPos = camera->GetCenter();
            
            if(mNextTargetKeyX < mTargetFramesX.mKeys.size()){
                targetPos.x = glm::mix(mTargetFramesX.mFrames[mTargetFramesX.mKeys[mNextTargetKeyX - 1]].value, mTargetFramesX.mFrames[mTargetFramesX.mKeys[mNextTargetKeyX]].value, (mCurrentFrame - mTargetFramesX.mFrames[mTargetFramesX.mKeys[mNextTargetKeyX - 1]].frame) / (mTargetFramesX.mFrames[mTargetFramesX.mKeys[mNextTargetKeyX]].frame - mTargetFramesX.mFrames[mTargetFramesX.mKeys[mNextTargetKeyX - 1]].frame));
            }

            if(mNextTargetKeyY < mTargetFramesY.mKeys.size()){
                targetPos.y = glm::mix(mTargetFramesY.mFrames[mTargetFramesY.mKeys[mNextTargetKeyY - 1]].value, mTargetFramesY.mFrames[mTargetFramesY.mKeys[mNextTargetKeyY]].value, (mCurrentFrame - mTargetFramesY.mFrames[mTargetFramesY.mKeys[mNextTargetKeyY - 1]].frame) / (mTargetFramesY.mFrames[mTargetFramesY.mKeys[mNextTargetKeyY]].frame - mTargetFramesY.mFrames[mTargetFramesY.mKeys[mNextTargetKeyY - 1]].frame));
            }

            if(mNextTargetKeyZ < mTargetFramesZ.mKeys.size()){
                targetPos.z = glm::mix(mTargetFramesZ.mFrames[mTargetFramesZ.mKeys[mNextTargetKeyZ - 1]].value, mTargetFramesZ.mFrames[mTargetFramesZ.mKeys[mNextTargetKeyZ]].value, (mCurrentFrame - mTargetFramesZ.mFrames[mTargetFramesZ.mKeys[mNextTargetKeyZ - 1]].frame) / (mTargetFramesZ.mFrames[mTargetFramesZ.mKeys[mNextTargetKeyZ]].frame - mTargetFramesZ.mFrames[mTargetFramesZ.mKeys[mNextTargetKeyZ - 1]].frame));
            }

            if(mNextFovKey < mFovFrames.mKeys.size()){
                camera->Fovy = glm::radians(glm::mix(mFovFrames.mFrames[mFovFrames.mKeys[mNextFovKey - 1]].value, mFovFrames.mFrames[mFovFrames.mKeys[mNextFovKey]].value, (mCurrentFrame - mFovFrames.mFrames[mFovFrames.mKeys[mNextFovKey - 1]].frame) / (mFovFrames.mFrames[mFovFrames.mKeys[mNextFovKey]].frame - mFovFrames.mFrames[mFovFrames.mKeys[mNextFovKey - 1]].frame)));
            }

            //std::cout << eyePos.x << "," << eyePos.y << "," << eyePos.z << " | " << targetPos.x << "," << targetPos.y << "," << targetPos.z << std::endl;

            camera->SetCenter(targetPos);
            camera->SetEye(eyePos);
        }

    }

    if(ImGui::BeginNeoSequencer("Camera Animation", &mCurrentFrame, &mStartFrame, &mFrameCount))
    {
        if(ImGui::BeginNeoGroup("Camera Position", &mCameraPosCollapsed)){
            if(ImGui::BeginNeoTimeline("\tX", mPosFramesX.mKeys)){
                ImGui::EndNeoTimeLine();
            }
            if(ImGui::BeginNeoTimeline("\tY", mPosFramesY.mKeys)){
                ImGui::EndNeoTimeLine();
            }
            if(ImGui::BeginNeoTimeline("\tZ", mPosFramesZ.mKeys)){
                ImGui::EndNeoTimeLine();
            }
            ImGui::EndNeoGroup();
        }


        if(ImGui::BeginNeoTimeline("Target Position X", mTargetFramesX.mKeys)){
            ImGui::EndNeoTimeLine();
        }
        if(ImGui::BeginNeoTimeline("Target Position Y", mTargetFramesY.mKeys)){
            ImGui::EndNeoTimeLine();
        }
        if(ImGui::BeginNeoTimeline("Target Position Z", mTargetFramesZ.mKeys)){
            ImGui::EndNeoTimeLine();
        }


        if(ImGui::BeginNeoTimeline("Camera FOV", mFovFrames.mKeys)){
            ImGui::EndNeoTimeLine();
        }

        if(ImGui::BeginNeoTimeline("Near Frame", mZNearFrames.mKeys)){
            ImGui::EndNeoTimeLine();
        }
        if(ImGui::BeginNeoTimeline("Far Plane", mZFarFrames.mKeys)){
            ImGui::EndNeoTimeLine();
        }

        if(ImGui::BeginNeoTimeline("Unknown Data", mUnkownDataFrames.mKeys)){
            ImGui::EndNeoTimeLine();
        }

        ImGui::EndNeoSequencer();
    }
    //TODO: edit keyframe data
}

void LCameraAnimationDOMNode::Load(bStream::CStream* stream)
{
    mPlaying = false;
    mCurrentFrame = 0;
    mStartFrame = 0;

    mFrameCount = stream->readUInt16();
    
    stream->readUInt16();

    // Frame Data for Camera Animations always starts at offset 68

    //std::cout << "==Reading Track Pos X==" << std::endl;
    mPosFramesZ.LoadTrack(stream, 68, ETrackType::CMN);
    //std::cout << "==Reading Track Pos Y==" << std::endl;
    mPosFramesY.LoadTrack(stream, 68, ETrackType::CMN);
    //std::cout << "==Reading Track Pos Z==" << std::endl;
    mPosFramesX.LoadTrack(stream, 68, ETrackType::CMN);

    //std::cout << "==Reading Track Target X==" << std::endl;
    mTargetFramesZ.LoadTrack(stream, 68, ETrackType::CMN);
    //std::cout << "==Reading Track Target Y==" << std::endl;
    mTargetFramesY.LoadTrack(stream, 68, ETrackType::CMN);
    //std::cout << "==Reading Track Target Z==" << std::endl;
    mTargetFramesX.LoadTrack(stream, 68, ETrackType::CMN);
    
    //std::cout << "==Reading Track Unk Data==" << std::endl;
    mUnkownDataFrames.LoadTrack(stream, 68, ETrackType::CMN);

    //std::cout << "==Reading Track FOV==" << std::endl;
    mFovFrames.LoadTrack(stream, 68, ETrackType::CMN);

    //std::cout << "==Reading Track ZNear==" << std::endl;
    mZNearFrames.LoadTrack(stream, 68, ETrackType::CMN);
    
    //std::cout << "==Reading Track ZFar==" << std::endl;
    mZFarFrames.LoadTrack(stream, 68, ETrackType::CMN);

    mUnknownFloat = stream->readFloat();

}