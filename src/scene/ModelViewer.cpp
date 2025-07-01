
#include "scene/ModelViewer.hpp"
#include "scene/Camera.hpp"
#include "imgui.h"
#include <glad/glad.h>
#include "misc/cpp/imgui_stdlib.h"
#include "ImGuizmo.h"

#include "glm/matrix.hpp"
#include "glm/ext/matrix_transform.hpp"

#include <J3D/Material/J3DUniformBufferObject.hpp>
#include <J3D/J3DModelLoader.hpp>
#include <J3D/Rendering/J3DRendering.hpp>
#include "io/BinIO.hpp"
#include "io/MdlIO.hpp"
#include "UGrid.hpp"

namespace PreviewWidget {
    static bool Ready { false };
    static uint32_t Fbo { 0 }, Rbo { 0 }, Tex { 0 };

    static bool Active { false };
    static LSceneCamera Camera;
    static glm::mat4 Identity { 1.0 };

    static float Rotate { -155.0f };
    static float Zoom { 430.0f };

    static EModelType CurrentEModelType { EModelType::None };
    static BIN::Model* ModelFurniture { nullptr };
    static MDL::Model* ModelActor { nullptr };
    static TXP::Animation* ActorTxp { nullptr };
    static UGrid* Grid { nullptr };

    BIN::Model* GetFurnitureModel(){
        return ModelFurniture;
    }

    void NewFurnitureModel(){
        delete ModelFurniture;
        ModelFurniture = new BIN::Model();
    }

    void SetActive(){
        Active = true;
        Rotate = -155.0f;
        Zoom = 430.0f;
    }

    void PlayAnimation(){
        if(CurrentEModelType == EModelType::Furniture && ModelFurniture != nullptr){
            ModelFurniture->mAnim.mPlaying = true;
            ModelFurniture->mAnim.mCurrentFrame = 0.0f;
            //ModelFurniture->ResetAnimation();
        }
    }

    void PauseAnimation(){
        if(CurrentEModelType == EModelType::Furniture && ModelFurniture != nullptr){
            ModelFurniture->mAnim.mPlaying = false;
        }
    }

    void SetInactive(){
        Active = false;
    }

    bool GetActive(){
        return Active;
    }

    uint32_t PreviewID(){
        return Tex;
    }

    void DoRotate(float amt){
        Rotate += amt;
    }

    void DoZoom(float amt){
        Zoom += amt;
    }

