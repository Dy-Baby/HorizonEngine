#pragma once

#include "RenderEngine/RenderEngineCommon.h"

namespace HE
{

struct RenderBackendBufferDesc
{
	static RenderBackendBufferDesc Create(uint32 elementSize, uint32 elementCount, BufferCreateFlags flags)
	{
		return RenderBackendBufferDesc(elementSize, elementCount, flags);
	}
	static RenderBackendBufferDesc CreateIndirect(uint32 elementSize, uint32 elementCount)
	{
		auto flags = BufferCreateFlags::Static | BufferCreateFlags::IndirectArguments | BufferCreateFlags::UnorderedAccess | BufferCreateFlags::ShaderResource;
		return RenderBackendBufferDesc(elementSize, elementCount, flags);
	}
	static RenderBackendBufferDesc CreateByteAddress(uint64 bytes)
	{
		auto flags = BufferCreateFlags::UnorderedAccess | BufferCreateFlags::ShaderResource | BufferCreateFlags::IndexBuffer;
		return RenderBackendBufferDesc(4, (uint32)(bytes >> 2), flags);
	}
	static RenderBackendBufferDesc CreateStructured(uint32 elementSize, uint32 elementCount)
	{
		auto flags = BufferCreateFlags::Static | BufferCreateFlags::UnorderedAccess | BufferCreateFlags::ShaderResource;
		return RenderBackendBufferDesc(elementSize, elementCount, flags);
	}
	static RenderBackendBufferDesc CreateShaderBindingTable(uint64 handleSize, uint32 handleCount)
	{
		auto flags = BufferCreateFlags::ShaderBindingTable | BufferCreateFlags::CpuOnly | BufferCreateFlags::CreateMapped;
		return RenderBackendBufferDesc(4, (uint32)((handleSize * handleCount) >> 2), flags);
	}
	RenderBackendBufferDesc() = default;
	RenderBackendBufferDesc(uint32 elementSize, uint32 elementCount, BufferCreateFlags flags)
		: elementSize(elementSize)
		, elementCount(elementCount)
		, size(elementSize* elementCount)
		, flags(flags) {}

	uint64 size;
	uint32 elementSize;
	uint32 elementCount;
	BufferCreateFlags flags;
};

struct RenderBackendTextureDesc
{
	static RenderBackendTextureDesc Create2D(
		uint32 width,
		uint32 height,
		PixelFormat format,
		TextureCreateFlags flags,
		RenderTargetClearValue clearValue = {},
		uint32 mipLevels = 1,
		uint32 samples = 1)
	{
		return RenderBackendTextureDesc(width, height, 1, mipLevels, 1, samples, TextureType::Texture2D, format, flags, clearValue);
	}
	static RenderBackendTextureDesc Create3D(
		uint32 width,
		uint32 height,
		uint32 depth,
		PixelFormat format,
		TextureCreateFlags flags, 
		uint32 mipLevels = 1,
		uint32 samples = 1)
	{
		return RenderBackendTextureDesc(width, height, depth, mipLevels, 1, samples, TextureType::Texture3D, format, flags, {});
	}
	static RenderBackendTextureDesc CreateCube(
		uint32 sizeInPixels,
		PixelFormat format,
		TextureCreateFlags flags,
		uint32 mipLevels = 1,
		uint32 samples = 1)
	{
		return RenderBackendTextureDesc(sizeInPixels, sizeInPixels, 1, mipLevels, 1, samples, TextureType::TextureCube, format, flags, {});
	}

	RenderBackendTextureDesc() = default;
	RenderBackendTextureDesc(
		uint32 width,
		uint32 height,
		uint32 depth,
		uint32 mipLevels,
		uint32 arraySize,
		uint32 samples,
		TextureType type,
		PixelFormat format,
		TextureCreateFlags flags,
		RenderTargetClearValue clearValue)
		: width(width)
		, height(height)
		, depth(depth)
		, mipLevels(mipLevels)
		, arrayLayers(arraySize)
		, samples(samples)
		, type(type)
		, format(format)
		, flags(flags) 
		, clearValue(clearValue) {}

