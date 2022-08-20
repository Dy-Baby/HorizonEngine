#include "HybridRenderPipeline.h"
#include "PostProcessCommon.h"
#include "RenderEngine/RenderEngine.h"
#include "ECS/ECS.h"

namespace HE
{

static const HybridRenderPipelineSettings hybridRenderPipelineSettings = {

};

HybridRenderPipeline::HybridRenderPipeline(RenderContext* context)
	: allocator(context->arena)
	, renderBackend(context->renderBackend)
	, shaderCompiler(context->shaderCompiler)
	, uiRenderer(context->uiRenderer)
{

}

void HybridRenderPipeline::Init()
{
	RenderBackendSamplerDesc samplerLinearClampDesc = RenderBackendSamplerDesc::CreateLinearClamp(0.0f, -FLOAT_MAX, FLOAT_MAX, 1);
	samplerLinearClamp = RenderBackendCreateSampler(renderBackend, deviceMask, &samplerLinearClampDesc, "SamplerLinearClamp");
	RenderBackendSamplerDesc samplerLinearWarpDesc = RenderBackendSamplerDesc::CreateLinearWarp(0.0f, -FLOAT_MAX, FLOAT_MAX, 1);
	samplerLinearWarp = RenderBackendCreateSampler(renderBackend, deviceMask, &samplerLinearWarpDesc, "SamplerLinearWarp");

	RenderBackendTextureDesc brdfLutDesc = RenderBackendTextureDesc::Create2D(brdfLutSize, brdfLutSize, PixelFormat::RG16Float, TextureCreateFlags::UnorderedAccess | TextureCreateFlags::ShaderResource);
	brdfLut = RenderBackendCreateTexture(renderBackend, deviceMask, &brdfLutDesc, nullptr, "BRDFLut");

	RenderBackendBufferDesc perFrameDataBufferDesc = RenderBackendBufferDesc::CreateByteAddress(sizeof(PerFrameData));
	perFrameDataBuffer = RenderBackendCreateBuffer(renderBackend, deviceMask, &perFrameDataBufferDesc, "PerFrameDataBuffer");

	RenderBackendShaderDesc gbufferShaderDesc;
	gbufferShaderDesc.rasterizationState.cullMode = RasterizationCullMode::None;
	gbufferShaderDesc.rasterizationState.frontFaceCounterClockwise = true;
	gbufferShaderDesc.colorBlendState.numColorAttachments = 3;
	gbufferShaderDesc.depthStencilState = {
		.depthTestEnable = true,
		.depthWriteEnable = true,
		.depthCompareOp = CompareOp::LessOrEqual,
		.stencilTestEnable = false,
	};

	std::vector<uint8> source;
	std::vector<const wchar*> includeDirs;
	std::vector<const wchar*> defines;
	includeDirs.push_back(TEXT("../../../Shaders/HybridRenderPipeline"));

	RenderBackendShaderDesc brdfLutShaderDesc;
	LoadShaderSourceFromFile("../../../Shaders/HybridRenderPipeline/BRDFLut.hsf", source);
	CompileShader(
		shaderCompiler,
		source,
		TEXT("BRDFLutCS"),
		RenderBackendShaderStage::Compute,
		ShaderRepresentation::SPIRV,
		includeDirs,
		defines,
		&brdfLutShaderDesc.stages[(uint32)RenderBackendShaderStage::Compute]);
	brdfLutShaderDesc.entryPoints[(uint32)RenderBackendShaderStage::Compute] = "BRDFLutCS";
	brdfLutShader = RenderBackendCreateShader(renderBackend, deviceMask, &brdfLutShaderDesc, "BRDFLutShader");

	LoadShaderSourceFromFile("../../../Shaders/HybridRenderPipeline/GBuffer.hsf", source);
	CompileShader(
		shaderCompiler,
		source,
		TEXT("GBufferVS"),
		RenderBackendShaderStage::Vertex,
		ShaderRepresentation::SPIRV,
		includeDirs,
		defines,
		&gbufferShaderDesc.stages[(uint32)RenderBackendShaderStage::Vertex]);
	gbufferShaderDesc.entryPoints[(uint32)RenderBackendShaderStage::Vertex] = "GBufferVS";
	CompileShader(
		shaderCompiler, 
		source,
		TEXT("GBufferPS"),
		RenderBackendShaderStage::Pixel,
		ShaderRepresentation::SPIRV,
		includeDirs,
		defines,
		&gbufferShaderDesc.stages[(uint32)RenderBackendShaderStage::Pixel]);
	gbufferShaderDesc.entryPoints[(uint32)RenderBackendShaderStage::Pixel] = "GBufferPS";
	gbufferShader = RenderBackendCreateShader(renderBackend, deviceMask, &gbufferShaderDesc, "GBufferShader");

	RenderBackendShaderDesc lightingShaderDesc;
	LoadShaderSourceFromFile("../../../Shaders/HybridRenderPipeline/Lighting.hsf", source);
	CompileShader(
		shaderCompiler,
		source,
		TEXT("LightingCS"),
		RenderBackendShaderStage::Compute,
		ShaderRepresentation::SPIRV,
		includeDirs,
		defines,
		&lightingShaderDesc.stages[(uint32)RenderBackendShaderStage::Compute]);
	lightingShaderDesc.entryPoints[(uint32)RenderBackendShaderStage::Compute] = "LightingCS";
	lightingShader = RenderBackendCreateShader(renderBackend, deviceMask, &lightingShaderDesc, "LightingShader");

	RenderBackendShaderDesc dofShaderDesc;
	LoadShaderSourceFromFile("../../../Shaders/HybridRenderPipeline/DepthOfField.hsf", source);
	CompileShader(
		shaderCompiler,
		source,
		TEXT("DepthOfFieldCS"),
		RenderBackendShaderStage::Compute,
		ShaderRepresentation::SPIRV,
		includeDirs,
		defines,
		&dofShaderDesc.stages[(uint32)RenderBackendShaderStage::Compute]);
	dofShaderDesc.entryPoints[(uint32)RenderBackendShaderStage::Compute] = "DepthOfFieldCS";
	dofShader = RenderBackendCreateShader(renderBackend, deviceMask, &dofShaderDesc, "DepthOfFieldShader");

	RenderBackendShaderDesc tonemappingShaderDesc;
	LoadShaderSourceFromFile("../../../Shaders/HybridRenderPipeline/Tonemapping.hsf", source);
	CompileShader(
		shaderCompiler, 
		source,
		TEXT("TonemappingCS"),
		RenderBackendShaderStage::Compute,
		ShaderRepresentation::SPIRV,
		includeDirs,
		defines,
		&tonemappingShaderDesc.stages[(uint32)RenderBackendShaderStage::Compute]);
	tonemappingShaderDesc.entryPoints[(uint32)RenderBackendShaderStage::Compute] = "TonemappingCS";
	tonemappingShader = RenderBackendCreateShader(renderBackend, deviceMask, &tonemappingShaderDesc, "TonemappingShader");

	RenderBackendShaderDesc fxaaShaderDesc;
	LoadShaderSourceFromFile("../../../Shaders/HybridRenderPipeline/FXAA.hsf", source);
	CompileShader(
		shaderCompiler,
		source,
		TEXT("FxaaCS"),
		RenderBackendShaderStage::Compute,
		ShaderRepresentation::SPIRV,
		includeDirs,
		defines,
		&fxaaShaderDesc.stages[(uint32)RenderBackendShaderStage::Compute]);
	fxaaShaderDesc.entryPoints[(uint32)RenderBackendShaderStage::Compute] = "FxaaCS";
	fxaaShader = RenderBackendCreateShader(renderBackend, deviceMask, &fxaaShaderDesc, "fxaaShader");

	RenderBackendRayTracingPipelineStateDesc rayTracingShadowsPipelineStateDesc = {
		.maxRayRecursionDepth = 1,
	};

	LoadShaderSourceFromFile("../../../Shaders/HybridRenderPipeline/RayTracingShadows.hsf", source);

	rayTracingShadowsPipelineStateDesc.shaders.push_back(RenderBackendRayTracingShaderDesc{
		.stage = RenderBackendShaderStage::RayGen,
		.entry = "RayTracingShadowsRayGen",
	});
	CompileShader(
		shaderCompiler,
		source,
		TEXT("RayTracingShadowsRayGen"),
		RenderBackendShaderStage::RayGen,
		ShaderRepresentation::SPIRV,
		includeDirs,
		defines,
		&rayTracingShadowsPipelineStateDesc.shaders[0].code);

	rayTracingShadowsPipelineStateDesc.shaders.push_back(RenderBackendRayTracingShaderDesc{
		.stage = RenderBackendShaderStage::Miss,
		.entry = "RayTracingShadowsMiss",
	});
	CompileShader(
		shaderCompiler,
		source,
		TEXT("RayTracingShadowsMiss"),
		RenderBackendShaderStage::Miss,
		ShaderRepresentation::SPIRV,
		includeDirs,
		defines,
		&rayTracingShadowsPipelineStateDesc.shaders[1].code);

	rayTracingShadowsPipelineStateDesc.shaderGroupDescs.resize(2);
	rayTracingShadowsPipelineStateDesc.shaderGroupDescs[0] = RenderBackendRayTracingShaderGroupDesc::CreateRayGen(0);
	rayTracingShadowsPipelineStateDesc.shaderGroupDescs[1] = RenderBackendRayTracingShaderGroupDesc::CreateMiss(1);

	rayTracingShadowsPipelineState = RenderBackendCreateRayTracingPipelineState(renderBackend, deviceMask, &rayTracingShadowsPipelineStateDesc, "rayTracingShadowsPipelineState");
	
	RenderBackendRayTracingShaderBindingTableDesc rayTracingShadowsSBTDesc = {
		.rayTracingPipelineState = rayTracingShadowsPipelineState,
		.numShaderRecords = 0,
	};
	rayTracingShadowsSBT = RenderBackendCreateRayTracingShaderBindingTable(renderBackend, deviceMask, &rayTracingShadowsSBTDesc, "rayTracingShadowsSBT");

	RenderBackendShaderDesc svgfReprojectCSDesc;
	LoadShaderSourceFromFile("../../../Shaders/HybridRenderPipeline/SVGF.hsf", source);
	CompileShader(
		shaderCompiler,
		source,
		TEXT("SVGFReprojectCS"),
		RenderBackendShaderStage::Compute,
		ShaderRepresentation::SPIRV,
		includeDirs,
		defines,
		&svgfReprojectCSDesc.stages[(uint32)RenderBackendShaderStage::Compute]);
	svgfReprojectCSDesc.entryPoints[(uint32)RenderBackendShaderStage::Compute] = "SVGFReprojectCS";
	svgfReprojectCS = RenderBackendCreateShader(renderBackend, deviceMask, &svgfReprojectCSDesc, "SVGFReprojectCS");

	RenderBackendShaderDesc svgfFilterMomentsCSDesc;
	LoadShaderSourceFromFile("../../../Shaders/HybridRenderPipeline/SVGF.hsf", source);
	CompileShader(
		shaderCompiler,
		source,
		TEXT("SVGFFilterMomentsCS"),
		RenderBackendShaderStage::Compute,
		ShaderRepresentation::SPIRV,
		includeDirs,
		defines,
		&svgfFilterMomentsCSDesc.stages[(uint32)RenderBackendShaderStage::Compute]);
	svgfFilterMomentsCSDesc.entryPoints[(uint32)RenderBackendShaderStage::Compute] = "SVGFFilterMomentsCS";
	svgfFilterMomentsCS = RenderBackendCreateShader(renderBackend, deviceMask, &svgfFilterMomentsCSDesc, "SVGFFilterMomentsCS");

	RenderBackendShaderDesc svgfAtrousCSDesc;
	LoadShaderSourceFromFile("../../../Shaders/HybridRenderPipeline/SVGF.hsf", source);
	CompileShader(
		shaderCompiler,
		source,
		TEXT("SVGFAtrousCS"),
		RenderBackendShaderStage::Compute,
		ShaderRepresentation::SPIRV,
		includeDirs,
		defines,
		&svgfAtrousCSDesc.stages[(uint32)RenderBackendShaderStage::Compute]);
	svgfAtrousCSDesc.entryPoints[(uint32)RenderBackendShaderStage::Compute] = "SVGFAtrousCS";
	svgfAtrousCS = RenderBackendCreateShader(renderBackend, deviceMask, &svgfAtrousCSDesc, "SVGFAtrousCS");
	
	SkyAtmosphereConfig config;
	skyAtmosphere = CreateSkyAtmosphere(renderBackend, shaderCompiler, &config);
}

void CooyRenderGraphFinalTextureToCameraTarget(RenderGraph* renderGraph)
{
	RenderGraphBlackboard& blackboard = renderGraph->blackboard;

	renderGraph->AddPass("Present Pass", RenderGraphPassFlags::NeverGetCulled,
	[&](RenderGraphBuilder& builder)
	{
		const auto& perFrameData = blackboard.Get<RenderGraphPerFrameData>();
		const auto& finalTextureData = blackboard.Get<RenderGraphFinalTexture>();
		auto& outputTextureData = blackboard.Get<RenderGraphOutputTexture>();

		auto finalTexture = builder.ReadTexture(finalTextureData.finalTexture, RenderBackendResourceState::CopySrc);
		auto outputTexture = outputTextureData.outputTexture = builder.WriteTexture(outputTextureData.outputTexture, RenderBackendResourceState::CopyDst);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			Offset2D offset = {};
			Extent2D extent = { perFrameData.data.targetResolutionWidth, perFrameData.data.targetResolutionHeight };
			commandList.CopyTexture2D(
				registry.GetRenderBackendTexture(finalTexture),
				offset, 0,
				registry.GetRenderBackendTexture(outputTexture),
				offset, 0,
				extent);
			RenderBackendBarrier transition = RenderBackendBarrier(registry.GetRenderBackendTexture(outputTexture), TextureSubresourceRange(0, 1, 0, 1), RenderBackendResourceState::CopyDst, RenderBackendResourceState::Present);
			commandList.Transitions(&transition, 1);
		};
	});
}

