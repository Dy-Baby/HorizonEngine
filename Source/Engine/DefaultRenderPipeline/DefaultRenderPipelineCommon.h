#pragma once

#include "Core/Core.h"
#include "RenderGraph/RenderGraph.h"

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

	uint32 renderResolutionWidth;
	uint32 renderResolutionHeight;
	uint32 targetResolutionWidth;
	uint32 targetResolutionHeight;
};

struct RenderGraphPerFrameData
{
	PerFrameData data;
	RenderBackendBufferHandle buffer;
};
RENDER_GRAPH_BLACKBOARD_REGISTER_STRUCT(RenderGraphPerFrameData);

struct RenderGraphGBuffer
{
	RenderGraphTextureHandle gbuffer0;
	RenderGraphTextureHandle gbuffer1;
	RenderGraphTextureHandle gbuffer2;
};
RENDER_GRAPH_BLACKBOARD_REGISTER_STRUCT(RenderGraphGBuffer);

struct RenderGraphDepthBuffer
{
	RenderGraphTextureHandle depthBuffer;
};
RENDER_GRAPH_BLACKBOARD_REGISTER_STRUCT(RenderGraphDepthBuffer);

struct RenderGraphSceneColor
{
	RenderGraphTextureHandle sceneColor;
};
RENDER_GRAPH_BLACKBOARD_REGISTER_STRUCT(RenderGraphSceneColor);

struct RenderGraphFinalTexture
{
	RenderGraphTextureHandle finalTexture;
};
RENDER_GRAPH_BLACKBOARD_REGISTER_STRUCT(RenderGraphFinalTexture);

struct RenderGraphOutputTexture
{
	RenderBackendTextureDesc outputTextureDesc;
	RenderGraphTextureHandle outputTexture;
};
RENDER_GRAPH_BLACKBOARD_REGISTER_STRUCT(RenderGraphOutputTexture);

}