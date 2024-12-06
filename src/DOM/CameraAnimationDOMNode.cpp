
#include "DOM/CameraAnimationDOMNode.hpp"
#include "imgui.h"
#include <glad/glad.h>
#include "misc/cpp/imgui_stdlib.h"
#include "imgui_neo_internal.h"
#include "imgui_neo_sequencer.h"

static bool mAnimPlayerReady { false };
static uint32_t mPreviewFbo { 0 }, mPreviewRbo { 0 }, mPreviewTex { 0 };

static glm::vec3 mEye, mCenter;
static float mFovY { 90.0f }; 
static bool Active { false };

namespace CameraAnimation {
    void SetPreviewActive(){
        Active = true;
    }

    void SetPreviewInactive(){
        Active = false;
    }

    bool GetPreviewActive(){
        return Active;
    }


    void InitPreview(){
		glGenFramebuffers(1, &mPreviewFbo);
		glBindFramebuffer(GL_FRAMEBUFFER, mPreviewFbo);

		glGenRenderbuffers(1, &mPreviewRbo);
		glBindRenderbuffer(GL_RENDERBUFFER, mPreviewRbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 640, 480);

		glGenTextures(1, &mPreviewTex);

		glBindTexture(GL_TEXTURE_2D, mPreviewTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 640, 480, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mPreviewTex, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mPreviewRbo);

		assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);


		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }

    void CleanupPreview(){
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        glDeleteFramebuffers(1, &mPreviewFbo);
        glDeleteRenderbuffers(1, &mPreviewRbo);
        glDeleteTextures(1, &mPreviewTex);
    }

    void RenderPreview(){
        if(Active){
            glBindFramebuffer(GL_FRAMEBUFFER, mPreviewFbo);
            glBindRenderbuffer(GL_RENDERBUFFER, mPreviewRbo);

            glViewport(0, 0, 640, 480);
            glClearColor(0.100f, 0.261f, 0.402f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            auto scene = LEditorScene::GetEditorScene();
            auto eye = scene->Camera.GetEye();
            auto center = scene->Camera.GetCenter();
            auto fov = scene->Camera.Fovy;
            auto mode = scene->Camera.mCamMode;
            scene->Camera.mCamMode = ECamMode::ANIMATION;

            scene->Camera.SetEye(mEye);
            scene->Camera.SetCenter(mCenter);
            scene->Camera.Fovy = mFovY;

            scene->Camera.UnRotate();
            scene->RenderSubmit(640, 480);
            scene->Camera.ReRotate();

            scene->Camera.mCamMode = mode;
            scene->Camera.SetEye(eye);
            scene->Camera.SetCenter(center);
            scene->Camera.Fovy = fov;

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
        }
    }
    
}

LCameraAnimationDOMNode::LCameraAnimationDOMNode(std::string name) : Super(name)
{
	mType = EDOMNodeType::CameraAnim;

    mNextPosKeyX = 1;
    mNextPosKeyY = 1;
    mNextPosKeyZ = 1;

    mNextTargetKeyX = 1;
    mNextTargetKeyY = 1;
    mNextTargetKeyZ = 1;

    mNextFovKey = 1;

}

