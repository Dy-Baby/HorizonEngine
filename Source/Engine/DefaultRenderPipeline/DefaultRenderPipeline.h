#pragma once

#include "HybridRenderPipelineCommon.h"
#include "SkyAtmosphere.h"

#define HE_DEFUALT_RENDER_PIPELINE_NAME "Horizon Engine Hybrid Render Pipeline"

namespace HE
{

struct HybridRenderPipelineSettings
{
	float renderScale;
	Vector2u shadowMapResolution;
};

class HybridRenderPipeline : public RenderPipeline
{
public:
	HybridRenderPipeline(RenderContext* context);
	void Init();
	void SetupRenderGraph(SceneView* view, RenderGraph* renderGraph) override;
	HybridRenderPipelineSettings settings;
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
	RenderBackendShaderHandle dofShader;
	RenderBackendShaderHandle tonemappingShader;
	RenderBackendShaderHandle fxaaShader;

#if DEBUG_ONLY_RAY_TRACING_ENBALE
	RenderBackendRayTracingPipelineStateHandle rayTracingShadowsPipelineState;
	RenderBackendBufferHandle rayTracingShadowsSBT;

	RenderBackendShaderHandle svgfReprojectCS;
	RenderBackendShaderHandle svgfFilterMomentsCS;
	RenderBackendShaderHandle svgfAtrousCS;
#endif

	SkyAtmosphere* skyAtmosphere;
};

}