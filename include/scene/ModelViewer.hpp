#pragma once

#include <cstdint>
#include "glm/glm.hpp"
#include "bstream.h"
#include "imgui.h"
#include "io/BinIO.hpp"

enum class EModelType {
    Actor,
    Furniture,
    None
};

namespace PreviewWidget {

    BIN::Model* GetFurnitureModel();

    void SetActive();
    void SetInactive();
    bool GetActive();


    void InitPreview();
    void CleanupPreview();
    void RenderPreview();

    void PlayAnimation();
    void PauseAnimation();

    void UpdateCamera();
    void LoadModel(bStream::CMemoryStream* ModelStream, EModelType Type);
    void SaveModel(bStream::CMemoryStream* ModelStream);
    void SetModelAnimation(bStream::CMemoryStream* AnimStream);
    void UnloadModel();

    void DoZoom(float amt);
    void DoRotate(float amt);

    void NewFurnitureModel(); 

    uint32_t PreviewID();

}
