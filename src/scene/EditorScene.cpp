#include "scene/EditorScene.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "imgui.h"
#include "ImGuizmo.h"
#include "Options.hpp"
#include "modes/EditorSelection.hpp"
#include "GLFW/glfw3.h"
#include "io/BinIO.hpp"

#include "cube_tex.h"

#include "DOM/EventDOMNode.hpp"
#include "DOM/GeneratorDOMNode.hpp"
#include "DOM/PathDOMNode.hpp"

#include <J3D/J3DUniformBufferObject.hpp>
#include <J3D/J3DModelLoader.hpp>

struct cube_vertex {
	float x, y, z, u, v;
};

//hardcoded cube rendering data
static const GLfloat s_cubeVertices[] = {
    -1.0f,-1.0f,-1.0f, 0.000059f, 1.0f-0.000004f,
    -1.0f,-1.0f, 1.0f, 0.000103f, 1.0f-0.336048f,
    -1.0f, 1.0f, 1.0f, 0.335973f, 1.0f-0.335903f,
    1.0f, 1.0f,-1.0f, 1.000023f, 1.0f-0.000013f,
    -1.0f,-1.0f,-1.0f, 0.667979f, 1.0f-0.335851f,
    -1.0f, 1.0f,-1.0f, 0.999958f, 1.0f-0.336064f,
    1.0f,-1.0f, 1.0f, 0.667979f, 1.0f-0.335851f,
    -1.0f,-1.0f,-1.0f, 0.336024f, 1.0f-0.671877f,
    1.0f,-1.0f,-1.0f, 0.667969f, 1.0f-0.671889f,
    1.0f, 1.0f,-1.0f, 1.000023f, 1.0f-0.000013f,
    1.0f,-1.0f,-1.0f, 0.668104f, 1.0f-0.000013f,
    -1.0f,-1.0f,-1.0f, 0.667979f, 1.0f-0.335851f,
    -1.0f,-1.0f,-1.0f, 0.000059f, 1.0f-0.000004f,
    -1.0f, 1.0f, 1.0f, 0.335973f, 1.0f-0.335903f,
    -1.0f, 1.0f,-1.0f, 0.336098f, 1.0f-0.000071f,
    1.0f,-1.0f, 1.0f, 0.667979f, 1.0f-0.335851f,
    -1.0f,-1.0f, 1.0f, 0.335973f, 1.0f-0.335903f,
    -1.0f,-1.0f,-1.0f, 0.336024f, 1.0f-0.671877f,
    -1.0f, 1.0f, 1.0f, 1.000004f, 1.0f-0.671847f,
    -1.0f,-1.0f, 1.0f, 0.999958f, 1.0f-0.336064f,
    1.0f,-1.0f, 1.0f, 0.667979f, 1.0f-0.335851f,
    1.0f, 1.0f, 1.0f, 0.668104f, 1.0f-0.000013f,
    1.0f,-1.0f,-1.0f, 0.335973f, 1.0f-0.335903f,
    1.0f, 1.0f,-1.0f, 0.667979f, 1.0f-0.335851f,
    1.0f,-1.0f,-1.0f, 0.335973f, 1.0f-0.335903f,
    1.0f, 1.0f, 1.0f, 0.668104f, 1.0f-0.000013f,
    1.0f,-1.0f, 1.0f, 0.336098f, 1.0f-0.000071f,
    1.0f, 1.0f, 1.0f, 0.000103f, 1.0f-0.336048f,
    1.0f, 1.0f,-1.0f, 0.000004f, 1.0f-0.671870f,
    -1.0f, 1.0f,-1.0f, 0.336024f, 1.0f-0.671877f,
    1.0f, 1.0f, 1.0f, 0.000103f, 1.0f-0.336048f,
    -1.0f, 1.0f,-1.0f, 0.336024f, 1.0f-0.671877f,
    -1.0f, 1.0f, 1.0f, 0.335973f, 1.0f-0.335903f,
    1.0f, 1.0f, 1.0f, 0.667969f, 1.0f-0.671889f,
    -1.0f, 1.0f, 1.0f, 1.000004f, 1.0f-0.671847f,
    1.0f,-1.0f, 1.0f, 0.667979f, 1.0f-0.335851f
	};
