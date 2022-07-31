#include "RenderBackend.h"

namespace HE
{
	void Tick(RenderBackend* backend)
	{
		backend->Tick(backend->instance);
	}

	void CreateRenderDevices(RenderBackend* backend, PhysicalDeviceID* physicalDeviceIDs, uint32 numDevices, uint32* outDeviceMasks)
	{
		backend->CreateRenderDevices(backend->instance, physicalDeviceIDs, numDevices, outDeviceMasks);
	}

	void DestroyRenderDevices(RenderBackend* backend)
	{
		backend->DestroyRenderDevices(backend->instance);
	}

	RenderBackendSwapChainHandle CreateSwapChain(RenderBackend* backend, uint32 deviceMask, uint64 windowHandle)
	{
		return backend->CreateSwapChain(backend->instance, deviceMask, windowHandle);
	}

	void DestroySwapChain(RenderBackend* backend, RenderBackendSwapChainHandle swapChain)
	{
		backend->DestroySwapChain(backend->instance, swapChain);
	}

	void ResizeSwapChain(RenderBackend* backend, RenderBackendSwapChainHandle swapChain, uint32* width, uint32* height)
	{
		backend->ResizeSwapChain(backend->instance, swapChain, width, height);
	}

	bool PresentSwapChain(RenderBackend* backend, RenderBackendSwapChainHandle swapChain)
	{
		return backend->PresentSwapChain(backend->instance, swapChain);
	}

	RenderBackendTextureHandle GetActiveSwapChainBuffer(RenderBackend* backend, RenderBackendSwapChainHandle swapChain)
	{
		return backend->GetActiveSwapChainBuffer(backend->instance, swapChain);
	}

	RenderBackendBufferHandle CreateBuffer(RenderBackend* backend, uint32 deviceMask, const RenderBackendBufferDesc* desc, const char* name)
	{
		return backend->CreateBuffer(backend->instance, deviceMask, desc, name);
	}

	void ResizeBuffer(RenderBackend* backend, RenderBackendBufferHandle buffer, uint64 size)
	{
		backend->ResizeBuffer(backend->instance, buffer, size);
	}

	void WriteBuffer(RenderBackend* backend, RenderBackendBufferHandle buffer, uint64 offset, void* data, uint64 size)
	{
		backend->WriteBuffer(backend->instance, buffer, offset, data, size);
	}

	void DestroyBuffer(RenderBackend* backend, RenderBackendBufferHandle buffer)
	{
		backend->DestroyBuffer(backend->instance, buffer);
	}

	RenderBackendTextureHandle CreateTexture(RenderBackend* backend, uint32 deviceMask, const RenderBackendTextureDesc* desc, const void* data, const char* name)
	{
		return backend->CreateTexture(backend->instance, deviceMask, desc, data, name);
	}

	void DestroyTexture(RenderBackend* backend, RenderBackendTextureHandle texture)
	{
		backend->DestroyTexture(backend->instance, texture);
	}

	RenderBackendTextureSRVHandle CreateTextureSRV(RenderBackend* backend, uint32 deviceMask, const RenderBackendTextureSRVDesc* desc, const char* name)
	{
		return backend->CreateTextureSRV(backend->instance, deviceMask, desc, name);
	}

	int32 GetTextureSRVDescriptorIndex(RenderBackend* backend, uint32 deviceMask, RenderBackendTextureHandle srv)
	{
		return backend->GetTextureSRVDescriptorIndex(backend->instance, deviceMask, srv);
	}

	RenderBackendTextureUAVHandle CreateTextureUAV(RenderBackend* backend, uint32 deviceMask, const RenderBackendTextureUAVDesc* desc, const char* name)
	{
		return backend->CreateTextureUAV(backend->instance, deviceMask, desc, name);
	}

	int32 GetTextureUAVDescriptorIndex(RenderBackend* backend, uint32 deviceMask, RenderBackendTextureHandle uav)
	{
		return backend->GetTextureUAVDescriptorIndex(backend->instance, deviceMask, uav);
	}

	RenderBackendSamplerHandle CreateSampler(RenderBackend* backend, uint32 deviceMask, const RenderBackendSamplerDesc* desc, const char* name)
	{
		return backend->CreateSampler(backend->instance, deviceMask, desc, name);
	}

	void DestroySampler(RenderBackend* backend, RenderBackendSamplerHandle sampler)
	{
		backend->DestroySampler(backend->instance, sampler);
	}

	RenderBackendShaderHandle CreateShader(RenderBackend* backend, uint32 deviceMask, const RenderBackendShaderDesc* desc, const char* name)
	{
		return backend->CreateShader(backend->instance, deviceMask, desc, name);
	}

	void DestroyShader(RenderBackend* backend, RenderBackendShaderHandle shader)
	{
		backend->DestroyShader(backend->instance, shader);
	}

	void SubmitRenderCommandLists(RenderBackend* backend, RenderCommandList** commandLists, uint32 numCommandLists)
	{
		backend->SubmitRenderCommandLists(backend->instance, commandLists, numCommandLists);
	}

	void GetRenderStatistics(RenderBackend* backend, uint32 deviceMask, RenderStatistics* statistics)
	{
		backend->GetRenderStatistics(backend->instance, deviceMask, statistics);
	}

	RenderBackendAccelerationStructureHandle CreateBottomLevelAS(RenderBackend* backend, uint32 deviceMask, const RenderBackendBottomLevelASDesc* desc, const char* name)
	{
		return backend->CreateBottomLevelAS(backend->instance, deviceMask, desc, name);
	}

	RenderBackendAccelerationStructureHandle CreateTopLevelAS(RenderBackend* backend, uint32 deviceMask, const RenderBackendTopLevelASDesc* desc, const char* name)
	{
		return backend->CreateTopLevelAS(backend->instance, deviceMask, desc, name);
	}
}