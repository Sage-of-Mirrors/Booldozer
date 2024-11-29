#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <glad/glad.h>
#include <cstdio>
#include <string>

class CPlaneRenderer {
    uint32_t mVao, mVbo, mProgramID, mTexture;
    uint32_t mMVPUniform;
    uint32_t mSelectedUniform;
    uint32_t mPickIDUniform;
    uint32_t mResX, mResY;

public:
    void Init(std::string texPath);

    void Draw(glm::mat4* transform, uint32_t id, uint32_t selected, int32_t texScaleX, int32_t texScaleY);

    CPlaneRenderer() {}
    ~CPlaneRenderer();
};