const char* cube_vtx_shader = "#version 460\n\
    #extension GL_ARB_separate_shader_objects : enable\n\
	struct GXLight {\n\
		vec4 Position;\n\
		vec4 Direction;\n\
		vec4 Color;\n\
		vec4 AngleAtten;\n\
		vec4 DistAtten;\n\
	};\n\
    layout (std140, binding=0) uniform uSharedData {\n\
		mat4 Proj;\n\
		mat4 View;\n\
		mat4 Model;\n\
		vec4 TevColor[4];\n\
		vec4 KonstColor[4];\n\
		GXLight Lights[8];\n\
		mat4 Envelopes[512];\n\
		mat4 TexMatrices[10];\n\
    };\n\
    uniform mat4 transform;\n\
    \
    layout(location = 0) in vec3 inPosition;\n\
    layout(location = 1) in vec2 inTexCoord;\n\
    \
    layout(location = 0) out vec2 fragTexCoord;\n\
    \
    void main()\n\
    {\
        gl_Position = Proj * View * transform * vec4(inPosition, 1.0);\n\
        fragTexCoord = inTexCoord;\n\
    }\
";

const char* cube_frg_shader = "#version 460\n\
    #extension GL_ARB_separate_shader_objects : enable\n\
    \
    uniform sampler2D texSampler;\n\
    layout(location = 0) in vec2 fragTexCoord;\n\
    layout(location = 0) out vec4 outColor;\n\
    \
    void main()\n\
    {\n\
        vec4 baseColor = texture(texSampler, vec2(fragTexCoord.y, fragTexCoord.x));\n\
        outColor = baseColor;\n\
        if(baseColor.a < 1.0 / 1.0f) discard;\n\
    }\
";


void LCubeManager::render(glm::mat4* transform, bool wireframe=false){
	glUseProgram(mCubeProgram);

	glUniformMatrix4fv(glGetUniformLocation(mCubeProgram, "transform"), 1, 0, &(*transform)[0][0]);

	if(wireframe) glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mCubeTex);

	glBindVertexArray(mVao);
	glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

	if(wireframe) glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
}

LCubeManager::LCubeManager(){
}

void LCubeManager::init(){
	int x, y, n;
	uint8_t* data = stbi_load_from_memory(&cube_png[0], cube_png_size, &x, &y, &n, 4);
	
	glGenTextures(1, &mCubeTex);
    glBindTexture(GL_TEXTURE_2D, mCubeTex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    
    glBindTexture(GL_TEXTURE_2D, 0);
	
	stbi_image_free(data);
	
    glGenVertexArrays(1, &mVao);
    glBindVertexArray(mVao);

    glGenBuffers(1, &mVbo);
    glBindBuffer(GL_ARRAY_BUFFER, mVbo);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(cube_vertex), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(cube_vertex), (void*)12);

    glBufferData(GL_ARRAY_BUFFER, sizeof(s_cubeVertices), s_cubeVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    char glErrorLogBuffer[4096];
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vs, 1, &cube_vtx_shader, NULL);
    glShaderSource(fs, 1, &cube_frg_shader, NULL);

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

    mCubeProgram = glCreateProgram();

    glAttachShader(mCubeProgram, vs);
    glAttachShader(mCubeProgram, fs);

    glLinkProgram(mCubeProgram);

    glGetProgramiv(mCubeProgram, GL_LINK_STATUS, &status); 
    if(GL_FALSE == status) {
        GLint logLen; 
        glGetProgramiv(mCubeProgram, GL_INFO_LOG_LENGTH, &logLen); 
        glGetProgramInfoLog(mCubeProgram, logLen, NULL, glErrorLogBuffer); 
        printf("Cube Shader Program Linking Error:\n%s\n", glErrorLogBuffer);
    } 

    glDetachShader(mCubeProgram, vs);
    glDetachShader(mCubeProgram, fs);

    glDeleteShader(vs);
    glDeleteShader(fs);

}

