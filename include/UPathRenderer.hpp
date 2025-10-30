#pragma once

#include <glm/glm.hpp>
#include <cstdint>
#include <vector>
#include <filesystem>
#include <scene/Camera.hpp>

typedef struct {
    glm::vec3 Position;
    glm::vec4 Color;
    uint32_t SpriteSize;
    int32_t ID;
} CPathPoint;

class CPathRenderer {
    uint32_t mShaderID;
    uint32_t mMVPUniform;
    uint32_t mPointModeUniform;

    uint32_t mVao;
    uint32_t mVbo;

public:
	std::vector<std::vector<CPathPoint>> mPaths;

    void UpdateData();
	void Draw(LSceneCamera* Camera, bool enableDepth = true);

	void Init();
	void CleanUp();
	CPathRenderer();
	~CPathRenderer();
};
