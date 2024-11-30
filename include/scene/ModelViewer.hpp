#pragma once

#include <cstdint>
#include "glm/glm.hpp"
#include "bstream.h"
#include "imgui.h"

enum class EModelType {
    Actor,
    Furniture,
    None
};

namespace PreviewWidget {

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
    void SetModelAnimation(bStream::CMemoryStream* AnimStream);
    void UnloadModel();

    void DoZoom(float amt);
    void DoRotate(float amt);

    uint32_t PreviewID();
    
}