void HybridRenderPipeline::SetupRenderGraph(SceneView* view, RenderGraph* renderGraph)
{
	RenderGraphBlackboard& blackboard = renderGraph->blackboard;
	auto& perFrameData = blackboard.CreateSingleton<RenderGraphPerFrameData>();
	auto& gbufferData = blackboard.CreateSingleton<RenderGraphGBuffer>();
	auto& depthBufferData = blackboard.CreateSingleton<RenderGraphDepthBuffer>();
	auto& sceneColorData = blackboard.CreateSingleton<RenderGraphSceneColor>();
	auto& finalTextureData = blackboard.CreateSingleton<RenderGraphFinalTexture>();
	auto& ouptutTextureData = blackboard.CreateSingleton<RenderGraphOutputTexture>();

	static uint32 frameIndex = 0;
	perFrameData.data = {
		.frameIndex = frameIndex,
		.gamma = 2.2,
		.exposure = 1.2,
		.sunDirection = { 0.00, 0.90045, 0.43497 },
		.solarIrradiance = { 10.0f, 10.0f, 10.0f },
		.solarAngularRadius = 0.004675f,
		.sunIlluminanceScale = { 10.0, 10.0, 10.0},
		.cameraPosition = view->camera.position,
		.nearPlane = view->camera.zNear,
		.farPlane = view->camera.zFar,
		.viewMatrix = view->camera.viewMatrix,
		.invViewMatrix = view->camera.invViewMatrix,
		.projectionMatrix = view->camera.projectionMatrix,
		.invProjectionMatrix = view->camera.invProjectionMatrix,
		.viewProjectionMatrix = view->camera.projectionMatrix * view->camera.viewMatrix,
		.invViewProjectionMatrix = view->camera.invViewMatrix * view->camera.invProjectionMatrix,
		.renderResolutionWidth = view->targetWidth,
		.renderResolutionHeight = view->targetHeight,
		.targetResolutionWidth = view->targetWidth,
		.targetResolutionHeight = view->targetHeight,
	};
	frameIndex++;

	perFrameData.buffer = perFrameDataBuffer;
	RenderBackendWriteBuffer(renderBackend, perFrameDataBuffer, 0, &perFrameData, sizeof(PerFrameData));

	ouptutTextureData.outputTexture = renderGraph->ImportExternalTexture(view->target, view->targetDesc, RenderBackendResourceState::Undefined, "Camera Target");
	ouptutTextureData.outputTextureDesc = view->targetDesc;

	RenderTargetClearValue clearColor = { .color = { .uint32 = {} } };
	RenderTargetClearValue clearDepth = { .depthStencil = { .depth = 1.0f } };

	RenderGraphTextureDesc vbuffer0Desc = RenderGraphTextureDesc::Create2D(
		perFrameData.data.renderResolutionWidth,
		perFrameData.data.renderResolutionHeight,
		PixelFormat::R32Uint,
		TextureCreateFlags::ShaderResource | TextureCreateFlags::RenderTarget,
		clearColor);

	RenderGraphTextureDesc gbuffer0Desc = RenderGraphTextureDesc::Create2D(
		perFrameData.data.renderResolutionWidth,
		perFrameData.data.renderResolutionHeight,
		PixelFormat::RGBA32Float,
		TextureCreateFlags::ShaderResource | TextureCreateFlags::RenderTarget,
		clearColor);
	gbufferData.gbuffer0 = renderGraph->CreateTexture(gbuffer0Desc, "GBuffer 0");

	RenderGraphTextureDesc gbuffer1Desc = RenderGraphTextureDesc::Create2D(
		perFrameData.data.renderResolutionWidth,
		perFrameData.data.renderResolutionHeight,
		PixelFormat::RGBA32Float,
		TextureCreateFlags::ShaderResource | TextureCreateFlags::RenderTarget,
		clearColor);
	gbufferData.gbuffer1 = renderGraph->CreateTexture(gbuffer1Desc, "GBuffer 1");

	RenderGraphTextureDesc gbuffer2Desc = RenderGraphTextureDesc::Create2D(
		perFrameData.data.renderResolutionWidth,
		perFrameData.data.renderResolutionHeight,
		PixelFormat::RGBA32Float,
		TextureCreateFlags::ShaderResource | TextureCreateFlags::RenderTarget,
		clearColor);
	gbufferData.gbuffer2 = renderGraph->CreateTexture(gbuffer2Desc, "GBuffer 2");

	RenderGraphTextureDesc gbuffer3Desc = RenderGraphTextureDesc::Create2D(
		perFrameData.data.renderResolutionWidth,
		perFrameData.data.renderResolutionHeight,
		PixelFormat::RGBA32Float,
		TextureCreateFlags::ShaderResource | TextureCreateFlags::RenderTarget,
		clearColor);
	gbufferData.gbuffer3 = renderGraph->CreateTexture(gbuffer2Desc, "GBuffer 3");

	RenderGraphTextureDesc velocityBufferDesc = RenderGraphTextureDesc::Create2D(
		perFrameData.data.renderResolutionWidth,
		perFrameData.data.renderResolutionHeight,
		PixelFormat::RGBA32Float,
		TextureCreateFlags::ShaderResource | TextureCreateFlags::RenderTarget,
		clearColor);
	gbufferData.velocityBuffer = renderGraph->CreateTexture(velocityBufferDesc, "Velocity Buffer");

	RenderGraphTextureDesc depthBufferDesc = RenderGraphTextureDesc::Create2D(
		perFrameData.data.renderResolutionWidth,
		perFrameData.data.renderResolutionHeight,
		PixelFormat::D32Float,
		TextureCreateFlags::ShaderResource | TextureCreateFlags::DepthStencil,
		clearDepth);
	depthBufferData.depthBuffer = renderGraph->CreateTexture(depthBufferDesc, "Depth Buffer");

	RenderGraphTextureDesc sceneColorDesc = RenderGraphTextureDesc::Create2D(
		perFrameData.data.renderResolutionWidth,
		perFrameData.data.renderResolutionHeight,
		PixelFormat::RGBA16Float,
		TextureCreateFlags::ShaderResource | TextureCreateFlags::UnorderedAccess);
	sceneColorData.sceneColor = renderGraph->CreateTexture(sceneColorDesc, "Scene Color");

	RenderGraphTextureDesc finalTextureDesc = RenderGraphTextureDesc::Create2D(
		perFrameData.data.renderResolutionWidth,
		perFrameData.data.renderResolutionHeight,
		PixelFormat::BGRA8Unorm,
		TextureCreateFlags::ShaderResource | TextureCreateFlags::RenderTarget | TextureCreateFlags::UnorderedAccess);
	finalTextureData.finalTexture = renderGraph->CreateTexture(finalTextureDesc, "Final Texture");

	if (renderBRDFLut)
	{
		renderGraph->AddPass("BRDF Lut Pass", RenderGraphPassFlags::Compute | RenderGraphPassFlags::NeverGetCulled,
		[&](RenderGraphBuilder& builder)
		{
			return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
			{
				uint32 dispatchWidth = CEIL_DIV(brdfLutSize, 8);
				uint32 dispatchHeight = CEIL_DIV(brdfLutSize, 8);

				ShaderArguments shaderArguments = {};
				shaderArguments.BindTextureUAV(0, RenderBackendTextureUAVDesc::Create(brdfLut, 0));

				RenderBackendBarrier transitionBefore = RenderBackendBarrier(brdfLut, TextureSubresourceRange(0, 1, 0, 1), RenderBackendResourceState::Undefined, RenderBackendResourceState::UnorderedAccess);
				commandList.Transitions(&transitionBefore, 1);
				commandList.Dispatch2D(
					brdfLutShader,
					shaderArguments,
					dispatchWidth,
					dispatchHeight);
				RenderBackendBarrier transitionAfter = RenderBackendBarrier(brdfLut, TextureSubresourceRange(0, 1, 0, 1), RenderBackendResourceState::UnorderedAccess, RenderBackendResourceState::ShaderResource);
				commandList.Transitions(&transitionAfter, 1);
			};
		});
		renderBRDFLut = false;
	}
/*
	if ()
	{
		renderGraph->AddPass("Equirectangular To Cube Pass", RenderGraphPassFlags::Compute,
		[&](RenderGraphBuilder& builder)
		{
			return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
			{
				uint32 dispatchX = CEIL_DIV(enviromentMapSize, 8);
				uint32 dispatchY = CEIL_DIV(enviromentMapSize, 8);
				uint32 dispatchZ = 6;

				ShaderArguments shaderArguments = {};
				shaderArguments.BindTextureSRV(0, RenderBackendTextureSRVDesc::Create(equirectangularMap));
				shaderArguments.BindTextureUAV(1, RenderBackendTextureUAVDesc::Create(enviromentMap, 0));

				RenderBackendBarrier transitions[2];
				transitions[0] = RenderBackendBarrier(equirectangularMap, TextureSubresourceRange(0, 1, 0, 1), RenderBackendResourceState::Undefined, RenderBackendResourceState::ShaderResource);
				transitions[1] = RenderBackendBarrier(enviromentMap, TextureSubresourceRange(0, 1, 0, 1), RenderBackendResourceState::Undefined, RenderBackendResourceState::UnorderedAccess);
				commandList.Transitions(&transitions, 2);
				commandList.Dispatch(
					equirectangularToCubeShader,
					shaderArguments,
					dispatchX,
					dispatchY,
					dispatchZ);
				transitions[0] = RenderBackendBarrier(brdfLut, TextureSubresourceRange(0, 1, 0, 1), RenderBackendResourceState::UnorderedAccess, RenderBackendResourceState::ShaderResource);
				commandList.Transitions(&transitions, 2);
			};
		});
		renderGraph->AddPass("IBL Diffuse Pass", RenderGraphPassFlags::Compute,
		[&](RenderGraphBuilder& builder)
		{
			return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
			{
				uint32 dispatchWidth = CEIL_DIV(brdfLutSize, 8);
				uint32 dispatchHeight = CEIL_DIV(brdfLutSize, 8);

				ShaderArguments shaderArguments = {};
				shaderArguments.BindTextureUAV(0, RenderBackendTextureUAVDesc::Create(brdfLut, 0));

				RenderBackendBarrier transitionBefore = RenderBackendBarrier(brdfLut, TextureSubresourceRange(0, 1, 0, 1), RenderBackendResourceState::Undefined, RenderBackendResourceState::UnorderedAccess);
				commandList.Transitions(&transitionBefore, 1);
				commandList.Dispatch2D(
					brdfLutShader,
					shaderArguments,
					dispatchWidth,
					dispatchHeight);
				RenderBackendBarrier transitionAfter = RenderBackendBarrier(brdfLut, TextureSubresourceRange(0, 1, 0, 1), RenderBackendResourceState::UnorderedAccess, RenderBackendResourceState::ShaderResource);
				commandList.Transitions(&transitionAfter, 1);
			};
		});
		renderGraph->AddPass("IBL Specular Pass", RenderGraphPassFlags::Compute,
		[&](RenderGraphBuilder& builder)
		{
			return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
			{
				uint32 dispatchWidth = CEIL_DIV(brdfLutSize, 8);
				uint32 dispatchHeight = CEIL_DIV(brdfLutSize, 8);

				ShaderArguments shaderArguments = {};
				shaderArguments.BindTextureUAV(0, RenderBackendTextureUAVDesc::Create(brdfLut, 0));

				commandList.Dispatch2D(
					brdfLutShader,
					shaderArguments,
					dispatchWidth,
					dispatchHeight);
			};
		});
	}
	*/

	renderGraph->AddPass("GBuffer Pass", RenderGraphPassFlags::Raster,
	[&](RenderGraphBuilder& builder)
	{
		const auto& perFrameData = blackboard.Get<RenderGraphPerFrameData>();
		auto& gbufferData = blackboard.Get<RenderGraphGBuffer>();
		auto& depthBufferData = blackboard.Get<RenderGraphDepthBuffer>();
		
		auto gbuffer0 = gbufferData.gbuffer0 = builder.WriteTexture(gbufferData.gbuffer0, RenderBackendResourceState::RenderTarget);
		auto gbuffer1 = gbufferData.gbuffer1 = builder.WriteTexture(gbufferData.gbuffer1, RenderBackendResourceState::RenderTarget);
		auto gbuffer2 = gbufferData.gbuffer2 = builder.WriteTexture(gbufferData.gbuffer2, RenderBackendResourceState::RenderTarget);
		auto gbuffer3 = gbufferData.gbuffer3 = builder.WriteTexture(gbufferData.gbuffer3, RenderBackendResourceState::RenderTarget);
		auto velocityBuffer = gbufferData.velocityBuffer = builder.WriteTexture(gbufferData.velocityBuffer, RenderBackendResourceState::RenderTarget);
		auto depthBuffer = depthBufferData.depthBuffer = builder.WriteTexture(depthBufferData.depthBuffer, RenderBackendResourceState::DepthStencil);

		builder.BindColorTarget(0, gbuffer0, RenderTargetLoadOp::Clear, RenderTargetStoreOp::Store);
		builder.BindColorTarget(1, gbuffer1, RenderTargetLoadOp::Clear, RenderTargetStoreOp::Store);
		builder.BindColorTarget(2, gbuffer2, RenderTargetLoadOp::Clear, RenderTargetStoreOp::Store);
		builder.BindColorTarget(3, gbuffer3, RenderTargetLoadOp::Clear, RenderTargetStoreOp::Store);
		builder.BindColorTarget(4, velocityBuffer, RenderTargetLoadOp::Clear, RenderTargetStoreOp::Store);
		builder.BindDepthStencilTarget(depthBuffer, RenderTargetLoadOp::Clear, RenderTargetStoreOp::Store);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			RenderBackendViewport viewport = {
				.x = 0.0f,
				.y = 0.0f,
				.width = (float)perFrameData.data.renderResolutionWidth,
				.height = (float)perFrameData.data.renderResolutionHeight,
				.minDepth = 0.0f,
				.maxDepth = 1.0f,
			};
			commandList.SetViewports(&viewport, 1);
			RenderBackendScissor scissor = {
				.left = 0,
				.top = 0,
				.width = perFrameData.data.renderResolutionWidth,
				.height = perFrameData.data.renderResolutionHeight,
			};
			commandList.SetScissors(&scissor, 1);

			for (uint32 i = 0; i < view->scene->numMeshes; i++)
			{
				uint32 numIndices = view->scene->meshes[i].numIndices;

				RenderBackendBufferHandle vertexPosition = view->scene->meshes[i].vertexBuffers[0];
				RenderBackendBufferHandle vertexNormal = view->scene->meshes[i].vertexBuffers[1];
				RenderBackendBufferHandle vertexTangent = view->scene->meshes[i].vertexBuffers[2];
				RenderBackendBufferHandle vertexTexCoord = view->scene->meshes[i].vertexBuffers[3];
				RenderBackendBufferHandle indexBuffer = view->scene->meshes[i].indexBuffer;
				RenderBackendBufferHandle worldMatrixBuffer = view->scene->worldMatrixBuffer;
				RenderBackendBufferHandle prevWorldMatrixBuffer = view->scene->prevWorldMatrixBuffer;
				RenderBackendBufferHandle materialBuffer = view->scene->materialBuffer;

				ShaderArguments shaderArguments = {};
				shaderArguments.BindBuffer(0, perFrameDataBuffer, 0);
				shaderArguments.BindBuffer(1, vertexPosition, 0);
				shaderArguments.BindBuffer(2, vertexNormal, 0);
				shaderArguments.BindBuffer(3, vertexTangent, 0);
				shaderArguments.BindBuffer(4, vertexTexCoord, 0);
				shaderArguments.BindBuffer(5, worldMatrixBuffer, i);
				shaderArguments.BindBuffer(6, prevWorldMatrixBuffer, i);
				shaderArguments.BindBuffer(7, materialBuffer, view->scene->meshes[i].materialID);
				//shaderArguments.BindBuffer(8, indirectBuffer, 0);

				commandList.DrawIndexed(
					gbufferShader,
					shaderArguments,
					indexBuffer,
					numIndices,
					1,
					0,
					0,
					0,
					PrimitiveTopology::TriangleList);
			}
		};
	});

	RenderGraphTextureDesc shadowMaskDesc = RenderGraphTextureDesc::Create2D(
		perFrameData.data.renderResolutionWidth,
		perFrameData.data.renderResolutionHeight,
		PixelFormat::R32Float,
		TextureCreateFlags::ShaderResource | TextureCreateFlags::UnorderedAccess);
	auto shadowMask = renderGraph->CreateTexture(shadowMaskDesc, "Shadow Mask");

	renderGraph->AddPass("Ray Tracing Shadows Pass", RenderGraphPassFlags::RayTrace,
	[&](RenderGraphBuilder& builder)
	{
		const auto& perFrameData = blackboard.Get<RenderGraphPerFrameData>();
		const auto& depthBufferData = blackboard.Get<RenderGraphDepthBuffer>();

		auto depthBuffer = builder.ReadTexture(depthBufferData.depthBuffer, RenderBackendResourceState::ShaderResource);

		shadowMask = builder.WriteTexture(shadowMask, RenderBackendResourceState::UnorderedAccess);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			uint32 width = perFrameData.data.renderResolutionWidth;
			uint32 height = perFrameData.data.renderResolutionHeight;

			ShaderArguments shaderArguments = {};
			shaderArguments.BindBuffer(0, perFrameDataBuffer, 0);
			shaderArguments.BindAS(1, view->scene->topLevelAS);
			shaderArguments.BindTextureSRV(2, RenderBackendTextureSRVDesc::Create(registry.GetRenderBackendTexture(depthBuffer)));
			shaderArguments.BindTextureUAV(3, RenderBackendTextureUAVDesc::Create(registry.GetRenderBackendTexture(shadowMask), 0)); 
			
			commandList.TraceRays(
				rayTracingShadowsPipelineState,
				rayTracingShadowsSBT,
				shaderArguments,
				width,
				height,
				1);
		};
	});

