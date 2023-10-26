#include "UPointSpriteManager.hpp"
#include <filesystem>
#include "stb_image.h"
#include <glad/glad.h>


const char* default_ps_vtx_shader_source = "#version 330\n\
layout (location = 0) in vec3 position;\n\
layout (location = 1) in int tex;\n\
layout (location = 2) in int size;\n\
layout (location = 3) in int flip_tex;\n\
flat out int tex_idx;\n\
flat out int flip;\n\
uniform mat4 gpu_ModelViewProjectionMatrix;\n\
void main()\n\
{\n\
    gl_Position = gpu_ModelViewProjectionMatrix * vec4(position, 1.0);\n\
    gl_PointSize = min(size, size / gl_Position.w);\n\
    tex_idx = tex;\n\
    flip = flip_tex;\n\
}\
";

const char* default_ps_frg_shader_source = "#version 330\n\
uniform sampler2DArray spriteTexture;\n\
flat in int tex_idx;\n\
flat in int flip;\n\
out vec4 outColor;\n\
void main()\n\
{\n\
    if(flip == 0){\n\
        gl_FragColor = texture(spriteTexture, vec3(gl_PointCoord, tex_idx));\n\
    } else {\n\
        gl_FragColor = texture(spriteTexture, vec3(-gl_PointCoord.x, gl_PointCoord.y, tex_idx));\n\
    }\n\
    if(gl_FragColor.a < 1.0 / 255.0) discard;\n\
}\
";

void CPointSpriteManager::Init(int BillboardResolution, int BillboardImageCount) {
	mBillboardResolution = BillboardResolution;
	mTextureCount = BillboardImageCount;
    glGenTextures(1, &mTextureID);
    glBindTexture(GL_TEXTURE_2D_ARRAY, mTextureID);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 4);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, BillboardResolution, BillboardResolution, BillboardImageCount, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

	//Compile Shaders
	{
	    char glErrorLogBuffer[4096];
	    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	
	    glShaderSource(vs, 1, &default_ps_vtx_shader_source, NULL);
	    glShaderSource(fs, 1, &default_ps_frg_shader_source, NULL);
	
	    glCompileShader(vs);
	
	    GLint status;
	    glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
	    if(status == GL_FALSE){
	        GLint infoLogLength;
	        glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &infoLogLength);
	
	        glGetShaderInfoLog(vs, infoLogLength, NULL, glErrorLogBuffer);
	
	        printf("Compile failure in vertex shader:\n%s\n", glErrorLogBuffer);
	    }
	
	    glCompileShader(fs);
	
	    glGetShaderiv(fs, GL_COMPILE_STATUS, &status);
	    if(status == GL_FALSE){
	        GLint infoLogLength;
	        glGetShaderiv(fs, GL_INFO_LOG_LENGTH, &infoLogLength);
	
	        glGetShaderInfoLog(fs, infoLogLength, NULL, glErrorLogBuffer);
	
	        printf("Compile failure in fragment shader:\n%s\n", glErrorLogBuffer);
	    }
	
	    mShaderID = glCreateProgram();
	
	    glAttachShader(mShaderID, vs);
	    glAttachShader(mShaderID, fs);
	
	    glLinkProgram(mShaderID);
	
	    glGetProgramiv(mShaderID, GL_LINK_STATUS, &status); 
	    if(GL_FALSE == status) {
	        GLint logLen; 
	        glGetProgramiv(mShaderID, GL_INFO_LOG_LENGTH, &logLen); 
	        glGetProgramInfoLog(mShaderID, logLen, NULL, glErrorLogBuffer); 
	        printf("Point Shader Program Linking Error:\n%s\n", glErrorLogBuffer);
	    } 
	
	    glDetachShader(mShaderID, vs);
	    glDetachShader(mShaderID, fs);
	
	    glDeleteShader(vs);
	    glDeleteShader(fs);

	}

    mMVPUniform = glGetUniformLocation(mShaderID, "gpu_ModelViewProjectionMatrix");

    glGenVertexArrays(1, &mVao);
    glBindVertexArray(mVao);

    glGenBuffers(1, &mVbo);
    glBindBuffer(GL_ARRAY_BUFFER, mVbo);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(CPointSprite), (void*)offsetof(CPointSprite, Position));
    glEnableVertexAttribArray(1);
    glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(CPointSprite), (void*)offsetof(CPointSprite, Texture));
    glEnableVertexAttribArray(2);
    glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(CPointSprite), (void*)offsetof(CPointSprite, SpriteSize));
    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(3, 1, GL_UNSIGNED_INT, sizeof(CPointSprite), (void*)offsetof(CPointSprite, Flip));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

}

CPointSpriteManager::CPointSpriteManager() {}

CPointSpriteManager::~CPointSpriteManager() {
	glDeleteTextures(1, &mTextureID);
}

void CPointSpriteManager::SetBillboardTexture(std::filesystem::path ImagePath, int TextureIndex){
	if(TextureIndex > mTextureCount || !std::filesystem::exists(ImagePath)) return;

	int x,y,n;
	unsigned char* img = stbi_load(ImagePath.string().c_str(), &x, &y, &n, 0);

    glBindTexture(GL_TEXTURE_2D_ARRAY, mTextureID);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, TextureIndex, mBillboardResolution, mBillboardResolution, 1, GL_RGBA, GL_UNSIGNED_BYTE, img);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

	stbi_image_free(img);
}

void CPointSpriteManager::UpdateData(){
    glBindBuffer(GL_ARRAY_BUFFER, mVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(CPointSprite) * mBillboards.size(), mBillboards.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void CPointSpriteManager::Draw(LSceneCamera *Camera) {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_PROGRAM_POINT_SIZE);
    //glEnable(GL_POINT_SPRITE);

	glm::mat4 mvp;
	mvp = Camera->GetProjectionMatrix() * Camera->GetViewMatrix() * glm::identity<glm::mat4>();

    glUseProgram(0);
    glBindVertexArray(0);

    glUseProgram(mShaderID);

	glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, mTextureID);

    glBindVertexArray(mVao);

    glUniformMatrix4fv(mMVPUniform, 1, 0, (float*)&mvp[0]);
    glDrawArrays(GL_POINTS, 0, mBillboards.size());

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    glBindVertexArray(0);
}