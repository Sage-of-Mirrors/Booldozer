#pragma once

const char* UGridVertexShader = 
	"#version 460\n\n"
	
	"// Input attributes\n"
	"layout(location = 0) in vec3 aPos;\n"
	"layout(location = 1) in vec3 aTex;\n\n"

	"// Output to Fragment Shader\n"
	"out vec2 oTex;\n"
	"out vec2 oTexCenter;\n\n"

	"uniform mat4 uModelMtx;\n"
	"uniform mat4 uViewMtx;\n"
	"uniform mat4 uProjMtx;\n"
	"uniform float uTexScale;\n\n"

	"void main()\n"
	"{\n"
	"\toTex = aTex.xy * uTexScale;\n"
	"\toTexCenter = vec2(uTexScale / 2, uTexScale / 2);\n"
	"\tmat4 MVP = uProjMtx * uViewMtx * uModelMtx;\n\n"

	"\tgl_Position = MVP * vec4(aPos, 1);\n"
	"}\n";

const char* UGridFragmentShader =
	"#version 460\n\n"
	
	"// Input from Vertex Shader\n"
	"in vec2 oTex;\n"
	"in vec2 oTexCenter;\n\n"
	
	"// Final pixel color\n"
	"out vec4 PixelColor;\n\n"
	
	"// Texture\n"
	"uniform sampler2D uTexture;\n\n"
	
	"void main()\n"
	"{\n"
	"\tPixelColor = texture(uTexture, oTex);\n"
	"\tPixelColor.a -= distance(oTex, oTexCenter) / 32;\n"
	"\tif (PixelColor.a < 0.01f)\n"
	"\t{\n"
	"\t\tdiscard;\n"
	"\t}\n"
	"}\n";