LCubeManager::~LCubeManager(){
	glDeleteTextures(1, &mCubeTex);
    glDeleteVertexArrays(1, &mVao);
    glDeleteBuffers(1, &mVbo);
	glDeleteBuffers(1, &mIbo);
}


LEditorScene::LEditorScene() : Initialized(false) {}

LEditorScene::~LEditorScene(){
	BinModel::DestroyShaders();
}

void LEditorScene::init(){
	Initialized = true;
	mCubeManager.init();

	mPathRenderer.Init();
	mPointManager.Init(512, 9);
	
	mPointManager.SetBillboardTexture(std::filesystem::current_path() / "res" / "img" / "ice_generator.png", 0);
	mPointManager.SetBillboardTexture(std::filesystem::current_path() / "res" / "img" / "fire_generator.png", 1);
	mPointManager.SetBillboardTexture(std::filesystem::current_path() / "res" / "img" / "water_generator.png", 2);

	mPointManager.SetBillboardTexture(std::filesystem::current_path() / "res" / "img" / "event.png", 3);
	mPointManager.SetBillboardTexture(std::filesystem::current_path() / "res" / "img" / "observer.png", 4);
	mPointManager.SetBillboardTexture(std::filesystem::current_path() / "res" / "img" / "enemy_placeholder.png", 5);

	mPointManager.SetBillboardTexture(std::filesystem::current_path() / "res" / "img" / "rat1.png", 6);
	mPointManager.SetBillboardTexture(std::filesystem::current_path() / "res" / "img" / "rat3.png", 7);
	mPointManager.SetBillboardTexture(std::filesystem::current_path() / "res" / "img" / "soundobj.png", 8);

	mDoorModels.reserve(14);
	BinModel::InitShaders();
	for (size_t f = 0; f < GCResourceManager.mGameArchive.dirnum; f++)
	{
		if(std::string(GCResourceManager.mGameArchive.dirs[f].name) == "door"){
			for (size_t i = GCResourceManager.mGameArchive.dirs[f].fileoff; i < GCResourceManager.mGameArchive.dirs[f].fileoff + GCResourceManager.mGameArchive.dirs[f].filenum; i++)
			{
				bStream::CMemoryStream bin_data((uint8_t*)GCResourceManager.mGameArchive.files[i].data, GCResourceManager.mGameArchive.files[i].size, bStream::Endianess::Big, bStream::OpenMode::In);
				if(std::filesystem::path(GCResourceManager.mGameArchive.files[i].name).extension() == ".bin"){
					auto doorModel = std::make_shared<BinModel>(&bin_data);
					mDoorModels.push_back(doorModel);
				}
			}
			
		}
	}
	
	J3DModelLoader Loader;
	
	GCarchive vrballArchive;

	if(GCResourceManager.mGameArchive.filenum != 0){
		GCarcfile* coinModelFile = GCResourceManager.GetFile(&GCResourceManager.mGameArchive, std::filesystem::path("kt_static") / "coin.bmd");
		if(coinModelFile != nullptr){
			//bStream::CMemoryStream modelData((uint8_t*)coinModelFile->data, coinModelFile->size, bStream::Endianess::Big, bStream::OpenMode::In);
			//mCoinModel = Loader.Load(&modelData, 0);
			//mCoin = mCoinModel->GetInstance();
		} else {
			std::cout << "Couldn't find coin" << std::endl;
		}
	}

	if((std::filesystem::exists(std::filesystem::path(OPTIONS.mRootPath) / "files" / "Iwamoto" / "vrball_M.szp"))){

		if(!GCResourceManager.LoadArchive((std::filesystem::path(OPTIONS.mRootPath) / "files" / "Iwamoto" / "vrball_M.szp").string().c_str(), &vrballArchive)){
			std::cout << "skybox problem oop" << std::endl;
		}

		if(vrballArchive.filenum != 0){
			GCarcfile* skyboxModel = GCResourceManager.GetFile(&vrballArchive, std::filesystem::path("vrball01.bmd"));

			if(skyboxModel != nullptr){
				bStream::CMemoryStream modelData((uint8_t*)skyboxModel->data, skyboxModel->size, bStream::Endianess::Big, bStream::OpenMode::In);
				mSkyboxModel = Loader.Load(&modelData, 0);
				mSkyBox = mSkyboxModel->GetInstance();
			} else {
				std::cout << "Couldn't find skybox" << std::endl;
			}

			gcFreeArchive(&vrballArchive);
		}
	}
}

