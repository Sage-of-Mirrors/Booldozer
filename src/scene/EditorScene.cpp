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
#include <io/MdlIO.hpp>

#include "cube_tex.h"

#include "DOM/EventDOMNode.hpp"
#include "DOM/GeneratorDOMNode.hpp"
#include "DOM/PathDOMNode.hpp"
#include "DOM/EnemyDOMNode.hpp"

#include <J3D/J3DUniformBufferObject.hpp>
#include <J3D/J3DModelLoader.hpp>
#include <J3D/J3DRendering.hpp>


struct cube_vertex {
	float x, y, z, u, v;
};

//hardcoded cube rendering data
static const GLfloat s_cubeVertices[] = {
	-10.0f,-10.0f,-10.0f, 0.000059f, 10.0f-0.000004f,
	-10.0f,-10.0f, 10.0f, 0.000103f, 10.0f-0.336048f,
	-10.0f, 10.0f, 10.0f, 0.335973f, 10.0f-0.335903f,
	10.0f, 10.0f,-10.0f, 10.000023f, 10.0f-0.000013f,
	-10.0f,-10.0f,-10.0f, 0.667979f, 10.0f-0.335851f,
	-10.0f, 10.0f,-10.0f, 0.999958f, 10.0f-0.336064f,
	10.0f,-10.0f, 10.0f, 0.667979f, 10.0f-0.335851f,
	-10.0f,-10.0f,-10.0f, 0.336024f, 10.0f-0.671877f,
	10.0f,-10.0f,-10.0f, 0.667969f, 10.0f-0.671889f,
	10.0f, 10.0f,-10.0f, 10.000023f, 10.0f-0.000013f,
	10.0f,-10.0f,-10.0f, 0.668104f, 10.0f-0.000013f,
	-10.0f,-10.0f,-10.0f, 0.667979f, 10.0f-0.335851f,
	-10.0f,-10.0f,-10.0f, 0.000059f, 10.0f-0.000004f,
	-10.0f, 10.0f, 10.0f, 0.335973f, 10.0f-0.335903f,
	-10.0f, 10.0f,-10.0f, 0.336098f, 10.0f-0.000071f,
	10.0f,-10.0f, 10.0f, 0.667979f, 10.0f-0.335851f,
	-10.0f,-10.0f, 10.0f, 0.335973f, 10.0f-0.335903f,
	-10.0f,-10.0f,-10.0f, 0.336024f, 10.0f-0.671877f,
	-10.0f, 10.0f, 10.0f, 10.000004f, 10.0f-0.671847f,
	-10.0f,-10.0f, 10.0f, 0.999958f, 10.0f-0.336064f,
	10.0f,-10.0f, 10.0f, 0.667979f, 10.0f-0.335851f,
	10.0f, 10.0f, 10.0f, 0.668104f, 10.0f-0.000013f,
	10.0f,-10.0f,-10.0f, 0.335973f, 10.0f-0.335903f,
	10.0f, 10.0f,-10.0f, 0.667979f, 10.0f-0.335851f,
	10.0f,-10.0f,-10.0f, 0.335973f, 10.0f-0.335903f,
	10.0f, 10.0f, 10.0f, 0.668104f, 10.0f-0.000013f,
	10.0f,-10.0f, 10.0f, 0.336098f, 10.0f-0.000071f,
	10.0f, 10.0f, 10.0f, 0.000103f, 10.0f-0.336048f,
	10.0f, 10.0f,-10.0f, 0.000004f, 10.0f-0.671870f,
	-10.0f, 10.0f,-10.0f, 0.336024f, 10.0f-0.671877f,
	10.0f, 10.0f, 10.0f, 0.000103f, 10.0f-0.336048f,
	-10.0f, 10.0f,-10.0f, 0.336024f, 10.0f-0.671877f,
	-10.0f, 10.0f, 10.0f, 0.335973f, 10.0f-0.335903f,
	10.0f, 10.0f, 10.0f, 0.667969f, 10.0f-0.671889f,
	-10.0f, 10.0f, 10.0f, 10.000004f, 10.0f-0.671847f,
	10.0f,-10.0f, 10.0f, 0.667979f, 10.0f-0.335851f
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

// From: https://github.com/opengl-tutorials/ogl/blob/master/misc05_picking/misc05_picking_custom.cpp

void ScreenPosToWorldRay(
	int mouseX, int mouseY,             // Mouse position, in pixels, from bottom-left corner of the window
	int screenWidth, int screenHeight,  // Window size, in pixels
	glm::mat4 ViewMatrix,               // Camera position and orientation
	glm::mat4 ProjectionMatrix,         // Camera parameters (ratio, field of view, near and far planes)
	glm::vec3& out_origin,              // Ouput : Origin of the ray. /!\ Starts at the near plane, so if you want the ray to start at the camera's position instead, ignore this.
	glm::vec3& out_direction            // Ouput : Direction, in world space, of the ray that goes "through" the mouse.
){

	// The ray Start and End positions, in Normalized Device Coordinates (Have you read Tutorial 4 ?)
	glm::vec4 lRayStart_NDC(
		((float)mouseX/(float)screenWidth  - 0.5f) * 2.0f, // [0,1024] -> [-1,1]
		((float)mouseY/(float)screenHeight - 0.5f) * 2.0f, // [0, 768] -> [-1,1]
		-1.0, // The near plane maps to Z=-1 in Normalized Device Coordinates
		1.0f
	);
	glm::vec4 lRayEnd_NDC(
		((float)mouseX/(float)screenWidth  - 0.5f) * 2.0f,
		((float)mouseY/(float)screenHeight - 0.5f) * 2.0f,
		0.0,
		1.0f
	);


	// The Projection matrix goes from Camera Space to NDC.
	// So inverse(ProjectionMatrix) goes from NDC to Camera Space.
	glm::mat4 InverseProjectionMatrix = glm::inverse(ProjectionMatrix);
	
	// The View Matrix goes from World Space to Camera Space.
	// So inverse(ViewMatrix) goes from Camera Space to World Space.
	glm::mat4 InverseViewMatrix = glm::inverse(ViewMatrix);
	
	glm::vec4 lRayStart_camera = InverseProjectionMatrix * lRayStart_NDC;    lRayStart_camera/=lRayStart_camera.w;
	glm::vec4 lRayStart_world  = InverseViewMatrix       * lRayStart_camera; lRayStart_world /=lRayStart_world .w;
	glm::vec4 lRayEnd_camera   = InverseProjectionMatrix * lRayEnd_NDC;      lRayEnd_camera  /=lRayEnd_camera  .w;
	glm::vec4 lRayEnd_world    = InverseViewMatrix       * lRayEnd_camera;   lRayEnd_world   /=lRayEnd_world   .w;


	// Faster way (just one inverse)
	//glm::mat4 M = glm::inverse(ProjectionMatrix * ViewMatrix);
	//glm::vec4 lRayStart_world = M * lRayStart_NDC; lRayStart_world/=lRayStart_world.w;
	//glm::vec4 lRayEnd_world   = M * lRayEnd_NDC  ; lRayEnd_world  /=lRayEnd_world.w;


	glm::vec3 lRayDir_world(lRayEnd_world - lRayStart_world);
	lRayDir_world = glm::normalize(lRayDir_world);


	out_origin = glm::vec3(lRayStart_world);
	out_direction = glm::normalize(lRayDir_world);
}


bool TestRayOBBIntersection(
	glm::vec3 ray_origin,        // Ray origin, in world space
	glm::vec3 ray_direction,     // Ray direction (NOT target position!), in world space. Must be normalize()'d.
	glm::vec3 aabb_min,          // Minimum X,Y,Z coords of the mesh when not transformed at all.
	glm::vec3 aabb_max,          // Maximum X,Y,Z coords. Often aabb_min*-1 if your mesh is centered, but it's not always the case.
	glm::mat4 ModelMatrix,       // Transformation applied to the mesh (which will thus be also applied to its bounding box)
	float& intersection_distance // Output : distance between ray_origin and the intersection with the OBB
){
	
	// Intersection method from Real-Time Rendering and Essential Mathematics for Games
	
	float tMin = 0.0f;
	float tMax = 100000.0f;

	glm::vec3 OBBposition_worldspace(ModelMatrix[3].x, ModelMatrix[3].y, ModelMatrix[3].z);

	glm::vec3 delta = OBBposition_worldspace - ray_origin;

	// Test intersection with the 2 planes perpendicular to the OBB's X axis
	{
		glm::vec3 xaxis(ModelMatrix[0].x, ModelMatrix[0].y, ModelMatrix[0].z);
		float e = glm::dot(xaxis, delta);
		float f = glm::dot(ray_direction, xaxis);

		if ( fabs(f) > 0.001f ){ // Standard case

			float t1 = (e+aabb_min.x)/f; // Intersection with the "left" plane
			float t2 = (e+aabb_max.x)/f; // Intersection with the "right" plane
			// t1 and t2 now contain distances betwen ray origin and ray-plane intersections

			// We want t1 to represent the nearest intersection, 
			// so if it's not the case, invert t1 and t2
			if (t1>t2){
				float w=t1;t1=t2;t2=w; // swap t1 and t2
			}

			// tMax is the nearest "far" intersection (amongst the X,Y and Z planes pairs)
			if ( t2 < tMax )
				tMax = t2;
			// tMin is the farthest "near" intersection (amongst the X,Y and Z planes pairs)
			if ( t1 > tMin )
				tMin = t1;

			// And here's the trick :
			// If "far" is closer than "near", then there is NO intersection.
			// See the images in the tutorials for the visual explanation.
			if (tMax < tMin )
				return false;

		}else{ // Rare case : the ray is almost parallel to the planes, so they don't have any "intersection"
			if(-e+aabb_min.x > 0.0f || -e+aabb_max.x < 0.0f)
				return false;
		}
	}


	// Test intersection with the 2 planes perpendicular to the OBB's Y axis
	// Exactly the same thing than above.
	{
		glm::vec3 yaxis(ModelMatrix[1].x, ModelMatrix[1].y, ModelMatrix[1].z);
		float e = glm::dot(yaxis, delta);
		float f = glm::dot(ray_direction, yaxis);

		if ( fabs(f) > 0.001f ){

			float t1 = (e+aabb_min.y)/f;
			float t2 = (e+aabb_max.y)/f;

			if (t1>t2){float w=t1;t1=t2;t2=w;}

			if ( t2 < tMax )
				tMax = t2;
			if ( t1 > tMin )
				tMin = t1;
			if (tMin > tMax)
				return false;

		}else{
			if(-e+aabb_min.y > 0.0f || -e+aabb_max.y < 0.0f)
				return false;
		}
	}


	// Test intersection with the 2 planes perpendicular to the OBB's Z axis
	// Exactly the same thing than above.
	{
		glm::vec3 zaxis(ModelMatrix[2].x, ModelMatrix[2].y, ModelMatrix[2].z);
		float e = glm::dot(zaxis, delta);
		float f = glm::dot(ray_direction, zaxis);

		if ( fabs(f) > 0.001f ){

			float t1 = (e+aabb_min.z)/f;
			float t2 = (e+aabb_max.z)/f;

			if (t1>t2){float w=t1;t1=t2;t2=w;}

			if ( t2 < tMax )
				tMax = t2;
			if ( t1 > tMin )
				tMin = t1;
			if (tMin > tMax)
				return false;

		}else{
			if(-e+aabb_min.z > 0.0f || -e+aabb_max.z < 0.0f)
				return false;
		}
	}

	intersection_distance = tMin;
	return true;

}


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
	MDL::DestroyShaders();
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

	mPointManager.SetBillboardTexture(std::filesystem::current_path() / "res" / "img" / "soundobj.png", 6);

	BinModel::InitShaders();
	MDL::InitShaders();

	mDoorModels.reserve(14);

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

void LEditorScene::UpdateRenderers(){
	std::cout << "calling update renderers. this should only happen a few times!" << std::endl;
	mPathRenderer.mPaths.clear();
	mPointManager.mBillboards.clear();
	for(auto room : mCurrentRooms){
		if(!room.expired() && Initialized)
		{
			auto curRoom = room.lock();

			curRoom->ForEachChildOfType<LBGRenderDOMNode>(EDOMNodeType::BGRender, [&](auto node){
					if(!node->GetIsRendered()) return;
					switch (node->GetNodeType())
					{
					case EDOMNodeType::RoomData:
						{
							glm::vec4 color = (curRoom->GetRoomNumber() == mSelectedRoomNumber ? glm::vec4(0.0f, 1.0f, 0.0f, 1.0f) : glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

							glm::vec3 min = static_cast<LRoomDataDOMNode*>(node.get())->GetMin();
							glm::vec3 max = static_cast<LRoomDataDOMNode*>(node.get())->GetMax();
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
						}
						break;
					case EDOMNodeType::Path:
						{
							std::vector<CPathPoint> path;
							
							auto points = node->template GetChildrenOfType<LPathPointDOMNode>(EDOMNodeType::PathPoint);
							
							for(auto& point : points){
								path.push_back(CPathPoint(point->GetPosition(), static_cast<LPathDOMNode*>(node.get())->mPathColor, 12800));
							}
							
							mPathRenderer.mPaths.push_back(path);
						}
						break;
					case EDOMNodeType::Generator:
						if (node->GetName() == "elice"){
							mPointManager.mBillboards.push_back({node->GetPosition(), 51200, 0, false, false});
						} else if(node->GetName() == "elfire"){
							mPointManager.mBillboards.push_back({node->GetPosition(), 51200, 1, false, false});
						} else if(node->GetName() == "elwater") {
							mPointManager.mBillboards.push_back({node->GetPosition(), 51200, 2, false, false});
						}
						break;
					case EDOMNodeType::Event:
						mPointManager.mBillboards.push_back({node->GetPosition(), 51200, 3, false, false});
						break;
					case EDOMNodeType::Observer:
						if(node->GetName() == "Sound Effect Player"){
							mPointManager.mBillboards.push_back({node->GetPosition(), 51200, 6, false, false});
						} else {
							mPointManager.mBillboards.push_back({node->GetPosition(), 51200, 4, false, false});
						}
						break;
					//case EDOMNodeType::Object:
					//	break;
					//case EDOMNodeType::Enemy:
					//	mPointManager.mBillboards.push_back({node->GetPosition(), 51200, 5, false, false});
					//	break;
				}
			});

		}
	}
	mPointManager.UpdateData();
	mPathRenderer.UpdateData();
}

// This whole thing is so so SO awful.

void LEditorScene::RenderSubmit(uint32_t m_width, uint32_t m_height){
	if (m_height == 0)
		m_height = 1;

	Camera.AspectRatio = float(m_width) / float(m_height);

	glm::mat4 view = Camera.GetViewMatrix();
	glm::mat4 proj = Camera.GetProjectionMatrix();

	J3DUniformBufferObject::SetProjAndViewMatrices(&proj, &view);

	std::vector<std::shared_ptr<J3DModelInstance>> renderables;

	for(std::weak_ptr<LRoomDOMNode> room : mCurrentRooms){
		if(!room.expired() && Initialized)
		{
			std::shared_ptr<LRoomDOMNode> roomLocked = room.lock();
			//vrball01.bmd
			if(roomLocked->GetSkyboxEnabled() && mSkyBox != nullptr){
				mSkyBox->SetTranslation(roomLocked->GetPosition());
				mSkyBox->SetRotation({0,0,0});
				mSkyBox->SetScale({15,15,15});
				renderables.push_back(mSkyBox);
				break;
			}
		}
	}

	glFrontFace(GL_CW);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	for(std::weak_ptr<LDoorDOMNode> doorRef : mRoomDoors){
		if (std::shared_ptr<LDoorDOMNode> door = doorRef.lock())
		{
			EDoorModel doorType = door->GetModel();
			if (doorType == EDoorModel::None)
				continue;

			// Construct transform matrix...
			glm::mat4 doorMat = glm::identity<glm::mat4>();

			// Translation. We need the translation from the transform matrix (column at [3]) as well as the Y scale (float at [1][1])
			doorMat = glm::translate(doorMat, glm::vec3((*door->GetMat())[3]) - glm::vec3(0, (*door->GetMat())[1][1] / 2.f, 0));

			// Rotation is based on the door's orientation type.
			if (door->GetOrientation() == EDoorOrientation::Side_Facing)
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

	for(std::weak_ptr<LRoomDOMNode> roomRef : mCurrentRooms){

		glm::mat4 identity = glm::identity<glm::mat4>();
		for (std::shared_ptr<BinModel> room : mRoomModels)
		{
			room->Draw(&identity);
		}
		
		std::shared_ptr<LRoomDOMNode> curRoom;

		if((curRoom = roomRef.lock()) && Initialized)
		{

			curRoom->ForEachChildOfType<LBGRenderDOMNode>(EDOMNodeType::BGRender, [&](auto node){
					if(!node->GetIsRendered()) return;
					
					//TODO: Render MDL if type is character

					switch (node->GetNodeType())
					{
					case EDOMNodeType::Furniture:
						if(mRoomFurniture.count(std::static_pointer_cast<LFurnitureDOMNode>(node)->GetModelName()) == 0)
						{
							mCubeManager.render(node->GetMat());
						} else {
							mRoomFurniture[std::static_pointer_cast<LFurnitureDOMNode>(node)->GetModelName()]->Draw(node->GetMat());
						}
					case EDOMNodeType::Character:
					case EDOMNodeType::Enemy:
					case EDOMNodeType::Observer:
					case EDOMNodeType::Generator:
					case EDOMNodeType::Key:
						if(mActorModels.contains(node->GetName())){
							if(mMaterialAnimations.contains(node->GetName())){
								mActorModels[node->GetName()]->Draw(node->GetMat(), mMaterialAnimations[node->GetName()].get());
							} else {
								mActorModels[node->GetName()]->Draw(node->GetMat(), nullptr);
							}
						}
						break;
					}
			});

		}
	}
	//Draw GL Lines Based thing for room boundaries

	mPointManager.Draw(&Camera);
	mPathRenderer.Draw(&Camera);

	// j3d
	J3DRendering::Render(0, Camera.GetCenter(), view, proj, renderables);
}

bool LEditorScene::HasRoomLoaded(int32_t roomNumber){
	for (std::weak_ptr<LRoomDOMNode>& room : mCurrentRooms)
	{
		if(!room.expired() && room.lock()->GetRoomNumber() == roomNumber) return true;
	}
	return false;
}

void LEditorScene::SetRoom(std::shared_ptr<LRoomDOMNode> room)
{
	// Get the select room's data so we can get the preload list
	std::vector<std::shared_ptr<LRoomDataDOMNode>> roomData = room->GetChildrenOfType<LRoomDataDOMNode>(EDOMNodeType::RoomData);
	mSelectedRoomNumber = room->GetRoomNumber();

	mRoomModels.clear();
	mRoomFurniture.clear();

	mActorModels.clear();
	mMaterialAnimations.clear();

	//This is ensured to exist, but check it anyway
	if(roomData.size() != 0)
	{
		mCurrentRooms = roomData.front()->GetAdjacencyList();
			
		mRoomDoors = roomData.front()->GetDoorList();

		for (std::weak_ptr<LRoomDOMNode>& adjacentRoomRef :  roomData.front()->GetAdjacencyList())
		{
			std::shared_ptr<LRoomDOMNode> adjacentRoom;

			if(!(adjacentRoom = adjacentRoomRef.lock())){
				continue;
			}

			adjacentRoom->ForEachChildOfType<LBGRenderDOMNode>(EDOMNodeType::BGRender, [&](auto node){
				if(node->GetNodeType() == EDOMNodeType::Character || node->GetNodeType() == EDOMNodeType::Enemy || node->GetNodeType() == EDOMNodeType::Observer || node->GetNodeType() == EDOMNodeType::Generator || node->GetNodeType() == EDOMNodeType::Key){
					std::string name = node->GetName();
					
					std::tuple<std::string, std::string, bool> actorRef = LResUtility::GetActorModelFromName(name);

					if(mActorModels.count(name) != 0 && (mMaterialAnimations.count(name) != 0 && std::get<1>(actorRef) != "")) return;

					std::filesystem::path modelPath = std::filesystem::path(OPTIONS.mRootPath) / "files" / "model" / (std::get<0>(actorRef) + ".szp");

					if(!std::get<2>(actorRef) && std::filesystem::exists(modelPath)){
						std::string actorName = std::get<0>(actorRef);
						std::string txpName = std::get<1>(actorRef);

						GCarchive modelArchive;
						if(!GCResourceManager.LoadArchive(modelPath.string().data(), &modelArchive)){
							std::cout << "Unable to load model archive " << modelPath.string() << std::endl;
							return;
						}

						if(mActorModels.count(name) == 0){
							GCarcfile* modelFile = GCResourceManager.GetFile(&modelArchive, std::filesystem::path("model") / (actorName + ".mdl"));

							if(modelFile == nullptr){
								std::cout << "Couldn't find model/" << actorName << ".mdl in archive" << std::endl;
							} else {
								bStream::CMemoryStream modelData((uint8_t*)modelFile->data, modelFile->size, bStream::Endianess::Big, bStream::OpenMode::In);
								mActorModels[name] = std::make_unique<MDL::Model>();
								mActorModels[name]->Load(&modelData);
							}
						}

						if(mMaterialAnimations.count(name) == 0 && txpName != ""){
							GCarcfile* txpFile = GCResourceManager.GetFile(&modelArchive, std::filesystem::path("txp") / (txpName + ".txp"));
							if(txpFile == nullptr){
								std::cout << "Couldn't find txp/" << txpName << ".txp in archive" << std::endl;
							} else {
								std::cout << "Loading txp " << txpName << std::endl;
								bStream::CMemoryStream txpData((uint8_t*)txpFile->data, txpFile->size, bStream::Endianess::Big, bStream::OpenMode::In);
								mMaterialAnimations[name] = std::make_unique<TXP::Animation>();
								mMaterialAnimations[name]->Load(&txpData);
							}
						}
					} else {
						// look in the game archive
						//todo: load archive from memstream
						GCarcfile* gameArchiveModelFile = GCResourceManager.GetFile(&GCResourceManager.mGameArchive, std::filesystem::path("model") / (std::get<0>(actorRef) + ".arc"));
						
						if(gameArchiveModelFile != nullptr){
							GCarchive modelArchive;
							gcInitArchive(&modelArchive, gameArchiveModelFile->ctx);
							
							if(gcLoadArchive(&modelArchive, gameArchiveModelFile->data, gameArchiveModelFile->size) == GC_ERROR_SUCCESS){
								GCarcfile* modelFile = GCResourceManager.GetFile(&modelArchive, std::filesystem::path("model") / (std::get<0>(actorRef) + ".mdl"));

								if(modelFile == nullptr){
									std::cout << "Couldn't find " << std::get<0>(actorRef) << ".mdl in game archive" << std::endl;
								} else {
									std::cout << "loading model from game archive..." << std::endl;
									bStream::CMemoryStream modelData((uint8_t*)modelFile->data, modelFile->size, bStream::Endianess::Big, bStream::OpenMode::In);
									mActorModels[name] = std::make_unique<MDL::Model>();
									mActorModels[name]->Load(&modelData);
								}
								gcFreeArchive(&modelArchive);
							}
						} else {
							std::cout << "Couldn't find model/" << std::get<0>(actorRef) << ".arc in game archive" << std::endl;
						}
					}
				}
			});

			std::shared_ptr<LRoomDataDOMNode> curRoomData = adjacentRoom->GetChildrenOfType<LRoomDataDOMNode>(EDOMNodeType::RoomData).front();

			std::filesystem::path resPath = std::filesystem::path(OPTIONS.mRootPath) / "files" / std::filesystem::path(curRoomData->GetResourcePath()).relative_path();

			if(!std::filesystem::exists(resPath)) continue;
			
			if(resPath.extension() == ".arc")
			{
				GCarchive roomArc;
				if(!GCResourceManager.LoadArchive(resPath.string().data(), &roomArc)){
					std::cout << "Unable to load room archive " << resPath << std::endl;
					continue;
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
				gcFreeArchive(&roomArc);
			} else {
				//If this is happening the map only has room models, no furniture.
				bStream::CFileStream bin(resPath.string(), bStream::Endianess::Big, bStream::OpenMode::In);
				mRoomModels.push_back(std::make_shared<BinModel>(&bin));
			}
		}

	}

	UpdateRenderers();
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

	// Easter egg where luigi occasionally blinks
	if(mActorModels.count("luige") > 0){
		if(mMaterialAnimations["luige"]->GetFrame() < mMaterialAnimations["luige"]->GetFrameCount()-1){
			mMaterialAnimations["luige"]->Update(dt);
		} else {
			if(rand() % 5000 == 1) mMaterialAnimations["luige"]->SetFrame(0);
		}
	}

	if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS && !ImGuizmo::IsUsing()){
		bool foundObject = false;
		glm::vec3 pos, dir;
		ScreenPosToWorldRay(x, y, w, h, Camera.GetViewMatrix(), Camera.GetProjectionMatrix(), pos, dir);

		for(std::weak_ptr<LRoomDOMNode> roomRef : mCurrentRooms){			
			std::shared_ptr<LRoomDOMNode> curRoom;
			if((curRoom = roomRef.lock()) && Initialized)
			{
				curRoom->ForEachChildOfType<LBGRenderDOMNode>(EDOMNodeType::BGRender, [&](auto node){
						if(!node->GetIsRendered() && !foundObject) return;
						
						switch (node->GetNodeType())
						{
						case EDOMNodeType::Character:
						case EDOMNodeType::Enemy:
						case EDOMNodeType::Observer:
						case EDOMNodeType::Generator:
						case EDOMNodeType::Key:
							if(mActorModels.contains(node->GetName())){
								float dist;
								if(TestRayOBBIntersection(pos, dir, mActorModels[node->GetName()]->bbMin, mActorModels[node->GetName()]->bbMax, *node->GetMat(), dist)){
									selection->AddToSelection(node);
									foundObject = true;
								}
							}
							break;
						}
				});

			}
		}

	}
}