	bool operator==(const RenderBackendTextureDesc& rhs) const
	{
		return width == rhs.width
			&& height == height
			&& depth == rhs.depth
			&& mipLevels == rhs.mipLevels
			&& arrayLayers == rhs.arrayLayers
			&& samples == rhs.samples
			&& type == rhs.type
			&& format == rhs.format
			&& flags == rhs.flags;
	}

	uint32 width;
	uint32 height;
	uint32 depth;
	uint32 mipLevels;
	uint32 arrayLayers;
	uint32 samples;
	TextureType type;
	PixelFormat format;
	TextureCreateFlags flags;
	RenderTargetClearValue clearValue;
};

struct RenderBackendTextureSRVDesc
{
	static RenderBackendTextureSRVDesc Create(RenderBackendTextureHandle texture)
	{
		return RenderBackendTextureSRVDesc(texture, 0, REMAINING_MIP_LEVELS, 0, REMAINING_ARRAY_LAYERS);
	}
	static RenderBackendTextureSRVDesc Create(RenderBackendTextureHandle texture, uint32 baseMipLevel, uint32 mipLevelCount, uint32 baseArrayLayer, uint32 arrayLayerCount)
	{
		return RenderBackendTextureSRVDesc(texture, baseMipLevel, mipLevelCount, baseArrayLayer, arrayLayerCount);
	}
	static RenderBackendTextureSRVDesc CreateForMipLevel(RenderBackendTextureHandle texture, uint32 mipLevel, uint32 baseArrayLayer = 0, uint32 arrayLayerCount = 1)
	{
		return RenderBackendTextureSRVDesc(texture, mipLevel, 1, baseArrayLayer, arrayLayerCount);
	}
	RenderBackendTextureSRVDesc() {}
	RenderBackendTextureSRVDesc(RenderBackendTextureHandle texture, uint32 baseMipLevel, uint32 numMipLevels, uint32 baseArrayLayer, uint32 numArrayLayers)
		: texture(texture)
		, baseMipLevel(baseMipLevel)
		, numMipLevels(numMipLevels)
		, baseArrayLayer(baseArrayLayer)
		, numArrayLayers(numArrayLayers) {}
	RenderBackendTextureHandle texture;
	uint32 baseMipLevel;
	uint32 numMipLevels;
	uint32 baseArrayLayer;
	uint32 numArrayLayers;
};

struct RenderBackendTextureUAVDesc
{
	static RenderBackendTextureUAVDesc Create(RenderBackendTextureHandle texture, uint32 mipLevel = 0)
	{
		return RenderBackendTextureUAVDesc(texture, mipLevel);
	}
	RenderBackendTextureUAVDesc() = default;
	RenderBackendTextureUAVDesc(RenderBackendTextureHandle texture, uint32 mipLevel)
		: texture(texture), mipLevel(mipLevel) {}
	RenderBackendTextureHandle texture = RenderBackendTextureHandle::NullHandle;
	uint32 mipLevel = 0;
};

struct RenderBackendSamplerDesc
{
	static RenderBackendSamplerDesc CreateLinearClamp(float mipLodBias, float minLod, float maxLod, uint32 maxAnisotropy)
	{
		return RenderBackendSamplerDesc(
			Filter::MinMagMipLinear, 
			TextureAddressMode::Clamp, 
			TextureAddressMode::Clamp,
			TextureAddressMode::Clamp, 
			mipLodBias,
			minLod,
			maxLod,
			maxAnisotropy);
	}
	static RenderBackendSamplerDesc CreateLinearWarp(float mipLodBias, float minLod, float maxLod, uint32 maxAnisotropy)
	{
		return RenderBackendSamplerDesc(
			Filter::MinMagMipLinear,
			TextureAddressMode::Warp,
			TextureAddressMode::Warp,
			TextureAddressMode::Warp,
			mipLodBias,
			minLod,
			maxLod,
			maxAnisotropy);
	}
	RenderBackendSamplerDesc(
		Filter filter,
		TextureAddressMode addressModeU,
		TextureAddressMode addressModeV,
		TextureAddressMode addressModeW,
		float mipLodBias,
		float minLod,
		float maxLod,
		uint32 maxAnisotropy)
		: filter(filter)
		, addressModeU(addressModeU)
		, addressModeV(addressModeV)
		, addressModeW(addressModeW)
		, mipLodBias(mipLodBias)
		, minLod(minLod)
		, maxLod(maxLod)
		, maxAnisotropy(maxAnisotropy) {}
	Filter filter;
	TextureAddressMode addressModeU;
	TextureAddressMode addressModeV;
	TextureAddressMode addressModeW;
	float mipLodBias;
	float minLod;
	float maxLod;
	uint32 maxAnisotropy;
};

struct RenderBackendShaderDesc
{
	RasterizationState rasterizationState;
	DepthStencilState depthStencilState;
	ColorBlendState colorBlendState;
	std::string entryPoints[(uint32)RenderBackendShaderStage::Count] = {};
	ShaderBlob stages[(uint32)RenderBackendShaderStage::Count] = {};
};

struct ShaderArguments
{
	struct TextureSRV
	{
		uint32 slot;
		RenderBackendTextureSRVDesc srv;
	};