glm::mat4 LEditorScene::getCameraView(){
	return Camera.GetViewMatrix();
}

glm::mat4 LEditorScene::getCameraProj(){
	return Camera.GetProjectionMatrix();
}

void LEditorScene::RenderSubmit(uint32_t m_width, uint32_t m_height){
	if (m_height == 0)
		m_height = 1;

	Camera.AspectRatio = float(m_width) / float(m_height);

	glm::mat4 view = Camera.GetViewMatrix();
	glm::mat4 proj = Camera.GetProjectionMatrix();

	J3DUniformBufferObject::SetProjAndViewMatrices(&proj, &view);

	for(std::weak_ptr<LRoomDOMNode> room : mCurrentRooms){
		if(!room.expired() && Initialized)
		{
			std::shared_ptr<LRoomDOMNode> roomLocked = room.lock();
			//vrball01.bmd
			if(roomLocked->GetSkyboxEnabled() && mSkyBox != nullptr){
				mSkyBox->SetTranslation(roomLocked->GetPosition());
				mSkyBox->SetRotation({0,0,0});
				mSkyBox->SetScale({15,15,15});
				mSkyBox->Render(0);
				break;
			}
		}
	}

    glFrontFace(GL_CW);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	mPointManager.mBillboards.clear();

	for(auto door : mRoomDoors){
		if (auto doorLocked = door.lock())
		{
			auto doorType = doorLocked->GetModel();
			if (doorType == EDoorModel::None)
				continue;

			// Construct transform matrix...
			glm::mat4 doorMat = glm::identity<glm::mat4>();

			// Translation. We need the translation from the transform matrix (column at [3]) as well as the Y scale (float at [1][1])
			doorMat = glm::translate(doorMat, glm::vec3((*doorLocked->GetMat())[3]) - glm::vec3(0, (*doorLocked->GetMat())[1][1] / 2.f, 0));

			// Rotation is based on the door's orientation type.
			if (doorLocked->GetOrientation() == EDoorOrientation::Side_Facing)
				doorMat = glm::rotate(doorMat, glm::radians(90.0f), glm::vec3(0, 1, 0));

			// The Square Mansion Door model is fucked, so this is a hack to make sure it shows up (mostly) correctly in the editor.
			bool bIgnoreTransforms = doorType == EDoorModel::Square_Mansion_Door;
			if (bIgnoreTransforms)
				doorMat = glm::translate(doorMat, glm::vec3(0, 0, 100));

			// Double doors need to be rendered twice, with the two halves moved accordingly.
			if (doorType == EDoorModel::Parlor_Double_Door || doorType == EDoorModel::Hearts_Double_Door)
			{
				glm::vec3 doubleDoorOffset = glm::vec3(0, 0, 100);

				// Offset the first door (right/forward) and render it.
				doorMat = glm::translate(doorMat, doubleDoorOffset);
				mDoorModels[(uint8_t)doorType - 1]->Draw(&doorMat);

				// Now offset the second door (left/backward) and rotate it 180 degrees.
				doubleDoorOffset *= 2;
				doorMat = glm::translate(doorMat, -doubleDoorOffset);
				doorMat = glm::rotate(doorMat, glm::radians(180.f), glm::vec3(0, 1, 0));

				// Render second door.
				mDoorModels[(uint8_t)doorType - 1]->Draw(&doorMat);
			}
			// Single door can just be rendered without hassle.
			else
			{
				mDoorModels[(uint8_t)doorType - 1]->Draw(&doorMat, bIgnoreTransforms);
			}
		}
	}

	for(auto room : mCurrentRooms){
		glm::mat4 identity = glm::identity<glm::mat4>();
		for (auto room : mRoomModels)
		{
			room->Draw(&identity);
		}
		
		if(!room.expired() && Initialized)
		{
			auto curRoom = room.lock();

			curRoom->ForEachChildOfType<LBGRenderDOMNode>(EDOMNodeType::BGRender, [&](auto node){
					glm::mat4 transform = glm::identity<glm::mat4>();
					switch (node->GetNodeType())
					{
					case EDOMNodeType::Furniture:
						if(mRoomFurniture.count(node->GetName()) == 0)
						{
							mCubeManager.render(node->GetMat());
						} else {
							mRoomFurniture[node->GetName()]->Draw(node->GetMat());
						}
						break;

					case EDOMNodeType::Character:
					case EDOMNodeType::Generator:
						if (node->GetName() == "elice"){
							mPointManager.mBillboards.push_back({node->GetPosition(), 51200, 0, false, false});
						} else if(node->GetName() == "elfire"){
							mPointManager.mBillboards.push_back({node->GetPosition(), 51200, 1, false, false});
						} else if(node->GetName() == "rat1"){
							mPointManager.mBillboards.push_back({node->GetPosition(), 51200, 6, false, false});
						} else if(node->GetName() == "rat3"){
							mPointManager.mBillboards.push_back({node->GetPosition(), 51200, 7, false, false});
						} else {
							mPointManager.mBillboards.push_back({node->GetPosition(), 51200, 2, false, false});
						}
						break;
					case EDOMNodeType::Event:
						mPointManager.mBillboards.push_back({node->GetPosition(), 51200, 3, false, false});
						break;
					case EDOMNodeType::Observer:
						if(node->GetName() == "Sound Effect Player"){
							mPointManager.mBillboards.push_back({node->GetPosition(), 51200, 8, false, false});
						} else {
							mPointManager.mBillboards.push_back({node->GetPosition(), 51200, 4, false, false});
						}
						break;
					case EDOMNodeType::Object:
						if(node->GetName() == "coin" && mCoin != nullptr){
							mCoin->SetTranslation(node->GetPosition());
							mCoin->SetRotation(node->GetRotation());
							mCoin->SetScale({1,1,1});
							mCoin->Render(0);
						}
						break;
					case EDOMNodeType::Enemy:
						mPointManager.mBillboards.push_back({node->GetPosition(), 51200, 5, false, false});
						break;
				}
			});

		}
	}
	//Draw GL Lines Based thing for room boundaries


	mPointManager.Draw(&Camera);
	mPathRenderer.Draw(&Camera);
}

