#include "RenderBackend.h"

namespace HE
{
	void RenderBackendTick(RenderBackend* backend)
	{
		backend->Tick(backend->instance);
	}

	void RenderBackendCreateRenderDevices(RenderBackend* backend, PhysicalDeviceID* physicalDeviceIDs, uint32 numDevices, uint32* outDeviceMasks)
	{
		backend->CreateRenderDevices(backend->instance, physicalDeviceIDs, numDevices, outDeviceMasks);
	}

	void RenderBackendDestroyRenderDevices(RenderBackend* backend)
	{
		backend->DestroyRenderDevices(backend->instance);
	}

	RenderBackendSwapChainHandle RenderBackendCreateSwapChain(RenderBackend* backend, uint32 deviceMask, uint64 windowHandle)
	{
		return backend->CreateSwapChain(backend->instance, deviceMask, windowHandle);
	}

	void RenderBackendDestroySwapChain(RenderBackend* backend, RenderBackendSwapChainHandle swapChain)
	{
		backend->DestroySwapChain(backend->instance, swapChain);
	}

	void RenderBackendResizeSwapChain(RenderBackend* backend, RenderBackendSwapChainHandle swapChain, uint32* width, uint32* height)
	{
		backend->ResizeSwapChain(backend->instance, swapChain, width, height);
	}

	bool RenderBackendPresentSwapChain(RenderBackend* backend, RenderBackendSwapChainHandle swapChain)
	{
		return backend->PresentSwapChain(backend->instance, swapChain);
	}

	RenderBackendTextureHandle RenderBackendGetActiveSwapChainBuffer(RenderBackend* backend, RenderBackendSwapChainHandle swapChain)
	{
		return backend->GetActiveSwapChainBuffer(backend->instance, swapChain);
	}

	RenderBackendBufferHandle RenderBackendCreateBuffer(RenderBackend* backend, uint32 deviceMask, const RenderBackendBufferDesc* desc, const char* name)
	{
		return backend->CreateBuffer(backend->instance, deviceMask, desc, name);
	}

	void RenderBackendResizeBuffer(RenderBackend* backend, RenderBackendBufferHandle buffer, uint64 size)
	{
		backend->ResizeBuffer(backend->instance, buffer, size);
	}

	void RenderBackendWriteBuffer(RenderBackend* backend, RenderBackendBufferHandle buffer, uint64 offset, void* data, uint64 size)
	{
		backend->WriteBuffer(backend->instance, buffer, offset, data, size);
	}

	void RenderBackendDestroyBuffer(RenderBackend* backend, RenderBackendBufferHandle buffer)
	{
		backend->DestroyBuffer(backend->instance, buffer);
	}

	RenderBackendTextureHandle RenderBackendCreateTexture(RenderBackend* backend, uint32 deviceMask, const RenderBackendTextureDesc* desc, const void* data, const char* name)
	{
		return backend->CreateTexture(backend->instance, deviceMask, desc, data, name);
	}

	void RenderBackendDestroyTexture(RenderBackend* backend, RenderBackendTextureHandle texture)
	{
		backend->DestroyTexture(backend->instance, texture);
	}

	RenderBackendTextureSRVHandle RenderBakendCreateTextureSRV(RenderBackend* backend, uint32 deviceMask, const RenderBackendTextureSRVDesc* desc, const char* name)
	{
		return backend->CreateTextureSRV(backend->instance, deviceMask, desc, name);
	}

	int32 RenderBackendGetTextureSRVDescriptorIndex(RenderBackend* backend, uint32 deviceMask, RenderBackendTextureHandle srv)
	{
		return backend->GetTextureSRVDescriptorIndex(backend->instance, deviceMask, srv);
	}

	RenderBackendTextureUAVHandle RenderBackendCreateTextureUAV(RenderBackend* backend, uint32 deviceMask, const RenderBackendTextureUAVDesc* desc, const char* name)
	{
		return backend->CreateTextureUAV(backend->instance, deviceMask, desc, name);
	}

	int32 RenderBackendGetTextureUAVDescriptorIndex(RenderBackend* backend, uint32 deviceMask, RenderBackendTextureHandle uav)
	{
		return backend->GetTextureUAVDescriptorIndex(backend->instance, deviceMask, uav);
	}

	RenderBackendSamplerHandle RenderBackendCreateSampler(RenderBackend* backend, uint32 deviceMask, const RenderBackendSamplerDesc* desc, const char* name)
	{
		return backend->CreateSampler(backend->instance, deviceMask, desc, name);
	}

	void RenderBackendDestroySampler(RenderBackend* backend, RenderBackendSamplerHandle sampler)
	{
		backend->DestroySampler(backend->instance, sampler);
	}

	RenderBackendShaderHandle RenderBackendCreateShader(RenderBackend* backend, uint32 deviceMask, const RenderBackendShaderDesc* desc, const char* name)
	{
		return backend->CreateShader(backend->instance, deviceMask, desc, name);
	}

	void RenderBackendDestroyShader(RenderBackend* backend, RenderBackendShaderHandle shader)
	{
		backend->DestroyShader(backend->instance, shader);
	}

	void RenderBackendSubmitRenderCommandLists(RenderBackend* backend, RenderCommandList** commandLists, uint32 numCommandLists)
	{
		backend->SubmitRenderCommandLists(backend->instance, commandLists, numCommandLists);
	}

	void RenderBackendGetRenderStatistics(RenderBackend* backend, uint32 deviceMask, RenderStatistics* statistics)
	{
		backend->GetRenderStatistics(backend->instance, deviceMask, statistics);
	}

	RenderBackendAccelerationStructureHandle RenderBackendCreateBottomLevelAS(RenderBackend* backend, uint32 deviceMask, const RenderBackendBottomLevelASDesc* desc, const char* name)
	{
		return backend->CreateBottomLevelAS(backend->instance, deviceMask, desc, name);
	}

	RenderBackendAccelerationStructureHandle RenderBackendCreateTopLevelAS(RenderBackend* backend, uint32 deviceMask, const RenderBackendTopLevelASDesc* desc, const char* name)
	{
		return backend->CreateTopLevelAS(backend->instance, deviceMask, desc, name);
	}
}