void LCameraAnimationDOMNode::RenderDetailsUI(float dt, LSceneCamera* camera)
{
    ImGui::Image(static_cast<uintptr_t>(mPreviewTex), ImVec2(640, 480), {0.0f, 1.0f}, {1.0f, 0.0f});

    if(ImGui::Button(">"))
    {
        mPlaying = true;
        mCurrentFrame = 0;
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
    }

    if(mPlaying)
    {

        if(mCurrentFrame++ > mFrameCount)
        {
            mPlaying = false;
            mCurrentFrame = 0;
            mNextPosKeyX = 1;
            mNextPosKeyY = 1;
            mNextPosKeyZ = 1;
            mNextTargetKeyX = 1;
            mNextTargetKeyY = 1;
            mNextTargetKeyZ = 1;
            mNextFovKey = 1;
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
        }
    } else {
        if(mCurrentFrame >= mPosFramesX.mKeys[mNextPosKeyX]) 
        {
            mNextPosKeyX++;
        } else if(mCurrentFrame < mPosFramesX.mKeys[mNextPosKeyX-1]){
            mNextPosKeyX--;
        }

        if(mCurrentFrame >= mPosFramesY.mKeys[mNextPosKeyY]) 
        {
            mNextPosKeyY++;
        } else if(mCurrentFrame < mPosFramesY.mKeys[mNextPosKeyY-1]){
            mNextPosKeyY--;
        }

        if(mCurrentFrame >= mPosFramesZ.mKeys[mNextPosKeyZ]) 
        {
            mNextPosKeyZ++;
        } else if(mCurrentFrame < mPosFramesZ.mKeys[mNextPosKeyZ-1]){
            mNextPosKeyZ--;
        }

        if(mCurrentFrame >= mTargetFramesX.mKeys[mNextTargetKeyX]) 
        {
            mNextTargetKeyX++;
        } else if(mCurrentFrame < mTargetFramesX.mKeys[mNextTargetKeyX-1]){
            mNextTargetKeyX--;
        }

        if(mCurrentFrame >= mTargetFramesY.mKeys[mNextTargetKeyY]) 
        {
            mNextTargetKeyY++;
        } else if(mCurrentFrame < mTargetFramesY.mKeys[mNextTargetKeyY-1]){
            mNextTargetKeyY--;
        }

        if(mCurrentFrame >= mTargetFramesZ.mKeys[mNextTargetKeyZ]) 
        {
            mNextTargetKeyZ++;
        } else if(mCurrentFrame < mTargetFramesZ.mKeys[mNextTargetKeyZ-1]){
            mNextTargetKeyZ--;
        }

        if(mCurrentFrame >= mFovFrames.mKeys[mNextFovKey]) 
        {
            mNextFovKey++;
        } else if(mCurrentFrame < mFovFrames.mKeys[mNextFovKey-1]){
            mNextFovKey--;
        }
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
        mFovY = glm::radians(glm::mix(mFovFrames.mFrames[mFovFrames.mKeys[mNextFovKey - 1]].value, mFovFrames.mFrames[mFovFrames.mKeys[mNextFovKey]].value, (mCurrentFrame - mFovFrames.mFrames[mFovFrames.mKeys[mNextFovKey - 1]].frame) / (mFovFrames.mFrames[mFovFrames.mKeys[mNextFovKey]].frame - mFovFrames.mFrames[mFovFrames.mKeys[mNextFovKey - 1]].frame)));
    }

        
    mEye = eyePos;
    mCenter = targetPos;


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

    //LGenUtility::Log << "==Reading Track Pos X==" << std::endl;
    mPosFramesZ.LoadTrack(stream, 68, ETrackType::CMN);
    //LGenUtility::Log << "==Reading Track Pos Y==" << std::endl;
    mPosFramesY.LoadTrack(stream, 68, ETrackType::CMN);
    //LGenUtility::Log << "==Reading Track Pos Z==" << std::endl;
    mPosFramesX.LoadTrack(stream, 68, ETrackType::CMN);

    //LGenUtility::Log << "==Reading Track Target X==" << std::endl;
    mTargetFramesZ.LoadTrack(stream, 68, ETrackType::CMN);
    //LGenUtility::Log << "==Reading Track Target Y==" << std::endl;
    mTargetFramesY.LoadTrack(stream, 68, ETrackType::CMN);
    //LGenUtility::Log << "==Reading Track Target Z==" << std::endl;
    mTargetFramesX.LoadTrack(stream, 68, ETrackType::CMN);
    
    //LGenUtility::Log << "==Reading Track Unk Data==" << std::endl;
    mUnkownDataFrames.LoadTrack(stream, 68, ETrackType::CMN);

    //LGenUtility::Log << "==Reading Track FOV==" << std::endl;
    mFovFrames.LoadTrack(stream, 68, ETrackType::CMN);

    //LGenUtility::Log << "==Reading Track ZNear==" << std::endl;
    mZNearFrames.LoadTrack(stream, 68, ETrackType::CMN);
    
    //LGenUtility::Log << "==Reading Track ZFar==" << std::endl;
    mZFarFrames.LoadTrack(stream, 68, ETrackType::CMN);

    mUnknownFloat = stream->readFloat();

}