	struct TextureUAV
	{
		uint32 slot;
		RenderBackendTextureUAVDesc uav;
	};

	struct Buffer
	{
		uint32 slot;
		RenderBackendBufferHandle handle;
		uint32 offset;
	};

	struct Slot
	{
		int32 type = 0;
		union
		{
			TextureSRV srvSlot;
			TextureUAV uavSlot;
			Buffer bufferSlot;
		};
	};

	void BindTextureSRV(uint32 slot, const RenderBackendTextureSRVDesc& srv)
	{
		slots[slot] = { .type = 1, .srvSlot = { slot, srv } };
	}

	void BindTextureUAV(uint32 slot, const RenderBackendTextureUAVDesc& uav)
	{
		slots[slot] = { .type = 2, .uavSlot = { slot, uav } };
	}

	void BindBuffer(uint32 slot, RenderBackendBufferHandle buffer, uint32 offset)
	{
		slots[slot] = { .type = 3, .bufferSlot = { slot, buffer, offset } };
	}

	void PushConstants(uint32 slot, float value)
	{
		data[slot] = value;
	}

	Slot slots[16];
	float data[16];
};

enum class RenderBackendAccelerationStructureBuildFlags
{
	None = 0,
	AllowUpdate = 1 << 0,
	AllowCompaction = 1 << 1,
	PreferFastTrace = 1 << 2,
	PreferFastBuild = 1 << 3,
	MinimizeMemory = 1 << 4,
};

enum class RenderBackendGeometryInstanceFlags
{
	None = 0,
	TriangleFacingCullDisable = 1 << 0,
	TriangleFrontCounterclockwise = 1 << 1,
	ForceOpaque = 1 << 2,
	ForceNoOpaque = 1 << 3,
};

struct RenderBackendGeometryInstance
{
	Matrix4x4 transformMatrix;
	uint32 instanceID : 24;
	uint32 instanceMask : 8;
	uint32 instanceContributionToHitGroupIndex : 24;
	RenderBackendGeometryInstanceFlags flags : 8;
	RenderBackendAccelerationStructureHandle blas;
};

enum class RenderBackendGeometryType
{
	Triangles = 0,
	AABBs = 1,
};

enum class RenderBackendGeometryFlags
{
	None = 0,
	Opaque = 1 << 0,
	NoDuplicateAnyHitInvocation = 1 << 1,
};

struct RenderBackendGeometryTriangleDesc
{
	uint32 numIndices;
	uint32 numVertices;
	uint32 vertexStride;
	RenderBackendBufferHandle vertexBuffer;
	uint32 vertexOffset;
	RenderBackendBufferHandle indexBuffer;
	uint32 indexOffset;
	RenderBackendBufferHandle transformBuffer;
	uint32 transformOffset;
};

struct RenderBackendGeometryAABBDesc
{
	RenderBackendBufferHandle buffer;
	uint32 offset;
};

struct RenderBackendGeometryDesc
{
	RenderBackendGeometryType type;
	RenderBackendGeometryFlags flags;
	union
	{
		RenderBackendGeometryTriangleDesc triangleDesc;
		RenderBackendGeometryAABBDesc aabbDesc;
	};
};

struct RenderBackendBottomLevelASDesc
{
	RenderBackendAccelerationStructureBuildFlags buildFlags;
	uint32 numGeometries;
	RenderBackendGeometryDesc* geometryDescs;
};

struct RenderBackendTopLevelASDesc
{
	RenderBackendAccelerationStructureBuildFlags buildFlags;
	RenderBackendGeometryFlags geometryFlags;
	uint32 numInstances;
	RenderBackendGeometryInstance* instances;
};

struct RenderBackendAccelerationStructureRange
{
	uint32 numPrimitives;
	uint32 primitiveOffset;
	uint32 firstVertex;
	uint32 transformOffset;
};

//struct GpuProfiler
//{
//	// double buffered
//	static const uint32_t NUM_OF_FRAMES = 3;
//	static const uint32_t MAX_TIMERS = 512;
//
//	Renderer* pRenderer = {};
//	Buffer* pReadbackBuffer[NUM_OF_FRAMES] = {};
//	QueryPool* pQueryPool[NUM_OF_FRAMES] = {};
//	uint64_t* pTimeStamp = NULL;
//	double                mGpuTimeStampFrequency = 0.0;
//
//	uint32_t mProfilerIndex = 0;
//	uint32_t mBufferIndex = 0;
//	uint32_t mCurrentTimerCount = 0;
//	uint32_t mMaxTimerCount = 0;
//	uint32_t mCurrentPoolIndex = 0;
//
//	GpuTimer* pGpuTimerPool = NULL;
//	GpuTimer* pCurrentNode = NULL;
//
//	// MicroProfile
//	char mGroupName[256] = "GPU";
//	ProfileThreadLog* pLog = nullptr;
//
//	bool mReset = true;
//	bool mUpdate = false;
//};

using PhysicalDeviceID = uint32;

#define RENDER_BACKEND_DEVICES_MASK_ALL 0xffffffff
#define RENDER_BACKEND_VERSION HE_MAKE_VERSION(1, 0, 0)

class RenderCommandList;

struct RenderBackend
{
	void* instance;
	void (*Tick)(void* instance);
	void (*CreateRenderDevices)(void* instance, PhysicalDeviceID* physicalDeviceIDs, uint32 numDevices, uint32* outDeviceMasks);
	void (*DestroyRenderDevices)(void* instance);
	RenderBackendSwapChainHandle (*CreateSwapChain)(void* instance, uint32 deviceMask, uint64 windowHandle);
	void (*DestroySwapChain)(void* instance, RenderBackendSwapChainHandle swapChain);
	void (*ResizeSwapChain)(void* instance, RenderBackendSwapChainHandle swapChain, uint32* width, uint32* height);
	bool (*PresentSwapChain)(void* instance, RenderBackendSwapChainHandle swapChain);
	RenderBackendTextureHandle (*GetActiveSwapChainBuffer)(void* instance, RenderBackendSwapChainHandle swapChain);
	RenderBackendBufferHandle (*CreateBuffer)(void* instance, uint32 deviceMask, const RenderBackendBufferDesc* desc, const char* name);
	void (*ResizeBuffer)(void* instance, RenderBackendBufferHandle buffer, uint64 size);
	void (*WriteBuffer)(void* instance, RenderBackendBufferHandle buffer, uint64 offset, void* data, uint64 size);
	void (*DestroyBuffer)(void* instance, RenderBackendBufferHandle buffer);
	RenderBackendTextureHandle (*CreateTexture)(void* instance, uint32 deviceMask, const RenderBackendTextureDesc* desc, const void* data, const char* name);
	void (*DestroyTexture)(void* instance, RenderBackendTextureHandle texture);
	RenderBackendTextureSRVHandle (*CreateTextureSRV)(void* instance, uint32 deviceMask, const RenderBackendTextureSRVDesc* desc, const char* name);
	int32 (*GetTextureSRVDescriptorIndex)(void* instance, uint32 deviceMask, RenderBackendTextureHandle srv);
	RenderBackendTextureUAVHandle (*CreateTextureUAV)(void* instance, uint32 deviceMask, const RenderBackendTextureUAVDesc* desc, const char* name);
	int32 (*GetTextureUAVDescriptorIndex)(void* instance, uint32 deviceMask, RenderBackendTextureHandle uav);
	RenderBackendSamplerHandle (*CreateSampler)(void* instance, uint32 deviceMask, const RenderBackendSamplerDesc* desc, const char* name);
	void (*DestroySampler)(void* instance, RenderBackendSamplerHandle sampler);
	RenderBackendShaderHandle (*CreateShader)(void* instance, uint32 deviceMask, const RenderBackendShaderDesc* desc, const char* name);
	void (*DestroyShader)(void* instance, RenderBackendShaderHandle shader);
	void (*SubmitRenderCommandLists)(void* instance, RenderCommandList** commandLists, uint32 numCommandLists);
	void (*GetRenderStatistics)(void* instance, uint32 deviceMask, RenderStatistics* statistics);
	RenderBackendAccelerationStructureHandle (*CreateBottomLevelAS)(void* instance, uint32 deviceMask, const RenderBackendBottomLevelASDesc* desc, const char* name);
	RenderBackendAccelerationStructureHandle (*CreateTopLevelAS)(void* instance, uint32 deviceMask, const RenderBackendTopLevelASDesc* desc, const char* name);
};

extern void Tick(void* instance);
extern void CreateRenderDevices(RenderBackend* backend, PhysicalDeviceID* physicalDeviceIDs, uint32 numDevices, uint32* outDeviceMasks);
extern void DestroyRenderDevices(RenderBackend* backend);
extern RenderBackendSwapChainHandle CreateSwapChain(RenderBackend* backend, uint32 deviceMask, uint64 windowHandle);
extern void DestroySwapChain(RenderBackend* backend, RenderBackendSwapChainHandle swapChain);
extern void ResizeSwapChain(RenderBackend* backend, RenderBackendSwapChainHandle swapChain, uint32* width, uint32* height);
extern bool PresentSwapChain(RenderBackend* backend, RenderBackendSwapChainHandle swapChain);
extern RenderBackendTextureHandle GetActiveSwapChainBuffer(RenderBackend* backend, RenderBackendSwapChainHandle swapChain);
extern RenderBackendBufferHandle CreateBuffer(RenderBackend* backend, uint32 deviceMask, const RenderBackendBufferDesc* desc, const char* name);
extern void ResizeBuffer(RenderBackend* backend, RenderBackendBufferHandle buffer, uint64 size);
extern void WriteBuffer(RenderBackend* backend, RenderBackendBufferHandle buffer, uint64 offset, void* data, uint64 size);
extern void DestroyBuffer(RenderBackend* backend, RenderBackendBufferHandle buffer);
extern RenderBackendTextureHandle CreateTexture(RenderBackend* backend, uint32 deviceMask, const RenderBackendTextureDesc* desc, const void* data, const char* name);
extern void DestroyTexture(RenderBackend* backend, RenderBackendTextureHandle texture);
extern RenderBackendTextureSRVHandle CreateTextureSRV(RenderBackend* backend, uint32 deviceMask, const RenderBackendTextureSRVDesc* desc, const char* name);
extern int32 GetTextureSRVDescriptorIndex(RenderBackend* backend, uint32 deviceMask, RenderBackendTextureHandle srv);
extern RenderBackendTextureUAVHandle CreateTextureUAV(RenderBackend* backend, uint32 deviceMask, const RenderBackendTextureUAVDesc* desc, const char* name);
extern int32 GetTextureUAVDescriptorIndex(RenderBackend* backend, uint32 deviceMask, RenderBackendTextureHandle uav);
extern RenderBackendSamplerHandle CreateSampler(RenderBackend* backend, uint32 deviceMask, const RenderBackendSamplerDesc* desc, const char* name);
extern void DestroySampler(RenderBackend* backend, RenderBackendSamplerHandle sampler);
extern RenderBackendShaderHandle CreateShader(RenderBackend* backend, uint32 deviceMask, const RenderBackendShaderDesc* desc, const char* name);
extern void DestroyShader(RenderBackend* backend, RenderBackendShaderHandle shader);
extern void SubmitRenderCommandLists(RenderBackend* backend, RenderCommandList** commandLists, uint32 numCommandLists);
extern void GetRenderStatistics(RenderBackend* backend, uint32 deviceMask, RenderStatistics* statistics);
extern RenderBackendAccelerationStructureHandle CreateBottomLevelAS(RenderBackend* backend, uint32 deviceMask, const RenderBackendBottomLevelASDesc* desc, const char* name);
extern RenderBackendAccelerationStructureHandle CreateTopLevelAS(RenderBackend* backend, uint32 deviceMask, const RenderBackendTopLevelASDesc* desc, const char* name);

}
