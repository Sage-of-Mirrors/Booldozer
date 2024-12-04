#include "UPlaneRenderer.hpp"
#include "stb_image.h"
#include "scene/EditorScene.hpp"

static float PlaneVertices[] = {
	0.0,  2.0,  1.0,  0.0, 1.0,
	0.0,  0.0,  1.0,  1.0, 1.0,
	0.0,  2.0, -1.0,  0.0, 0.0,
	0.0,  0.0, -1.0,  1.0, 0.0
};

const char* plane_vtx_shader_source = "#version 460\n\
    uniform mat4 gpu_ModelViewProjectionMatrix;\n\
    layout(location = 0) in vec3 inPosition;\n\
    layout(location = 1) in vec2 inTexCoord;\n\
    \
    layout(location = 0) out vec2 fragTexCoord;\n\
    \
    void main()\n\
    {\
        gl_Position = gpu_ModelViewProjectionMatrix * vec4(inPosition, 1.0);\n\
        fragTexCoord = inTexCoord;\n\
    }\
";

const char* plane_frg_shader_source = "#version 460\n\
    #extension GL_ARB_separate_shader_objects : enable\n\
    \
    uniform sampler2D texSampler;\n\
    uniform int selected;\n\
    uniform int pickID;\n\
    uniform int resX;\n\
    uniform int resY;\n\
    layout(location = 0) in vec2 fragTexCoord;\n\
    \
    layout(location = 0) out vec4 outColor;\n\
    layout(location = 1) out int outPick;\n\
    \
    void main()\n\
    {\n\
        outColor = texture(texSampler, vec2(fragTexCoord.y * resY, fragTexCoord.x * resX) * vec2(0.05, 0.05));\n\
        outPick = pickID;\n\
    }\
";

void CPlaneRenderer::Init(std::string texPath){

    // Compile Shaders for mirror planes
	char glErrorLogBuffer[4096];
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(vs, 1, &plane_vtx_shader_source, nullptr);
	glShaderSource(fs, 1, &plane_frg_shader_source, nullptr);

	glCompileShader(vs);

	GLint status;
	glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
	if(status == GL_FALSE){
		GLint infoLogLength;
		glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &infoLogLength);

		glGetShaderInfoLog(vs, infoLogLength, nullptr, glErrorLogBuffer);

		printf("[Editor Scene]: Compile failure in mirror vertex shader:\n%s\n", glErrorLogBuffer);
	}

	glCompileShader(fs);

	glGetShaderiv(fs, GL_COMPILE_STATUS, &status);
	if(status == GL_FALSE){
		GLint infoLogLength;
		glGetShaderiv(fs, GL_INFO_LOG_LENGTH, &infoLogLength);
		glGetShaderInfoLog(fs, infoLogLength, nullptr, glErrorLogBuffer);
		printf("[EditorScene]: Compile failure in mirror fragment shader:\n%s\n", glErrorLogBuffer);
	}

	mProgramID = glCreateProgram();

	glAttachShader(mProgramID, vs);
	glAttachShader(mProgramID, fs);

	glLinkProgram(mProgramID);

	glGetProgramiv(mProgramID, GL_LINK_STATUS, &status); 
	if(GL_FALSE == status) {
		GLint logLen; 
		glGetProgramiv(mProgramID, GL_INFO_LOG_LENGTH, &logLen); 
		glGetProgramInfoLog(mProgramID, logLen, nullptr, glErrorLogBuffer); 
		printf("[EditorScene]: Mirror Shader Program Linking Error:\n%s\n", glErrorLogBuffer);
	} 

	glDetachShader(mProgramID, vs);
	glDetachShader(mProgramID, fs);

	glDeleteShader(vs);
	glDeleteShader(fs);

    mMVPUniform = glGetUniformLocation(mProgramID, "gpu_ModelViewProjectionMatrix");    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    mSelectedUniform = glGetUniformLocation(mProgramID, "selected");
    mPickIDUniform = glGetUniformLocation(mProgramID, "pickID");
    mResX = glGetUniformLocation(mProgramID, "resX");
    mResY = glGetUniformLocation(mProgramID, "resY");

	glGenVertexArrays(1, &mVao);
    glBindVertexArray(mVao);

    glGenBuffers(1, &mVbo);
    glBindBuffer(GL_ARRAY_BUFFER, mVbo);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float[5]), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float[5]), (void*)12);

    glBufferData(GL_ARRAY_BUFFER, sizeof(PlaneVertices), PlaneVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

	int x,y,n;
	unsigned char* img = stbi_load(texPath.c_str(), &x, &y, &n, 0);

    glGenTextures(1, &mTexture);

    glBindTexture(GL_TEXTURE_2D, mTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(img);
    glUseProgram(0);
}

void CPlaneRenderer::Draw(glm::mat4* transform, uint32_t id, uint32_t selected, int32_t texScaleX, int32_t texScaleY){
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_PROGRAM_POINT_SIZE);

    glBindTexture(GL_TEXTURE_2D, mTexture);

	glm::mat4 mvp = LEditorScene::GetEditorScene()->Camera.GetProjectionMatrix() * LEditorScene::GetEditorScene()->Camera.GetViewMatrix();
    mvp *= glm::scale((*transform), glm::vec3(1.0, texScaleY, texScaleX));

    glUseProgram(mProgramID);
    glBindVertexArray(mVao);
    glUniformMatrix4fv(mMVPUniform, 1, 0, (float*)&mvp[0]);
    glUniform1i(mSelectedUniform, selected);
    glUniform1i(mPickIDUniform, id);
    glUniform1i(mResX, texScaleX);
    glUniform1i(mResY, texScaleY);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}

CPlaneRenderer::~CPlaneRenderer(){
    glDeleteTextures(1, &mTexture);
    glDeleteProgram(mProgramID);
    glDeleteVertexArrays(1, &mVao);
    glDeleteBuffers(1, &mVbo);
}