#pragma once

#include "RenderGraphResources.h"

namespace HE
{

struct RenderGraphPersistentTexture
{
	bool active;
	RenderBackendTextureHandle texture;
	RenderBackendTextureDesc desc;
    RenderBackendResourceState initialState;
};

struct RenderGraphPersistentBuffer
{
	bool active;
	RenderBackendBufferHandle buffer;
	RenderBackendBufferDesc desc;
	RenderBackendResourceState initialState;
};

class RenderGraphResourcePool
{
public:
	void Tick();
	void CacheTexture(const RenderGraphPersistentTexture& texture);
	RenderBackendTextureHandle FindOrCreateTexture(RenderBackend* backend, const RenderBackendTextureDesc* desc, const char* name);
private:
	friend class RenderGraph;
	std::pmr::vector<RenderGraphPersistentTexture> allocatedTextures;
	uint32 frameCounter = 0;
};

extern RenderGraphResourcePool* gRenderGraphResourcePool;

}
