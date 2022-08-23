#pragma once

#include "Core/Core.h"

import HorizonEngine.Render.RenderGraph;

namespace HE
{

struct PerFrameData
{
	uint32 frameIndex = 0;

	float gamma;
	float exposure;

	Vector3 sunDirection;
	Vector3 solarIrradiance;
	float solarAngularRadius;
	Vector3 sunIlluminanceScale;

	Vector3 cameraPosition;
	float nearPlane;
	float farPlane;

	Matrix4x4 viewMatrix;
	Matrix4x4 invViewMatrix;
	Matrix4x4 projectionMatrix;
	Matrix4x4 invProjectionMatrix;
	Matrix4x4 viewProjectionMatrix;
	Matrix4x4 invViewProjectionMatrix;
	Matrix4x4 prevViewProjectionMatrix;

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
	RenderGraphTextureHandle gbuffer3;
	RenderGraphTextureHandle velocityBuffer;
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