#if 0
	float filterIterations = 4;
	float feedbackTap = 1;
	float phiColor = 10.0;
	float phiNormal = 128.0;
	float alpha = 0.05;
	float momentsAlpha = 0.2;

	auto svgfIllumination = renderGraph->CreateTexture(RenderGraphTextureDesc::Create2D(
		perFrameData.data.renderResolutionWidth,
		perFrameData.data.renderResolutionHeight,
		PixelFormat::RGBA32Float,
		TextureCreateFlags::ShaderResource | TextureCreateFlags::UnorderedAccess),
		"SVGF Illumination");

	auto svgfMoments = renderGraph->CreateTexture(RenderGraphTextureDesc::Create2D(
		perFrameData.data.renderResolutionWidth,
		perFrameData.data.renderResolutionHeight,
		PixelFormat::RG32Float,
		TextureCreateFlags::ShaderResource | TextureCreateFlags::UnorderedAccess),
		"SVGF Moments");

	auto svgfHistoryLength = renderGraph->CreateTexture(RenderGraphTextureDesc::Create2D(
		perFrameData.data.renderResolutionWidth,
		perFrameData.data.renderResolutionHeight,
		PixelFormat::R16Float,
		TextureCreateFlags::ShaderResource | TextureCreateFlags::UnorderedAccess),
		"SVGF History Length");

	RenderGraphTextureHandle prevLinearDepthBuffer = renderGraph->ImportExternalTexture(prevLinearDepthBufferRB.texture, prevLinearDepthBufferRB.desc, prevLinearDepthBufferRB.initialState, "Prev Linear Depth Buffer");
	RenderGraphTextureHandle prevIllum             = renderGraph->ImportExternalTexture(prevIllumRB.texture, prevIllumRB.desc, prevIllumRB.initialState, "Prev Linear Depth Buffer");
	RenderGraphTextureHandle prevMoments           = renderGraph->ImportExternalTexture(prevMomentsRB.texture, prevMomentsRB.desc, prevMomentsRB.initialState, "Prev Linear Depth Buffer");
	RenderGraphTextureHandle prevHistoryLength     = renderGraph->ImportExternalTexture(prevHistoryLengthRB.texture, prevHistoryLengthRB.desc, prevHistoryLengthRB.initialState, "Prev Linear Depth Buffer");

	renderGraph->AddPass("SVGF Reproject Pass", RenderGraphPassFlags::Compute,
	[&](RenderGraphBuilder& builder)
	{
		const auto& perFrameData = blackboard.Get<RenderGraphPerFrameData>();
		const auto& depthBufferData = blackboard.Get<RenderGraphDepthBuffer>();

		auto gbuffer1 = builder.ReadTexture(gbufferData.gbuffer1, RenderBackendResourceState::ShaderResource);
		auto linearDepthBuffer = builder.ReadTexture(gbufferData.gbuffer3, RenderBackendResourceState::ShaderResource);
		auto velocityBuffer = gbufferData.velocityBuffer = builder.ReadTexture(gbufferData.velocityBuffer, RenderBackendResourceState::ShaderResource);

		shadowMask = builder.WriteTexture(shadowMask, RenderBackendResourceState::UnorderedAccess);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			uint32 width = perFrameData.data.renderResolutionWidth;
			uint32 height = perFrameData.data.renderResolutionHeight;

			ShaderArguments shaderArguments = {};
			shaderArguments.BindBuffer(0, perFrameDataBuffer, 0);
			shaderArguments.BindTextureSRV(1, RenderBackendTextureSRVDesc::Create(registry.GetRenderBackendTexture(shadowMask)));
			shaderArguments.BindTextureSRV(2, RenderBackendTextureSRVDesc::Create(registry.GetRenderBackendTexture(gbuffer1)));
			shaderArguments.BindTextureSRV(3, RenderBackendTextureSRVDesc::Create(registry.GetRenderBackendTexture(linearDepthBuffer)));
			shaderArguments.BindTextureSRV(4, RenderBackendTextureSRVDesc::Create(registry.GetRenderBackendTexture(velocityBuffer)));
			shaderArguments.BindTextureSRV(5, RenderBackendTextureSRVDesc::Create(registry.GetRenderBackendTexture(prevIllum)));
			shaderArguments.BindTextureSRV(6, RenderBackendTextureSRVDesc::Create(registry.GetRenderBackendTexture(prevMoments)));
			shaderArguments.BindTextureSRV(7, RenderBackendTextureSRVDesc::Create(registry.GetRenderBackendTexture(prevHistoryLength)));
			shaderArguments.BindTextureSRV(8, RenderBackendTextureSRVDesc::Create(registry.GetRenderBackendTexture(prevLinearDepthBuffer)));
			shaderArguments.BindTextureUAV(9, RenderBackendTextureUAVDesc::Create(registry.GetRenderBackendTexture(svgfIllumination), 0));
			shaderArguments.BindTextureUAV(10, RenderBackendTextureUAVDesc::Create(registry.GetRenderBackendTexture(svgfMoments), 0));
			shaderArguments.BindTextureUAV(11, RenderBackendTextureUAVDesc::Create(registry.GetRenderBackendTexture(svgfHistoryLength), 0));
			shaderArguments.PushConstants(0, phiColor);
			shaderArguments.PushConstants(1, phiNormal);
			shaderArguments.PushConstants(2, alpha);
			shaderArguments.PushConstants(3, momentsAlpha);

			commandList.Dispatch2D(
				svgfReprojectCS,
				shaderArguments,
				width,
				height);
		};
	});

	auto svgfFilteredIllumination = renderGraph->CreateTexture(RenderGraphTextureDesc::Create2D(
		perFrameData.data.renderResolutionWidth,
		perFrameData.data.renderResolutionHeight,
		PixelFormat::RGBA32Float,
		TextureCreateFlags::ShaderResource | TextureCreateFlags::UnorderedAccess),
		"SVGF Filtered Illumination");

	renderGraph->AddPass("SVFG Filter Moments Pass", RenderGraphPassFlags::Compute,
	[&](RenderGraphBuilder& builder)
	{
		const auto& perFrameData = blackboard.Get<RenderGraphPerFrameData>();
		const auto& depthBufferData = blackboard.Get<RenderGraphDepthBuffer>();

		auto gbuffer1 = builder.ReadTexture(gbufferData.gbuffer1, RenderBackendResourceState::ShaderResource);
		auto linearDepthBuffer = builder.ReadTexture(gbufferData.gbuffer3, RenderBackendResourceState::ShaderResource);
		svgfIllumination = builder.ReadTexture(svgfIllumination, RenderBackendResourceState::UnorderedAccess);
		svgfMoments = builder.ReadTexture(svgfMoments, RenderBackendResourceState::UnorderedAccess);
		svgfHistoryLength = builder.ReadTexture(svgfHistoryLength, RenderBackendResourceState::UnorderedAccess);
		
		svgfFilteredIllumination = builder.WriteTexture(svgfFilteredIllumination, RenderBackendResourceState::UnorderedAccess);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			uint32 width = perFrameData.data.renderResolutionWidth;
			uint32 height = perFrameData.data.renderResolutionHeight;

			ShaderArguments shaderArguments = {};
			shaderArguments.BindBuffer(0, perFrameDataBuffer, 0);
			shaderArguments.BindTextureSRV(2, RenderBackendTextureSRVDesc::Create(registry.GetRenderBackendTexture(gbuffer1)));
			shaderArguments.BindTextureSRV(3, RenderBackendTextureSRVDesc::Create(registry.GetRenderBackendTexture(linearDepthBuffer)));
			shaderArguments.BindTextureUAV(9, RenderBackendTextureUAVDesc::Create(registry.GetRenderBackendTexture(svgfIllumination), 0));
			shaderArguments.BindTextureUAV(10, RenderBackendTextureUAVDesc::Create(registry.GetRenderBackendTexture(svgfMoments), 0));
			shaderArguments.BindTextureUAV(11, RenderBackendTextureUAVDesc::Create(registry.GetRenderBackendTexture(svgfHistoryLength), 0));
			shaderArguments.BindTextureUAV(12, RenderBackendTextureUAVDesc::Create(registry.GetRenderBackendTexture(svgfFilteredIllumination), 0));
			shaderArguments.PushConstants(0, phiColor);
			shaderArguments.PushConstants(1, phiNormal);
			shaderArguments.PushConstants(2, alpha);
			shaderArguments.PushConstants(3, momentsAlpha);

			commandList.Dispatch2D(
				svgfFilterMomentsCS,
				shaderArguments,
				width,
				height);
		};
	});

	renderGraph->AddPass("SVFG Atrous Pass", RenderGraphPassFlags::RayTrace,
	[&](RenderGraphBuilder& builder)
	{
		const auto& perFrameData = blackboard.Get<RenderGraphPerFrameData>();
		const auto& depthBufferData = blackboard.Get<RenderGraphDepthBuffer>();

		auto depthBuffer = builder.ReadTexture(depthBufferData.depthBuffer, RenderBackendResourceState::ShaderResource);
		r = builder.ReadTexture(depthBufferData.depthBuffer, RenderBackendResourceState::ShaderResource);
		r = builder.ReadTexture(depthBufferData.depthBuffer, RenderBackendResourceState::ShaderResource);

		shadowMask = builder.WriteTexture(shadowMask, RenderBackendResourceState::UnorderedAccess);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			uint32 width = perFrameData.data.renderResolutionWidth;
			uint32 height = perFrameData.data.renderResolutionHeight;

			for (uint32 iterationIndex = 0; iterationIndex < filterIterations; iterationIndex++)
			{
				uint32 stepSize = (1 << iterationIndex);
				ShaderArguments shaderArguments = {};
				shaderArguments.BindBuffer(0, perFrameDataBuffer, 0);
				shaderArguments.BindAS(1, view->scene->topLevelAS);
				shaderArguments.BindTextureSRV(2, RenderBackendTextureSRVDesc::Create(registry.GetRenderBackendTexture(depthBuffer)));
				shaderArguments.BindTextureUAV(3, RenderBackendTextureUAVDesc::Create(registry.GetRenderBackendTexture(shadowMask), 0));
				commandList.Dispatch2D(
					svgfAtrousCS,
					shaderArguments,
					width,
					height);
			}
		};
	});

	gRenderGraphResourcePool->CacheTexture(prevLinearDepthBufferRB);
	gRenderGraphResourcePool->CacheTexture(prevIllumRB);
	gRenderGraphResourcePool->CacheTexture(prevMomentsRB);
	gRenderGraphResourcePool->CacheTexture(prevHistoryLengthRB);

	renderGraph->ExportTextureDeferred(gbufferData.gbuffer3, &prevLinearDepthBufferRB);
	renderGraph->ExportTextureDeferred(svgfIllumination,     &prevIllumRB);
	renderGraph->ExportTextureDeferred(svgfMoments,          &prevMomentsRB);
	renderGraph->ExportTextureDeferred(svgfHistoryLength,    &prevHistoryLengthRB);

