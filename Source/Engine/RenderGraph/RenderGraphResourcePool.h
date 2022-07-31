#pragma once

#include "RenderGraphResources.h"

namespace HE
{

struct RenderGraphPooledTexture
{
	bool active;
    RenderBackendResourceState initialState;
    RenderBackendResourceState currentState;
	RenderBackendTextureDesc desc;
	RenderBackendTextureHandle texture;
};

struct RenderGraphPooledBuffer
{
	bool active;
	RenderBackendResourceState initialState;
	RenderBackendResourceState currentState;
	RenderBackendBufferDesc desc;
	RenderBackendBufferHandle buffer;
};

class RenderGraphResourcePool
{
public:
	void Tick();
	RenderBackendTextureHandle FindOrCreateTexture(RenderBackend* backend, const RenderBackendTextureDesc* desc, const char* name);
private:
	friend class RenderGraph;
	std::pmr::vector<RenderGraphPooledTexture> allocatedTextures;
	uint32 frameCounter = 0;
};

extern RenderGraphResourcePool* gRenderGraphResourcePool;

}
