#pragma once

#include "DefaultRenderPipelineCommon.h"
#include "SkyAtmosphere.h"

#define HE_DEFUALT_RENDER_PIPELINE_NAME "Horizon Engine Default Render Pipeline"

namespace HE
{

struct DefaultRenderPipelineSettings
{
	float renderScale;
	Vector2u shadowMapResolution;
};

class DefaultRenderPipeline : public RenderPipeline
{
public:
	DefaultRenderPipeline(RenderContext* context);
	void Init();
	void SetupRenderGraph(SceneView* view, RenderGraph* renderGraph) override;
	DefaultRenderPipelineSettings settings;
private:
	MemoryArena* allocator;
	UIRenderer* uiRenderer;
	RenderBackend* renderBackend;
	ShaderCompiler* shaderCompiler;
	uint32 deviceMask = ~0u;

	bool renderBRDFLut = true;
	uint32 brdfLutSize = 256;
	RenderBackendTextureHandle brdfLut;

	uint32 enviromentMapSize = 1024;
	RenderBackendTextureHandle equirectangularMap;
	RenderBackendTextureHandle enviromentMap;
	RenderBackendTextureHandle iblDiffuseLut;
	RenderBackendTextureHandle iblSpecularLut;

	RenderBackendSamplerHandle samplerLinearClamp;
	RenderBackendSamplerHandle samplerLinearWarp;

	RenderBackendBufferHandle perFrameDataBuffer;

	RenderBackendShaderHandle brdfLutShader;
	RenderBackendShaderHandle gbufferShader;
	RenderBackendShaderHandle lightingShader;
	RenderBackendShaderHandle tonemappingShader;

	SkyAtmosphere* skyAtmosphere;
};

}