    void InitPreview(){
        Grid = new UGrid();
        Grid->Init();

		glGenFramebuffers(1, &Fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, Fbo);

		glGenRenderbuffers(1, &Rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, Rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 500, 500);

		glGenTextures(1, &Tex);

		glBindTexture(GL_TEXTURE_2D, Tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 500, 500, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Tex, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, Rbo);

		assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);


		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

    }

    void CleanupPreview(){
        delete Grid;
        if(ModelFurniture != nullptr) delete ModelFurniture;
        if(ModelActor != nullptr) delete ModelActor;
        if(ActorTxp != nullptr) delete ActorTxp;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        glDeleteFramebuffers(1, &Fbo);
        glDeleteRenderbuffers(1, &Rbo);
        glDeleteTextures(1, &Tex);
    }

    void LoadModel(bStream::CMemoryStream* ModelStream, EModelType Type){
        if(ModelFurniture != nullptr){
            delete ModelFurniture;
            ModelFurniture = nullptr;
        }
        if(ModelActor != nullptr){
            delete ModelActor;
            ModelActor = nullptr;
        }
        if(Type == EModelType::Furniture){
            ModelFurniture = new BIN::Model();
            ModelFurniture->Load(ModelStream);
            ModelFurniture->mAnim.mCurrentFrame = 0;
            //ModelFurniture->ResetAnimation();
            Camera.SetCenter(ModelFurniture->mGraphNodes[0].Position);
            CurrentEModelType = Type;
        } else if(Type == EModelType::Actor){
            ModelActor = new MDL::Model();
            ModelActor->Load(ModelStream);
            Camera.SetCenter(glm::vec3(0,0,0));
            CurrentEModelType = Type;
        }
    }

    void SaveModel(bStream::CMemoryStream* ModelStream){
        if(ModelFurniture != nullptr){
            ModelFurniture->Write(ModelStream);
        }
    }

    void SetModelAnimation(bStream::CMemoryStream* AnimStream){
        if(CurrentEModelType == EModelType::Furniture && ModelFurniture != nullptr){
            ModelFurniture->LoadAnimation(AnimStream);
            //ModelFurniture->ResetAnimation();
        } else if(CurrentEModelType == EModelType::Actor && ModelActor != nullptr){
            ActorTxp = new TXP::Animation();
            ActorTxp->Load(AnimStream);
        }
    }

    void UnloadModel(){
        if(ModelFurniture != nullptr){
            delete ModelFurniture;
            ModelFurniture = nullptr;
        }
        if(ModelActor != nullptr){
            delete ModelActor;
            ModelActor = nullptr;
        }

        if(ActorTxp != nullptr){
            delete ActorTxp;
            ActorTxp = nullptr;
        }

        CurrentEModelType = EModelType::None;
    }

    void RenderPreview(){
        if(Active){

            glDisable(GL_CULL_FACE);

            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


            glBindFramebuffer(GL_FRAMEBUFFER, Fbo);
            glBindRenderbuffer(GL_RENDERBUFFER, Rbo);

            glViewport(0, 0, 500, 500);
            glClearColor(0.100f, 0.261f, 0.402f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


            // TODO: Draw Grid somehow

            if(CurrentEModelType == EModelType::Furniture && ModelFurniture != nullptr){
                Camera.SetEye({ModelFurniture->mGraphNodes[0].Position.x + (sin(Rotate) * (Zoom * 2)), ModelFurniture->mGraphNodes[0].Position.y + Zoom, ModelFurniture->mGraphNodes[0].Position.z - (cos(Rotate) * (Zoom * 2))});

                // Ow.
	            J3DUniformBufferObject::SetProjAndViewMatrices(Camera.GetProjectionMatrix(), Camera.GetViewMatrix());
	            J3DUniformBufferObject::SubmitUBO();

                ModelFurniture->Draw(&Identity, 0, false);
                Grid->Render({ModelFurniture->mGraphNodes[0].Position.x + (sin(Rotate) * (Zoom * 2)), ModelFurniture->mGraphNodes[0].Position.y + Zoom, ModelFurniture->mGraphNodes[0].Position.z - (cos(Rotate) * (Zoom * 2))}, Camera.GetProjectionMatrix(), Camera.GetViewMatrix());
            } else if(CurrentEModelType == EModelType::Actor && ModelActor != nullptr) {
                Camera.SetEye({sin(Rotate) * (Zoom * 2), Zoom, cos(Rotate) * (Zoom * 2)});


	            J3DUniformBufferObject::SetProjAndViewMatrices(Camera.GetProjectionMatrix(), Camera.GetViewMatrix());
	            J3DUniformBufferObject::SubmitUBO();

                ModelActor->Draw(&Identity, 0, false, ActorTxp);
                Grid->Render({(sin(Rotate) * (Zoom * 2)), Zoom, (cos(Rotate) * (Zoom * 2))}, Camera.GetProjectionMatrix(), Camera.GetViewMatrix());
            } else {
	            J3DUniformBufferObject::SetProjAndViewMatrices(Camera.GetProjectionMatrix(), Camera.GetViewMatrix());
	            J3DUniformBufferObject::SubmitUBO();
                Grid->Render({(sin(Rotate) * (Zoom * 2)), Zoom, (cos(Rotate) * (Zoom * 2))}, Camera.GetProjectionMatrix(), Camera.GetViewMatrix());
            }

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);

        }
    }

}
