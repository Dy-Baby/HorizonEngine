#pragma once

#include "ShaderSystem/ShaderSystemCommon.h"

namespace HE
{

struct Shader
{
	MemoryArena* allocator;
	RenderBackendInstance backend;
	GUID guid;
	RenderBackendShaderHandle handle;
	bool compiled;
};

struct ShaderMacro
{
	std::pmr::string name;
	std::pmr::string value;
};

typedef std::pmr::vector<ShaderMacro> ShaderMacros;

class ShaderLibrary
{
public:
	Shader* LoadShader(const char* filename, const char* entry, const ShaderMacros* macros);
	Shader* ReloadShader();
	void UnloadShader();
private:
	MemoryArena* allocator;
	RenderBackendInstance backend;
	RenderBackendShaderCompilerInterface compilerInterface;
	RenderBackendShaderCompiler compiler;
	Shader* shaders;
};

}