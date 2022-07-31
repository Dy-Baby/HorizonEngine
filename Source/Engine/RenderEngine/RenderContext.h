#pragma once

#include "Core/Core.h"

namespace HE
{

struct RenderBackend;
struct ShaderLibrary;
struct ShaderCompiler;
class RenderCommandList;
class UIRenderer;

struct RenderContext
{
	MemoryArena* arena;
	RenderBackend* renderBackend;
	ShaderCompiler* shaderCompiler;
	UIRenderer* uiRenderer;
	std::vector<RenderCommandList*> commandLists;
	// ShaderLibrary* shaderLibrary;
};

}
