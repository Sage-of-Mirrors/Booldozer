
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

namespace PreviewWidget {
    static bool Ready { false };
    static uint32_t Fbo { 0 }, Rbo { 0 }, Tex { 0 };

    static bool Active { false };
    static LSceneCamera Camera;
    static glm::mat4 Identity { 1.0 };

    static float Rotate { 0.0f };
    static float Zoom { 500.0f };

    static BinModel* Model { nullptr };

    void SetActive(){
        Active = true;
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
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        glDeleteFramebuffers(1, &Fbo);
        glDeleteRenderbuffers(1, &Rbo);
        glDeleteTextures(1, &Tex);
    }

    void LoadModel(bStream::CMemoryStream* ModelStream){
        if(Model != nullptr){
            delete Model;
            Model = nullptr;
        }
        Model = new BinModel(ModelStream);
        Camera.SetCenter(Model->GetRootPosition());
    }

    void UnloadModel(){
        if(Model != nullptr){
            delete Model;
        }
        Model = nullptr;
    }

    void RenderPreview(){
        if(Active){
            glBindFramebuffer(GL_FRAMEBUFFER, Fbo);
            glBindRenderbuffer(GL_RENDERBUFFER, Rbo);

            glViewport(0, 0, 500, 500);
            glClearColor(0.100f, 0.261f, 0.402f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            

            // TODO: Draw Grid somehow

            if(Model != nullptr){
                Camera.SetEye({Model->GetRootPosition().x + (sin(Rotate) * (Zoom * 2)), Model->GetRootPosition().y + Zoom, Model->GetRootPosition().z - (cos(Rotate) * (Zoom * 2))});
                
                // Ow.
	            J3DUniformBufferObject::SetProjAndViewMatrices(Camera.GetProjectionMatrix(), Camera.GetViewMatrix());
	            J3DUniformBufferObject::SubmitUBO();

                Model->Draw(&Identity, 0, false, false);
            }

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
            
        }
    }
    
}