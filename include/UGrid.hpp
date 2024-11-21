#pragma once

#include <cstdint>
#include <glm/glm.hpp>

const float GRID_SCALE = 10000.0f;
const float TEX_SCALE = 100.0f;
const uint32_t MIPMAP_COUNT = 6;

class UGrid {
    glm::vec3 mVertices[8] {
        glm::vec3(-1.0f, 0.0f, 1.0f) * GRID_SCALE, // Top left
        glm::vec3(0.0f, 0.0f, 0.0f),

        glm::vec3(1.0f, 0.0f, 1.0f) * GRID_SCALE,  // Top right
        glm::vec3(0.0f, 1.0f, 0.0f),

        glm::vec3(1.0f, 0.0f, -1.0f) * GRID_SCALE, // Bottom right
        glm::vec3(1.0f, 1.0f, 0.0f),

        glm::vec3(-1.0f, 0.0f, -1.0f) * GRID_SCALE, // Bottom left
        glm::vec3(1.0f, 0.0f, 0.0f)
    };

    uint8_t mIndices[6] { 0, 1, 2, 2, 0, 3 };

    glm::mat4 mModelMtx;

    int32_t mShaderProgram;
    uint32_t mTextureHandle;

    uint32_t mModelMtxUniformId;
    uint32_t mViewMtxUniformId;
    uint32_t mProjMtxUniformId;
    uint32_t mTexScaleUniformId;

    uint32_t mVBO;
    uint32_t mIBO;
    uint32_t mVAO;

    void InitTexture();
    void InitShader();
    void InitGeometry();

public:
    UGrid();
    ~UGrid();

    void Init();
    void Render(const glm::vec3& camPos, const glm::mat4& projMtx, const glm::mat4& viewMtx);
};