bool LEditorScene::HasRoomLoaded(int32_t roomNumber){
	for (auto& room : mCurrentRooms)
	{
		if(room.lock()->GetRoomNumber() == roomNumber) return true;
	}
	return false;
}

void LEditorScene::SetRoom(std::shared_ptr<LRoomDOMNode> room)
{
	// Get the select room's data so we can get the preload list
	auto roomData = room->GetChildrenOfType<LRoomDataDOMNode>(EDOMNodeType::RoomData);

	mRoomModels.clear();
	mRoomFurniture.clear();
	mPathRenderer.mPaths.clear();
	
	auto paths = room->GetChildrenOfType<LPathDOMNode>(EDOMNodeType::Path);
	
	for(auto path : paths){
		mPathRenderer.mPaths.push_back(path->mPathRenderable);
		std::cout << "Added path " << path->GetName() << " to renderer" << std::endl; 
	}

	//This is ensured to exist, but check it anyway
	if(roomData.size() != 0)
	{
		mCurrentRooms = roomData.front()->GetAdjacencyList();
			
		mRoomDoors = roomData.front()->GetDoorList();

		for (auto& aroom :  roomData.front()->GetAdjacencyList())
		{
			
			auto curRoomData = aroom.lock()->GetChildrenOfType<LRoomDataDOMNode>(EDOMNodeType::RoomData).front();


			glm::vec4 color = (roomData.front() == curRoomData ? (glm::vec4){0.0f, 1.0f, 0.0f, 1.0f} : (glm::vec4){1.0f, 1.0f, 1.0f, 1.0f});

			glm::vec3 min = curRoomData->GetMin();
			glm::vec3 max = curRoomData->GetMax();
			std::vector<CPathPoint> bounds_bottom = {
				{min, color, 51200},
				{{min.x, min.y, max.z}, color, 51200},
				{{max.x, min.y, max.z}, color, 51200},
				{{max.x, min.y, min.z}, color, 51200},
				{min, color, 51200},
			};

			std::vector<CPathPoint> bounds_top = {
				{max, color, 51200},
				{{max.x, max.y, min.z}, color, 51200},
				{{min.x, max.y, min.z}, color, 51200},
				{{min.x, max.y, max.z}, color, 51200},
				{max, color, 51200},
			};

			std::vector<CPathPoint> bounds_edge1 = {{min, color, 51200}, {{min.x, max.y, min.z}, color, 51200}};
			std::vector<CPathPoint> bounds_edge2 = {{{max.x, min.y, min.z}, color, 51200}, {{max.x, max.y, min.z}, color, 51200}};
			std::vector<CPathPoint> bounds_edge3 = {{{min.x, min.y, max.z}, color, 51200}, {{min.x, max.y, max.z}, color, 51200}};
			std::vector<CPathPoint> bounds_edge4 = {{max, color, 51200}, {{max.x, min.y, max.z}, color, 51200}};

			mPathRenderer.mPaths.push_back(bounds_bottom);
			mPathRenderer.mPaths.push_back(bounds_top);

			mPathRenderer.mPaths.push_back(bounds_edge1);
			mPathRenderer.mPaths.push_back(bounds_edge2);
			mPathRenderer.mPaths.push_back(bounds_edge3);
			mPathRenderer.mPaths.push_back(bounds_edge4);

			std::filesystem::path resPath = std::filesystem::path(OPTIONS.mRootPath) / "files" / std::filesystem::path(curRoomData->GetResourcePath()).relative_path();
			
			if(resPath.extension() == ".arc")
			{
				GCarchive roomArc;
				if(!GCResourceManager.LoadArchive(resPath.string().data(), &roomArc)){
					std::cout << "Unable to load room archive " << resPath << std::endl;
				}

				for(int file = 0; file < roomArc.filenum; file++)
				{
					std::filesystem::path curPath = std::filesystem::path(roomArc.files[file].name);
					if(curPath.extension() == ".bin")
					{
						bStream::CMemoryStream bin((uint8_t*)roomArc.files[file].data, roomArc.files[file].size, bStream::Endianess::Big, bStream::OpenMode::In);
						std::cout << "loading " << curPath.filename() << std::endl;
						if (curPath.filename().stem() != "room")
						{
							mRoomFurniture[curPath.filename().stem().string()] = std::make_shared<BinModel>(&bin);
							std::cout << "completed loading " << curPath.filename() << std::endl;
						} else {
							mRoomModels.push_back(std::make_shared<BinModel>(&bin));
							std::cout << "completed loading room model" << std::endl;
						}
						
					}
				}
				std::cout << "all models locked and loaded" << std::endl;
			} else {
				//If this is happening the map only has room models, no furniture.
				bStream::CFileStream bin(resPath.string(), bStream::Endianess::Big, bStream::OpenMode::In);
				mRoomModels.push_back(std::make_shared<BinModel>(&bin));
			}
		}

	}

	mPathRenderer.UpdateData();
}

void LEditorScene::update(GLFWwindow* window, float dt, LEditorSelection* selection)
{
	if(Camera.mCamMode == ECamMode::ORBIT){
		//oh god no
		Camera.SetCenter(mCurrentRooms[0].lock().get()->GetChildrenOfType<LRoomDataDOMNode>(EDOMNodeType::RoomData)[0].get()->GetPosition());
	}

	Camera.Update(window, dt);

	int w, h;
	int vx, vy;
	double x, y;
	glfwGetCursorPos(window, &x, &y);

	glfwGetWindowSize(window, &w, &h);
	glfwGetWindowPos(window, &vx, &vy);

	//TODO: replace with depth picking or other
}
