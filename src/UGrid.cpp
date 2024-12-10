#include "UGrid.hpp"
#include "UGridTex.hpp"
#include "UGridShader.hpp"

#include <algorithm>

#include <glad/glad.h>

UGrid::UGrid() : mModelMtx(glm::mat4(1.0f)), mShaderProgram(-1), mTextureHandle(-1), mModelMtxUniformId(-1), mViewMtxUniformId(-1), mProjMtxUniformId(-1) {

}

UGrid::~UGrid() {
    if (mShaderProgram != -1) {
        glDeleteProgram(mShaderProgram);
    }

    if (mTextureHandle != -1) {
        glDeleteTextures(1, &mTextureHandle);
    }

    if(mVAO != -1){
        glDeleteVertexArrays(1, &mVAO);
    }

    if(mVBO != -1){
        glDeleteVertexArrays(1, &mVBO);
    }

    if(mIBO != -1){
        glDeleteVertexArrays(1, &mIBO);
    }
}

void UGrid::InitTexture() {
    glCreateTextures(GL_TEXTURE_2D, 1, &mTextureHandle);

    glTextureParameteri(mTextureHandle, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(mTextureHandle, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(mTextureHandle, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(mTextureHandle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTextureStorage2D(mTextureHandle, MIPMAP_COUNT, GL_RGBA8, GRIDTEX_WIDTH, GRIDTEX_HEIGHT);
    glTextureSubImage2D(mTextureHandle, 0, 0, 0, GRIDTEX_WIDTH, GRIDTEX_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, UGridTex);

    glGenerateTextureMipmap(mTextureHandle);
}

void UGrid::InitShader() {
    uint32_t vertShader, fragShader;
    
    vertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertShader, 1, &UGridVertexShader, NULL);
    glCompileShader(vertShader);

    fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, &UGridFragmentShader, NULL);
    glCompileShader(fragShader);

    mShaderProgram = glCreateProgram();
    glAttachShader(mShaderProgram, vertShader);
    glAttachShader(mShaderProgram, fragShader);

    glLinkProgram(mShaderProgram);

    glDetachShader(mShaderProgram, vertShader);
    glDetachShader(mShaderProgram, fragShader);
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    mModelMtxUniformId = glGetUniformLocation(mShaderProgram, "uModelMtx");
    mViewMtxUniformId = glGetUniformLocation(mShaderProgram, "uViewMtx");
    mProjMtxUniformId = glGetUniformLocation(mShaderProgram, "uProjMtx");

    mTexScaleUniformId = glGetUniformLocation(mShaderProgram, "uTexScale");
    glProgramUniform1f(mShaderProgram, mTexScaleUniformId, TEX_SCALE);
    
    uint32_t texSamplerId = glGetUniformLocation(mShaderProgram, "uTexture");
    glProgramUniform1i(mShaderProgram, texSamplerId, 0);
}

void UGrid::InitGeometry() {
    glCreateBuffers(1, &mVBO);
    glNamedBufferStorage(mVBO, 8 * sizeof(glm::vec3), mVertices, NULL);

    glCreateBuffers(1, &mIBO);
    glNamedBufferStorage(mIBO, 6 * sizeof(uint8_t), mIndices, NULL);

    glCreateVertexArrays(1, &mVAO);
    glVertexArrayVertexBuffer(mVAO, 0, mVBO, 0, 2 * sizeof(glm::vec3));
    glVertexArrayVertexBuffer(mVAO, 1, mVBO, sizeof(glm::vec3), 2 * sizeof(glm::vec3));
    glVertexArrayElementBuffer(mVAO, mIBO);

    glEnableVertexArrayAttrib(mVAO, 0);
    glVertexArrayAttribBinding(mVAO, 0, 0);
    glVertexArrayAttribFormat(mVAO, 0, 3, GL_FLOAT, GL_FALSE, 0);

    glEnableVertexArrayAttrib(mVAO, 1);
    glVertexArrayAttribBinding(mVAO, 1, 0);
    glVertexArrayAttribFormat(mVAO, 1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3));
}

void UGrid::Init() {
    InitTexture();
    InitShader();
    InitGeometry();
}

void UGrid::Render(const glm::vec3& camPos, const glm::mat4& projMtx, const glm::mat4& viewMtx) {
    glBindVertexArray(mVAO);
    glUseProgram(mShaderProgram);
    glBindTextureUnit(0, mTextureHandle);

    glUniformMatrix4fv(mModelMtxUniformId, 1, GL_FALSE, &mModelMtx[0][0]);
    glUniformMatrix4fv(mViewMtxUniformId, 1, GL_FALSE, &viewMtx[0][0]);
    glUniformMatrix4fv(mProjMtxUniformId, 1, GL_FALSE, &projMtx[0][0]);

    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);

    glDisable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ZERO);

    glDisable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glDisable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);

    glBindTextureUnit(0, 0);
    glUseProgram(0);
    glBindVertexArray(0);
}
