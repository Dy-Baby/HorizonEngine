#pragma once

#include "Core/Core.h"
#include "RenderGraph/RenderGraph.h"

namespace HE
{

struct RenderGraphViewData
{
	uint32 viewWidth;
	uint32 viewHeight;
	Extent2D renderResolution;
};
RENDER_GRAPH_BLACKBOARD_REGISTER_STRUCT(RenderGraphViewData);

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