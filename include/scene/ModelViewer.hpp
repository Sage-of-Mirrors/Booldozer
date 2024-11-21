#pragma once

#include <cstdint>
#include "glm/glm.hpp"
#include "bstream.h"
#include "imgui.h"

namespace PreviewWidget {

    void SetActive();
    void SetInactive();
    bool GetActive();


    void InitPreview();
    void CleanupPreview();
    void RenderPreview();

    void UpdateCamera();
    void LoadModel(bStream::CMemoryStream* modelStream);
    void UnloadModel();

    void DoZoom(float amt);
    void DoRotate(float amt);

    uint32_t PreviewID();
    
}