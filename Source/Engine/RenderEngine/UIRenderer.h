#pragma once

#include "Core/Core.h"
#include "RenderEngine/RenderEngineCommon.h"
#include "RenderEngine/RenderBackend.h"
#include "RenderEngine/RenderCommandList.h"
#include "RenderEngine/ShaderCompiler.h"

#include <imgui/imgui.h>
// #include <ImGuizmo/ImGuizmo.h>

namespace HE
{

class UIRenderer
{
public:
	UIRenderer(
		void* window,
		RenderBackend* renderBackend,
		ShaderCompiler* shaderCompiler)
		: window(window)
		, renderBackend(renderBackend)
		, shaderCompiler(shaderCompiler)
		, deviceMask(~0u) {}

	virtual ~UIRenderer() {}
	bool Init();
	void Shutdown();
	// void SetImage(RenderBackendTextureHandle image);
	void BeginFrame();
	void EndFrame();
	void Render(RenderCommandList& commandList, RenderBackendTextureHandle outputTexture);
private:	
	void* window;
	RenderBackend* renderBackend;
	ShaderCompiler* shaderCompiler;
	uint32 deviceMask;

	ImGuiContext* context; 
	RenderBackendTextureHandle defaultFontTexture;
	RenderBackendShaderHandle imguiShader;
	RenderBackendBufferHandle vertexBuffer;
	RenderBackendBufferHandle indexBuffer;
	RenderBackendBufferHandle constantBuffer;
	uint64 vertexBufferSize = 0;
	uint64 indexBufferSize = 0;
	std::vector<ImDrawVert> vertices;
	std::vector<ImDrawIdx> indices;
};

}
