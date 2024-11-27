#include <glm/glm.hpp>
#include <vector>
#include <glad/glad.h>
#include <cstdio>
#include <string>

class CPlaneRenderer {
    uint32_t mVao, mVbo, mProgramID, mTexture;

public:
    void Init(std::string texPath);

    void Draw(glm::mat4* transform, uint32_t id, uint32_t selected);

    CPlaneRenderer() {}
    ~CPlaneRenderer();
};