#include "RenderGraphRegistry.h"
#include "RenderGraph.h"

namespace HE
{	

RenderGraphTexture* RenderGraphRegistry::GetTexture(RenderGraphTextureHandle handle) const
{
	return renderGraph->textures[handle.GetIndex()];
}

RenderGraphBuffer* RenderGraphRegistry::GetBuffer(RenderGraphBufferHandle handle) const
{
	return renderGraph->buffers[handle.GetIndex()];
}

RenderGraphTexture* RenderGraphRegistry::GetExternalTexture(RenderBackendTextureHandle handle) const
{
	if (renderGraph->externalTextures.find(handle) == renderGraph->externalTextures.end())
	{
		return nullptr;
	}
	return GetTexture(renderGraph->externalTextures[handle]);
}

RenderGraphBuffer* RenderGraphRegistry::GetExternalBuffer(RenderBackendBufferHandle handle) const
{
	if (renderGraph->externalBuffers.find(handle) == renderGraph->externalBuffers.end())
	{
		return nullptr;
	}
	return GetBuffer(renderGraph->externalBuffers[handle]);
}

const RenderBackendTextureDesc& RenderGraphRegistry::GetTextureDesc(RenderGraphTextureHandle handle) const
{
	return GetTexture(handle)->GetDesc();
}

const RenderBackendBufferDesc& RenderGraphRegistry::GetBufferDesc(RenderGraphBufferHandle handle) const
{
	return GetBuffer(handle)->GetDesc();
}

RenderBackendTextureHandle RenderGraphRegistry::GetRenderBackendTexture(RenderGraphTextureHandle handle) const
{
	return GetTexture(handle)->GetRenderBackendTexture();
}

RenderBackendBufferHandle RenderGraphRegistry::GetRenderBackendBuffer(RenderGraphBufferHandle handle) const
{
	return GetBuffer(handle)->GetRenderBackendBuffer();
}

}