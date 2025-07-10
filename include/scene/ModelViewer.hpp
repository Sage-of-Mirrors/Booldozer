#pragma once

#include <cstdint>
#include "glm/glm.hpp"
#include "bstream.h"
#include "imgui.h"
#include "io/BinIO.hpp"
#include "io/MdlIO.hpp"

enum class EModelType {
    Actor,
    Furniture,
    None
};

namespace PreviewWidget {

    BIN::Model* GetFurnitureModel();
    MDL::Model* GetActorModel();

    void SetActive();
    void SetInactive();
    bool GetActive();


    void InitPreview();
    void CleanupPreview();
    void RenderPreview(float dt);

    void PlayAnimation();
    void PauseAnimation();

    void UpdateCamera();
    void LoadModel(bStream::CMemoryStream* ModelStream, EModelType Type);
    void SaveModel(bStream::CMemoryStream* ModelStream);
    void SetModelAnimation(bStream::CMemoryStream* AnimStream);
    void SetSkeletalAnimation(bStream::CMemoryStream* AnimStream);
    void UnloadModel();

    void DoZoom(float amt);
    void DoRotate(float amt);

    void NewFurnitureModel(); 

    uint32_t PreviewID();

}
