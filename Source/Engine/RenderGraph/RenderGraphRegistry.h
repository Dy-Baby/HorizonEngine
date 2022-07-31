#pragma once

#include "RenderGraph/RenderGraphCommon.h"

namespace HE
{

class RenderGraph;
class RenderGraphPass;

class RenderGraphRegistry
{
public:
	RenderGraphRegistry(RenderGraph* renderGraph, RenderGraphPass* pass)
		: renderGraph(renderGraph), pass(pass) {}
	RenderGraphTexture* GetTexture(RenderGraphTextureHandle handle) const;
	RenderGraphBuffer* GetBuffer(RenderGraphBufferHandle handle) const;
	RenderGraphTexture* GetExternalTexture(RenderBackendTextureHandle handle) const;
	RenderGraphBuffer* GetExternalBuffer(RenderBackendBufferHandle handle) const;
	const RenderBackendTextureDesc& GetTextureDesc(RenderGraphTextureHandle handle) const;
	const RenderBackendBufferDesc& GetBufferDesc(RenderGraphBufferHandle handle) const;
	RenderBackendTextureHandle GetRenderBackendTexture(RenderGraphTextureHandle handle) const;
	RenderBackendBufferHandle GetRenderBackendBuffer(RenderGraphBufferHandle handle) const;
private:
	RenderGraph* const renderGraph;
	RenderGraphPass* const pass;
};

}
