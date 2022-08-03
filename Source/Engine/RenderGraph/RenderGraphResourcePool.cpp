#pragma once

#include "RenderGraphResourcePool.h"

namespace HE
{

RenderGraphResourcePool* gRenderGraphResourcePool = new RenderGraphResourcePool();

void RenderGraphResourcePool::Tick()
{
	for (auto& pooledTexture : allocatedTextures)
	{
		pooledTexture.active = false;
	}
	frameCounter++;
}

RenderBackendTextureHandle RenderGraphResourcePool::FindOrCreateTexture(RenderBackend* backend, const RenderBackendTextureDesc* desc, const char* name)
{
	for (auto& pooledTexture : allocatedTextures)
	{
		if (pooledTexture.active)
		{
			continue;
		}
		if (pooledTexture.desc == *desc)
		{
			pooledTexture.active = true;
			return pooledTexture.texture;
		}
	}

	uint32 deviceMask = ~0u;
	RenderBackendTextureHandle texture = RenderBackendCreateTexture(backend, deviceMask, desc, nullptr, name);
	RenderBackendResourceState initialState = RenderBackendResourceState::Undefined;

	RenderGraphPooledTexture pooledTexture = {
		.active = true,
		.initialState = initialState,
		.currentState = initialState,
		.desc = *desc,
		.texture = texture,
	};
	allocatedTextures.emplace_back(pooledTexture);

	return allocatedTextures.back().texture;
}

}
