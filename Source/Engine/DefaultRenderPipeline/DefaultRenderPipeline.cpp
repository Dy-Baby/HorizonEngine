#include "DefaultRenderPipeline.h"
#include "PostProcessing.h"
#include "RenderEngine/RenderEngine.h"

namespace HE
{

struct PerFrameData
{
	float time;

	float gamma;
	float exposure;

	Vector3 sunDirection;
	Vector3 solarIrradiance;
	float solarAngularRadius;
	Vector3 sunIlluminanceScale;

	Vector3 cameraPosition;
	Matrix4x4 viewMatrix;
	Matrix4x4 invViewMatrix;
	Matrix4x4 projectionMatrix;
	Matrix4x4 invProjectionMatrix;
	Matrix4x4 viewProjectionMatrix;
	Matrix4x4 invViewProjectionMatrix;

	uint32 renderResolutioWidth;
	uint32 renderResolutioHeight;
	uint32 targetResolutioWidth;
	uint32 targetResolutioHeight;
};

static const DefaultRenderPipelineSettings defaultRenderPipelineSettings = {

};

DefaultRenderPipeline::DefaultRenderPipeline(RenderContext* context)
	: allocator(context->arena)
	, renderBackend(context->renderBackend)
	, shaderCompiler(context->shaderCompiler)
	, uiRenderer(context->uiRenderer)
{

}

void DefaultRenderPipeline::Init()
{
	RenderBackendSamplerDesc samplerLinearClampDesc = RenderBackendSamplerDesc::CreateLinearClamp(0.0f, -FLOAT_MAX, FLOAT_MAX, 1);
	samplerLinearClamp = CreateSampler(renderBackend, deviceMask, &samplerLinearClampDesc, "SamplerLinearClamp");
	RenderBackendSamplerDesc samplerLinearWarpDesc = RenderBackendSamplerDesc::CreateLinearWarp(0.0f, -FLOAT_MAX, FLOAT_MAX, 1);
	samplerLinearWarp = CreateSampler(renderBackend, deviceMask, &samplerLinearWarpDesc, "SamplerLinearWarp");

	RenderBackendTextureDesc brdfLutDesc = RenderBackendTextureDesc::Create2D(brdfLutSize, brdfLutSize, PixelFormat::RG16Float, TextureCreateFlags::UnorderedAccess | TextureCreateFlags::ShaderResource);
	brdfLut = CreateTexture(renderBackend, deviceMask, &brdfLutDesc, nullptr, "BRDFLut");

	RenderBackendBufferDesc perFrameDataBufferDesc = RenderBackendBufferDesc::CreateByteAddress(sizeof(PerFrameData));
	perFrameDataBuffer = CreateBuffer(renderBackend, deviceMask, &perFrameDataBufferDesc, "PerFrameDataBuffer");

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
	includeDirs.push_back(TEXT("D:/Programming/Projects/Horizon/Shaders/DefaultRenderPipeline"));

	RenderBackendShaderDesc brdfLutShaderDesc;
	LoadShaderSourceFromFile("D:/Programming/Projects/Horizon/Shaders/DefaultRenderPipeline/BRDFLut.hsf", source);
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
	brdfLutShader = CreateShader(renderBackend, deviceMask, &brdfLutShaderDesc, "BRDFLutShader");

	LoadShaderSourceFromFile("D:/Programming/Projects/Horizon/Shaders/DefaultRenderPipeline/GBuffer.hsf", source);
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
	gbufferShader = CreateShader(renderBackend, deviceMask, &gbufferShaderDesc, "GBufferShader");

	RenderBackendShaderDesc lightingShaderDesc;
	LoadShaderSourceFromFile("D:/Programming/Projects/Horizon/Shaders/DefaultRenderPipeline/Lighting.hsf", source);
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
	lightingShader = CreateShader(renderBackend, deviceMask, &lightingShaderDesc, "LightingShader");

	RenderBackendShaderDesc tonemappingShaderDesc;
	LoadShaderSourceFromFile("D:/Programming/Projects/Horizon/Shaders/DefaultRenderPipeline/Tonemapping.hsf", source);
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
	tonemappingShader = CreateShader(renderBackend, deviceMask, &tonemappingShaderDesc, "TonemappingShader");
}

void CooyRenderGraphFinalTextureToCameraTarget(RenderGraph* renderGraph)
{
	RenderGraphBlackboard& blackboard = renderGraph->blackboard;

	renderGraph->AddPass("Present Pass", RenderGraphPassFlags::NeverGetCulled,
	[&](RenderGraphBuilder& builder)
	{
		const auto& viewData = blackboard.Get<RenderGraphViewData>();
		const auto& finalTextureData = blackboard.Get<RenderGraphFinalTexture>();
		auto& outputTextureData = blackboard.Get<RenderGraphOutputTexture>();

		auto finalTexture = builder.ReadTexture(finalTextureData.finalTexture, RenderBackendResourceState::CopySrc);
		auto outputTexture = outputTextureData.outputTexture = builder.WriteTexture(outputTextureData.outputTexture, RenderBackendResourceState::CopyDst);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			Offset2D offset = {};
			Extent2D extent = { viewData.viewWidth, viewData.viewHeight };
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

void DefaultRenderPipeline::SetupRenderGraph(SceneView* view, RenderGraph* renderGraph)
{
	PerFrameData perFrameData = {
		.gamma = 2.2,
		.exposure = 1.4,
		.cameraPosition = view->camera.position,
		.viewMatrix = view->camera.viewMatrix,
		.invViewMatrix = view->camera.invViewMatrix,
		.projectionMatrix = view->camera.projectionMatrix,
		.invProjectionMatrix = view->camera.invProjectionMatrix,
		.viewProjectionMatrix = view->camera.projectionMatrix * view->camera.viewMatrix,
		.invViewProjectionMatrix = view->camera.invViewMatrix * view->camera.invProjectionMatrix,
		.renderResolutioWidth = view->targetWidth,
		.renderResolutioHeight = view->targetHeight,
		.targetResolutioWidth = view->targetWidth,
		.targetResolutioHeight = view->targetHeight,
	};
	WriteBuffer(renderBackend, perFrameDataBuffer, 0, &perFrameData, sizeof(PerFrameData));

	RenderGraphBlackboard& blackboard = renderGraph->blackboard;
	auto& viewData = blackboard.CreateSingleton<RenderGraphViewData>();
	auto& gbufferData = blackboard.CreateSingleton<RenderGraphGBuffer>();
	auto& depthBufferData = blackboard.CreateSingleton<RenderGraphDepthBuffer>();
	auto& sceneColorData = blackboard.CreateSingleton<RenderGraphSceneColor>();
	auto& finalTextureData = blackboard.CreateSingleton<RenderGraphFinalTexture>();
	auto& ouptutTextureData = blackboard.CreateSingleton<RenderGraphOutputTexture>();

	viewData.viewWidth = view->targetWidth;
	viewData.viewHeight = view->targetHeight;
	viewData.renderResolution = { .width = view->targetWidth, .height = view->targetHeight };

	ouptutTextureData.outputTexture = renderGraph->ImportExternalTexture(view->target, view->targetDesc, RenderBackendResourceState::Undefined, "Camera Target");
	ouptutTextureData.outputTextureDesc = view->targetDesc;

	RenderTargetClearValue clearColor = { .color = { .uint32 = {} } };
	RenderTargetClearValue clearDepth = { .depthStencil = { .depth = 1.0f } };

	RenderGraphTextureDesc vbuffer0Desc = RenderGraphTextureDesc::Create2D(
		viewData.renderResolution.width,
		viewData.renderResolution.height,
		PixelFormat::R32Uint,
		TextureCreateFlags::ShaderResource | TextureCreateFlags::RenderTarget,
		clearColor);
	RenderGraphTextureDesc gbuffer0Desc = RenderGraphTextureDesc::Create2D(
		viewData.renderResolution.width,
		viewData.renderResolution.height,
		PixelFormat::RGBA32Float,
		TextureCreateFlags::ShaderResource | TextureCreateFlags::RenderTarget,
		clearColor);
	RenderGraphTextureDesc gbuffer1Desc = RenderGraphTextureDesc::Create2D(
		viewData.renderResolution.width,
		viewData.renderResolution.height,
		PixelFormat::RGBA32Float,
		TextureCreateFlags::ShaderResource | TextureCreateFlags::RenderTarget,
		clearColor);
	RenderGraphTextureDesc gbuffer2Desc = RenderGraphTextureDesc::Create2D(
		viewData.renderResolution.width,
		viewData.renderResolution.height,
		PixelFormat::RGBA32Float,
		TextureCreateFlags::ShaderResource | TextureCreateFlags::RenderTarget,
		clearColor);
	RenderGraphTextureDesc depthBufferDesc = RenderGraphTextureDesc::Create2D(
		viewData.renderResolution.width,
		viewData.renderResolution.height,
		PixelFormat::D32Float,
		TextureCreateFlags::ShaderResource | TextureCreateFlags::DepthStencil,
		clearDepth);
	RenderGraphTextureDesc sceneColorDesc = RenderGraphTextureDesc::Create2D(
		viewData.renderResolution.width,
		viewData.renderResolution.height,
		PixelFormat::RGBA16Float,
		TextureCreateFlags::ShaderResource | TextureCreateFlags::UnorderedAccess);
	RenderGraphTextureDesc finalTextureDesc = RenderGraphTextureDesc::Create2D(
		viewData.renderResolution.width,
		viewData.renderResolution.height,
		PixelFormat::BGRA8Unorm,
		TextureCreateFlags::ShaderResource | TextureCreateFlags::RenderTarget | TextureCreateFlags::UnorderedAccess);

	gbufferData.gbuffer0 = renderGraph->CreateTexture(gbuffer0Desc, "GBuffer 0");
	gbufferData.gbuffer1 = renderGraph->CreateTexture(gbuffer1Desc, "GBuffer 1");
	gbufferData.gbuffer2 = renderGraph->CreateTexture(gbuffer2Desc, "GBuffer 2");
	depthBufferData.depthBuffer = renderGraph->CreateTexture(depthBufferDesc, "Depth Buffer");
	sceneColorData.sceneColor = renderGraph->CreateTexture(sceneColorDesc, "Scene Color");
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

	renderGraph->AddPass("GBuffer Pass", RenderGraphPassFlags::Raster,
	[&](RenderGraphBuilder& builder)
	{
		const auto& viewData = blackboard.Get<RenderGraphViewData>();
		auto& gbufferData = blackboard.Get<RenderGraphGBuffer>();
		auto& depthBufferData = blackboard.Get<RenderGraphDepthBuffer>();
		
		auto gbuffer0 = gbufferData.gbuffer0 = builder.WriteTexture(gbufferData.gbuffer0, RenderBackendResourceState::RenderTarget);
		auto gbuffer1 = gbufferData.gbuffer1 = builder.WriteTexture(gbufferData.gbuffer1, RenderBackendResourceState::RenderTarget);
		auto gbuffer2 = gbufferData.gbuffer2 = builder.WriteTexture(gbufferData.gbuffer2, RenderBackendResourceState::RenderTarget);
		auto depthBuffer = depthBufferData.depthBuffer = builder.WriteTexture(depthBufferData.depthBuffer, RenderBackendResourceState::DepthStencil);

		builder.BindColorTarget(0, gbuffer0, RenderTargetLoadOp::Clear, RenderTargetStoreOp::Store);
		builder.BindColorTarget(1, gbuffer1, RenderTargetLoadOp::Clear, RenderTargetStoreOp::Store);
		builder.BindColorTarget(2, gbuffer2, RenderTargetLoadOp::Clear, RenderTargetStoreOp::Store);
		builder.BindDepthStencilTarget(depthBuffer, RenderTargetLoadOp::Clear, RenderTargetStoreOp::Store);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			RenderBackendViewport viewport = {
				.x = 0.0f,
				.y = 0.0f,
				.width = (float)viewData.renderResolution.width,
				.height = (float)viewData.renderResolution.height,
				.minDepth = 0.0f,
				.maxDepth = 1.0f,
			};
			commandList.SetViewports(&viewport, 1);
			RenderBackendScissor scissor = {
				.left = 0,
				.top = 0,
				.width = viewData.renderResolution.width,
				.height = viewData.renderResolution.height,
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
				shaderArguments.BindBuffer(5, worldMatrixBuffer, 0);
				shaderArguments.BindBuffer(6, prevWorldMatrixBuffer, 0);
				shaderArguments.BindBuffer(7, materialBuffer, 0);
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

	renderGraph->AddPass("Lighting Pass", RenderGraphPassFlags::Compute,
	[&](RenderGraphBuilder& builder)
	{
		const auto& viewData = blackboard.Get<RenderGraphViewData>();
		const auto& gbufferData = blackboard.Get<RenderGraphGBuffer>();
		// const auto& depthBufferData = blackboard.Get<RenderGraphDepthBuffer>();
		auto& sceneColorData = blackboard.Get<RenderGraphSceneColor>();
		auto gbuffer0 = builder.ReadTexture(gbufferData.gbuffer0, RenderBackendResourceState::ShaderResource);
		auto gbuffer1 = builder.ReadTexture(gbufferData.gbuffer1, RenderBackendResourceState::ShaderResource);
		auto gbuffer2 = builder.ReadTexture(gbufferData.gbuffer2, RenderBackendResourceState::ShaderResource);

		// auto depthBuffer = builder.ReadTexture(depthBufferData.depthBuffer, RenderBackendResourceState::DepthStencilReadOnly);
		auto sceneColor = sceneColorData.sceneColor = builder.WriteTexture(sceneColorData.sceneColor, RenderBackendResourceState::UnorderedAccess);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			uint32 dispatchWidth = CEIL_DIV(viewData.renderResolution.width, 8);
			uint32 dispatchHeight = CEIL_DIV(viewData.renderResolution.height, 8);

			ShaderArguments shaderArguments = {};
			shaderArguments.BindBuffer(0, perFrameDataBuffer, 0);
			shaderArguments.BindTextureSRV(1, RenderBackendTextureSRVDesc::Create(registry.GetRenderBackendTexture(gbuffer0)));
			shaderArguments.BindTextureSRV(2, RenderBackendTextureSRVDesc::Create(registry.GetRenderBackendTexture(gbuffer1)));
			shaderArguments.BindTextureSRV(3, RenderBackendTextureSRVDesc::Create(registry.GetRenderBackendTexture(gbuffer2)));
			// shaderArguments.BindTextureSRV(5, RenderBackendTextureSRVDesc::Create(registry.GetRenderBackendTexture(depthBuffer)));
			shaderArguments.BindTextureUAV(4, RenderBackendTextureUAVDesc::Create(registry.GetRenderBackendTexture(sceneColor), 0));

			commandList.Dispatch2D(
				lightingShader,
				shaderArguments,
				dispatchWidth,
				dispatchHeight);
		};
	});
	
	/*const bool renderSkyAtmosphere = ShouldRenderSkyAtmosphere(scene);
	if (renderSkyAtmosphere)
	{
		RenderSkyAtmosphereLookUpTables(renderGraph);
	}*/

#if HE_EDITOR
	RenderGizmo();
#endif

	renderGraph->AddPass("Final Pass", RenderGraphPassFlags::Compute,
	[&](RenderGraphBuilder& builder)
	{
		const auto& viewData = blackboard.Get<RenderGraphViewData>();
		const auto& sceneColorData = blackboard.Get<RenderGraphSceneColor>();
		auto& finalTextureData = blackboard.Get<RenderGraphFinalTexture>();

		auto sceneColor = builder.ReadTexture(sceneColorData.sceneColor, RenderBackendResourceState::ShaderResource);
		auto finalTexture = finalTextureData.finalTexture = builder.WriteTexture(finalTextureData.finalTexture, RenderBackendResourceState::UnorderedAccess);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			uint32 dispatchWidth = CEIL_DIV(viewData.viewWidth, POST_PROCESSING_THREAD_GROUP_SIZE);
			uint32 dispatchHeight = CEIL_DIV(viewData.viewHeight, POST_PROCESSING_THREAD_GROUP_SIZE);

			ShaderArguments shaderArguments = {};
			shaderArguments.BindBuffer(0, perFrameDataBuffer, 0);
			shaderArguments.BindTextureSRV(1, RenderBackendTextureSRVDesc::Create(registry.GetRenderBackendTexture(sceneColor)));
			shaderArguments.BindTextureUAV(2, RenderBackendTextureUAVDesc::Create(registry.GetRenderBackendTexture(finalTexture), 0));

			commandList.Dispatch2D(
				tonemappingShader,
				shaderArguments,
				dispatchWidth,
				dispatchHeight);
		};
	});

	renderGraph->AddPass("ImGui Pass", RenderGraphPassFlags::Raster | RenderGraphPassFlags::SkipRenderPass,
	[&](RenderGraphBuilder& builder)
	{
		auto& finalTextureData = blackboard.Get<RenderGraphFinalTexture>();
		auto finalTexture = finalTextureData.finalTexture = builder.ReadWriteTexture(finalTextureData.finalTexture, RenderBackendResourceState::RenderTarget);

		builder.BindColorTarget(0, finalTexture, RenderTargetLoadOp::Load, RenderTargetStoreOp::Store);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			//RenderBackendBarrier transition = RenderBackendBarrier(registry.GetRenderBackendTexture(finalTexture), TextureSubresourceRange(0, 1, 0, 1), RenderBackendResourceState::Undefined, RenderBackendResourceState::RenderTarget);
			//commandList.Transitions(&transition, 1);

			uiRenderer->Render(commandList, registry.GetRenderBackendTexture(finalTexture));
		};
	});

	CooyRenderGraphFinalTextureToCameraTarget(renderGraph);

	// Export resources
}

}