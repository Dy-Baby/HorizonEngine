#pragma once

#include "RenderGraph/RenderGraphCommon.h"
#include "RenderGraph/RenderGraphResources.h"

namespace HE
{

class RenderGraphBuilder
{
public:
	RenderGraphBuilder(RenderGraph* renderGraph, RenderGraphPass* pass)
		: renderGraph(renderGraph), pass(pass) {}
	~RenderGraphBuilder() = default;
	RenderGraphTextureHandle CreateTransientTexture(const RenderGraphTextureDesc& desc, const char* name);
	RenderGraphBufferHandle CreateTransientBuffer(const RenderGraphBufferDesc& desc, const char* name);
	RenderGraphTextureHandle ReadTexture(RenderGraphTextureHandle handle, RenderBackendResourceState initalState, const RenderGraphTextureSubresourceRange& range = RenderGraphTextureSubresourceRange::WholeRange);
	RenderGraphTextureHandle WriteTexture(RenderGraphTextureHandle handle, RenderBackendResourceState initalState, const RenderGraphTextureSubresourceRange& range = RenderGraphTextureSubresourceRange::WholeRange);
	RenderGraphTextureHandle ReadWriteTexture(RenderGraphTextureHandle handle, RenderBackendResourceState initalState, const RenderGraphTextureSubresourceRange& range = RenderGraphTextureSubresourceRange::WholeRange);
	RenderGraphBufferHandle ReadBuffer(RenderGraphBufferHandle handle, RenderBackendResourceState initalState);
	RenderGraphBufferHandle WriteBuffer(RenderGraphBufferHandle handle, RenderBackendResourceState initalState);
	RenderGraphBufferHandle ReadWriteBuffer(RenderGraphBufferHandle handle, RenderBackendResourceState initalState);
	void BindColorTarget(uint32 slot, RenderGraphTextureHandle handle, RenderTargetLoadOp loadOp, RenderTargetStoreOp storeOp, uint32 mipLevel = 0, uint32 arraylayer = 0);
	void BindDepthStencilTarget(RenderGraphTextureHandle handle, RenderTargetLoadOp depthLoadOp, RenderTargetStoreOp depthStoreOp, RenderTargetLoadOp stencilLoadOp = RenderTargetLoadOp::DontCare, RenderTargetStoreOp stencilStoreOp = RenderTargetStoreOp::DontCare);
private:
	RenderGraph* const renderGraph;
	RenderGraphPass* const pass;
};

}