#endif

	renderGraph->AddPass("Lighting Pass", RenderGraphPassFlags::Compute,
	[&](RenderGraphBuilder& builder)
	{
		const auto& perFrameData = blackboard.Get<RenderGraphPerFrameData>();
		const auto& gbufferData = blackboard.Get<RenderGraphGBuffer>();
		// const auto& depthBufferData = blackboard.Get<RenderGraphDepthBuffer>();
		auto& sceneColorData = blackboard.Get<RenderGraphSceneColor>();
		auto gbuffer0 = builder.ReadTexture(gbufferData.gbuffer0, RenderBackendResourceState::ShaderResource);
		auto gbuffer1 = builder.ReadTexture(gbufferData.gbuffer1, RenderBackendResourceState::ShaderResource);
		auto gbuffer2 = builder.ReadTexture(gbufferData.gbuffer2, RenderBackendResourceState::ShaderResource);
		shadowMask = builder.ReadTexture(shadowMask, RenderBackendResourceState::ShaderResource);
		// auto depthBuffer = builder.ReadTexture(depthBufferData.depthBuffer, RenderBackendResourceState::DepthStencilReadOnly);

		auto sceneColor = sceneColorData.sceneColor = builder.WriteTexture(sceneColorData.sceneColor, RenderBackendResourceState::UnorderedAccess);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			uint32 dispatchWidth = CEIL_DIV(perFrameData.data.renderResolutionWidth, 8);
			uint32 dispatchHeight = CEIL_DIV(perFrameData.data.renderResolutionHeight, 8);

			ShaderArguments shaderArguments = {};
			shaderArguments.BindBuffer(0, perFrameDataBuffer, 0);
			shaderArguments.BindTextureSRV(1, RenderBackendTextureSRVDesc::Create(registry.GetRenderBackendTexture(gbuffer0)));
			shaderArguments.BindTextureSRV(2, RenderBackendTextureSRVDesc::Create(registry.GetRenderBackendTexture(gbuffer1)));
			shaderArguments.BindTextureSRV(3, RenderBackendTextureSRVDesc::Create(registry.GetRenderBackendTexture(gbuffer2)));
			shaderArguments.BindTextureSRV(4, RenderBackendTextureSRVDesc::Create(registry.GetRenderBackendTexture(shadowMask)));
			shaderArguments.BindTextureUAV(5, RenderBackendTextureUAVDesc::Create(registry.GetRenderBackendTexture(sceneColor), 0));
			// shaderArguments.BindTextureSRV(5, RenderBackendTextureSRVDesc::Create(registry.GetRenderBackendTexture(depthBuffer)));

			commandList.Dispatch2D(
				lightingShader,
				shaderArguments,
				dispatchWidth,
				dispatchHeight);
		};
	});
	
	// const bool renderSkyAtmosphere = ShouldRenderSkyAtmosphere();
	const bool renderSkyAtmosphere = false;
	if (renderSkyAtmosphere)
	{
		const auto& entities = view->scene->GetEntityManager()->GetView<SkyAtmosphereComponent>();
		for (const auto& entity : entities)
		{
			RenderSky(*renderGraph, *skyAtmosphere, view->scene->GetEntityManager()->GetComponent<SkyAtmosphereComponent>(entity));
			break;
		}
	}

	RenderGraphTextureDesc dofTextureDesc = RenderGraphTextureDesc::Create2D(
		perFrameData.data.targetResolutionWidth,
		perFrameData.data.targetResolutionHeight,
		PixelFormat::RGBA16Float,
		TextureCreateFlags::ShaderResource | TextureCreateFlags::UnorderedAccess);
	auto dofTexture = renderGraph->CreateTexture(dofTextureDesc, "DOF Buffer");

	//renderGraph->AddPass("Depth of Field Pass", RenderGraphPassFlags::Compute,
	//[&](RenderGraphBuilder& builder)
	//{
	//	const auto& perFrameData = blackboard.Get<RenderGraphPerFrameData>();
	//	const auto& sceneColorData = blackboard.Get<RenderGraphSceneColor>();

	//	auto depthBuffer = builder.ReadTexture(depthBufferData.depthBuffer, RenderBackendResourceState::ShaderResource);
	//	auto sceneColor = builder.ReadTexture(sceneColorData.sceneColor, RenderBackendResourceState::ShaderResource);
	//	dofTexture = builder.WriteTexture(dofTexture, RenderBackendResourceState::UnorderedAccess);

	//	return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
	//	{
	//		uint32 dispatchWidth = CEIL_DIV(perFrameData.data.targetResolutionWidth, POST_PROCESS_THREAD_GROUP_SIZE);
	//		uint32 dispatchHeight = CEIL_DIV(perFrameData.data.targetResolutionHeight, POST_PROCESS_THREAD_GROUP_SIZE);

	//		ShaderArguments shaderArguments = {};
	//		shaderArguments.BindBuffer(0, perFrameDataBuffer, 0);
	//		shaderArguments.BindTextureSRV(1, RenderBackendTextureSRVDesc::Create(registry.GetRenderBackendTexture(depthBuffer)));
	//		shaderArguments.BindTextureSRV(2, RenderBackendTextureSRVDesc::Create(registry.GetRenderBackendTexture(sceneColor)));
	//		shaderArguments.BindTextureUAV(3, RenderBackendTextureUAVDesc::Create(registry.GetRenderBackendTexture(dofTexture), 0));

	//		commandList.Dispatch2D(
	//			dofShader,
	//			shaderArguments,
	//			dispatchWidth,
	//			dispatchHeight);
	//	};
	//});

	RenderGraphTextureDesc ldrTextureDesc = RenderGraphTextureDesc::Create2D(
		perFrameData.data.targetResolutionWidth,
		perFrameData.data.targetResolutionHeight,
		PixelFormat::RGBA8Unorm,
		TextureCreateFlags::ShaderResource | TextureCreateFlags::UnorderedAccess);
	auto ldrTexture = renderGraph->CreateTexture(ldrTextureDesc, "LDR Buffer");

	renderGraph->AddPass("Tonemapping Pass", RenderGraphPassFlags::Compute,
	[&](RenderGraphBuilder& builder)
	{
		const auto& perFrameData = blackboard.Get<RenderGraphPerFrameData>();

		auto sceneColor = builder.ReadTexture(sceneColorData.sceneColor, RenderBackendResourceState::ShaderResource);
		ldrTexture = builder.WriteTexture(ldrTexture, RenderBackendResourceState::UnorderedAccess);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			uint32 dispatchWidth = CEIL_DIV(perFrameData.data.targetResolutionWidth, POST_PROCESS_THREAD_GROUP_SIZE);
			uint32 dispatchHeight = CEIL_DIV(perFrameData.data.targetResolutionHeight, POST_PROCESS_THREAD_GROUP_SIZE);

			ShaderArguments shaderArguments = {};
			shaderArguments.BindBuffer(0, perFrameDataBuffer, 0);
			shaderArguments.BindTextureSRV(1, RenderBackendTextureSRVDesc::Create(registry.GetRenderBackendTexture(sceneColor)));
			shaderArguments.BindTextureUAV(2, RenderBackendTextureUAVDesc::Create(registry.GetRenderBackendTexture(ldrTexture), 0));

			commandList.Dispatch2D(
				tonemappingShader,
				shaderArguments,
				dispatchWidth,
				dispatchHeight);
		};
	});

	renderGraph->AddPass("FXAA Pass", RenderGraphPassFlags::Compute,
	[&](RenderGraphBuilder& builder)
	{
		const auto& perFrameData = blackboard.Get<RenderGraphPerFrameData>();
		const auto& sceneColorData = blackboard.Get<RenderGraphSceneColor>();
		auto& finalTextureData = blackboard.Get<RenderGraphFinalTexture>();

		ldrTexture = builder.ReadTexture(ldrTexture, RenderBackendResourceState::ShaderResource);
		auto finalTexture = finalTextureData.finalTexture = builder.WriteTexture(finalTextureData.finalTexture, RenderBackendResourceState::UnorderedAccess);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			uint32 dispatchWidth = CEIL_DIV(perFrameData.data.targetResolutionWidth, POST_PROCESS_THREAD_GROUP_SIZE);
			uint32 dispatchHeight = CEIL_DIV(perFrameData.data.targetResolutionHeight, POST_PROCESS_THREAD_GROUP_SIZE);

			ShaderArguments shaderArguments = {};
			shaderArguments.BindBuffer(0, perFrameDataBuffer, 0);
			shaderArguments.BindTextureSRV(1, RenderBackendTextureSRVDesc::Create(registry.GetRenderBackendTexture(ldrTexture)));
			shaderArguments.BindTextureUAV(2, RenderBackendTextureUAVDesc::Create(registry.GetRenderBackendTexture(finalTexture), 0));

			commandList.Dispatch2D(
				fxaaShader,
				shaderArguments,
				dispatchWidth,
				dispatchHeight);
		};
	});

#if HE_EDITOR
	RenderGizmo();
#endif

	renderGraph->AddPass("ImGui Pass", RenderGraphPassFlags::Raster | RenderGraphPassFlags::SkipRenderPass,
	[&](RenderGraphBuilder& builder)
	{
		auto& finalTextureData = blackboard.Get<RenderGraphFinalTexture>();
		auto finalTexture = finalTextureData.finalTexture = builder.ReadWriteTexture(finalTextureData.finalTexture, RenderBackendResourceState::RenderTarget);

		builder.BindColorTarget(0, finalTexture, RenderTargetLoadOp::Load, RenderTargetStoreOp::Store);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			uiRenderer->Render(commandList, registry.GetRenderBackendTexture(finalTexture));
		};
	});

	CooyRenderGraphFinalTextureToCameraTarget(renderGraph);

	// Export resources
}

}