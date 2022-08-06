#pragma once

#include "Core/Core.h"
#include "RenderEngine/RenderBackendHandles.h"
#include "RenderEngine/PixelFormat.h"

namespace HE
{

/**
 * @see: https://pcisig.com/membership/member-companies
 */
enum class GpuVendorID
{
	Unknown = 0,
	NIVIDIA = 0x10DE,
	AMD     = 0x1002,
};

enum
{
	MaxNumRenderDevices = 64,
	MaxNumSwapChainBuffers = 4,
	MaxNumSimultaneousColorRenderTargets = 8,
	MaxNumViewports = 8,
	MaxNumShaderStages = 8,
};

#define NUM_QUEUE_FAMILIES 3

enum class QueueFamily
{
	/// Copy queue
	Copy     = 0,
	/// Asynchronous compute queue
	Compute  = 1,
	/// Graphics queue
	Graphics = 2,
	Count
};
static_assert((uint32)QueueFamily::Graphics == 2, "Default graphics queue index should be 2.");
static_assert((uint32)QueueFamily::Count == NUM_QUEUE_FAMILIES);

enum class RenderBackendPipelineType
{
	Graphics,
	Compute,
	RayTracing,
};
	
enum class RenderBackendShaderStage
{
	Vertex       = 0,
	Pixel        = 1,
	Compute      = 2,
	RayGen       = 3,
	AnyHit       = 4,
	ClosestHit   = 5,
	Miss         = 6,
	Intersection = 7,
	Count,
};
ENUM_CLASS_OPERATORS(RenderBackendShaderStage);

enum class ShaderStageFlags
{
	None         = 0,
	Vertex       = (1 << (int)RenderBackendShaderStage::Vertex),
	Pixel        = (1 << (int)RenderBackendShaderStage::Pixel),
	Compute      = (1 << (int)RenderBackendShaderStage::Compute),
	RayGen       = (1 << (int)RenderBackendShaderStage::RayGen),
	AnyHit       = (1 << (int)RenderBackendShaderStage::AnyHit),
	ClosestHit   = (1 << (int)RenderBackendShaderStage::ClosestHit),
	Miss         = (1 << (int)RenderBackendShaderStage::Miss),
	Intersection = (1 << (int)RenderBackendShaderStage::Intersection),
	All          = ~0,
};
ENUM_CLASS_OPERATORS(ShaderStageFlags);

struct ShaderBlob
{
	uint64 size;
	uint8* data;
};

enum class PrimitiveTopology
{
	PointList     = 0,
	LineList      = 1,
	LineStrip     = 2,
	TriangleList  = 3,
	TriangleStrip = 4,
	TriangleFan   = 5,
};

enum class RasterizationCullMode
{
	None,
	Front,
	Back,
};

enum class RasterizationFillMode
{
	Wireframe,
	Solid,
};

enum class StencilOp
{
	Keep             = 0,
	Zero             = 1,
	Replace          = 2,
	IncreaseAndClamp = 3,
	DecreaseAndClamp = 4,
	Invert           = 5,
	IncreaseAndWrap  = 6,
	DecreaseAndWrap  = 7,
};

enum class CompareOp
{
	Never          = 0,
	Less           = 1,
	Equal          = 2,
	LessOrEqual    = 3,
	Greater        = 4,
	NotEqual       = 5,
	GreaterOrEqual = 6,
	Always         = 7,
};

enum class BlendOp
{
	Add             = 0,
	Subtract        = 1,
	ReverseSubtract = 2,
	Min             = 3,
	Max             = 4,
};

enum class BlendFactor
{
	Zero                  = 0,
	One                   = 1,
	SrcColor              = 2,
	OneMinusSrcColor      = 3,
	DstColor              = 4,
	OneMinusDstColor      = 5,
	SrcAlpha              = 6,
	OneMinusSrcAlpha      = 7,
	DstAlpha              = 8,
	OneMinusDstAlpha      = 9,
	ConstantColor         = 10,
	OneMinusConstantColor = 11,
	ConstantAlpha         = 12,
	OneMinusConstantAlpha = 13,
	SrcAlphaSaturate      = 14,
	Src1Color             = 15,
	OneMinusSrc1Color     = 16,
	Src1Alpha             = 17,
	OneMinusSrc1Alpha     = 18,
};

enum class ColorComponentFlags
{
	R = (1 << 0),
	G = (1 << 1),
	B = (1 << 2),
	A = (1 << 3),
	All = R | G | B | A,
};

struct RasterizationState
{
	RasterizationCullMode cullMode = RasterizationCullMode::None;
	RasterizationFillMode fillMode = RasterizationFillMode::Solid;
	bool  frontFaceCounterClockwise = false;
	bool  depthClampEnable = false;
	float depthBiasConstantFactor = 1.0f;
	float depthBiasSlopeFactor = 1.0f;
};

struct StencilOpState
{
	StencilOp stencilPassOp = {};
	StencilOp stencilDepthFailOp = {};
	StencilOp stencilFailOp = {};
	CompareOp compareOp = {};
	uint32    compareMask = 0;
	uint32    writeMask = 0;
	uint32    reference = 0;
};

struct DepthStencilState
{
	bool           depthTestEnable = false;
	bool           depthWriteEnable = false;
	CompareOp      depthCompareOp = CompareOp::Never;
	bool           stencilTestEnable = false;
	StencilOpState front = {};
	StencilOpState back = {};
};

struct ColorBlendAttachmentState
{
	bool                 blendEnable = false;
	BlendFactor          srcColorBlendFactor = BlendFactor::Zero;
	BlendFactor          dstColorBlendFactor = BlendFactor::Zero;
	BlendOp              colorBlendOp = BlendOp::Add;
	BlendFactor          srcAlphaBlendFactor = BlendFactor::Zero;
	BlendFactor          dstAlphaBlendFactor = BlendFactor::Zero;
	BlendOp              alphaBlendOp = BlendOp::Add;
	ColorComponentFlags  colorWriteMask = ColorComponentFlags::All;
};

struct ColorBlendState
{
	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	uint32 numColorAttachments = 0;
	ColorBlendAttachmentState attachmentStates[MaxNumSimultaneousColorRenderTargets];
};

enum class RenderTargetLoadOp
{
	DontCare,
	Load,
	Clear,
	Count
};

enum class RenderTargetStoreOp
{
	DontCare,
	Store,
	Count
};

enum class ClearDepthStencil
{
	Depth,
	Stencil,
	DepthStencil,
};

enum class TextureType
{
	Texture1D,
	Texture2D,
	Texture3D,
	TextureCube,
	Count
};	
	
enum class TextureFilter
{
	Nearest,
	Linear,
};

enum class QueryType
{
	Occlusion,
	Timestamp,
};

enum class TextureCreateFlags
{
	None             = 0,
	RenderTarget     = (1 << 0),
	InputAttachment  = (1 << 1),
	DepthStencil     = (1 << 2),
	ShaderResource   = (1 << 3),
	UnorderedAccess  = (1 << 4),
	Present          = (1 << 5),
	SRGB             = (1 << 6),
	NoTilling        = (1 << 7),
	Dynamic          = (1 << 8), 
};
ENUM_CLASS_OPERATORS(TextureCreateFlags);

enum class TextureAddressMode
{
	Warp   = 0,
	Mirror = 1,
	Clamp  = 2,
	Border = 3,
};

enum class Filter
{
	MinMagMipPoint,
	MinMagPointMipLinear,
	MinPointMagLinearMipPoint,
	MinPointMagMipLinear,
	MinLinearMagMipPoint,
	MinLinearMagPointMipLinear,
	MinMagLinearMipPoint,
	MinMagMipLinear,
	Anisotropic,
	ComparisonMinMagMipPoint,
	ComparisonMinMagPointMipLinear,
	ComparisonMinPointMagLinearMipPoint,
	ComparisonMinPointMagMipLinear,
	ComparisonMinLinearMagMipPoint,
	ComparisonMinLinearMagPointMipLinear,
	ComparisonMinMagLinearMipPoint,
	ComparisonMinMagMipLinear,
	ComparisonAnisotropic,
	MinimumMinMagMipPoint,
	MinimumMinMagPointMipLinear,
	MinimumMinPointMagLinearMipPoint,
	MinimumMinPointMagMipLinear,
	MinimumMinLinearMagMipPoint,
	MinimumMinLinearMagPointMipLinear,
	MinimumMinMagLinearMipPoint,
	MinimumMinMagMipLinear,
	MinimumAnisotropic,
	MaximumMinMagMipPoint,
	MaximumMinMagPointMipLinear,
	MaximumMinPointMagLinearMipPoint,
	MaximumMinPointMagMipLinear,
	MaximumMinLinearMagMipPoint,
	MaximumMinLinearMagPointMipLinear,
	MaximumMinMagLinearMipPoint,
	MaximumMinMagMipLinear,
	MaximumAnisotropic,
};

enum class BufferCreateFlags
{
	None                 = 0,
	// Update
	Static               = (1 << 0),
	Dynamic              = (1 << 1),
	// Usage
	UnorderedAccess      = (1 << 2),
	CopySrc              = (1 << 3),
	CopyDst              = (1 << 4),
	IndirectArguments    = (1 << 5),
	ShaderResource       = (1 << 6),
	VertexBuffer         = (1 << 7),
	IndexBuffer          = (1 << 8),
	UniformBuffer        = (1 << 9),
	AccelerationStruture = (1 << 10),
	ShaderBindingTable   = (1 << 11),
	// Memory access
	CreateMapped         = (1 << 12),
	CpuOnly              = (1 << 13),
	GpuOnly              = (1 << 14),
	CpuToGpu             = (1 << 15),
	GpuToCpu             = (1 << 16),
};
ENUM_CLASS_OPERATORS(BufferCreateFlags);

enum class RenderBackendResourceState
{
	Undefined             = 0,
	// Read only
	Present               = (1 << 0),
	IndirectArgument      = (1 << 1),
	VertexBuffer          = (1 << 2),
	IndexBuffer           = (1 << 3),
	ShaderResource        = (1 << 4),
	CopySrc               = (1 << 5),
	DepthStencilReadOnly  = (1 << 7),
	// Write only
	RenderTarget          = (1 << 8),
	CopyDst               = (1 << 9),
	// Read-write
	UnorderedAccess       = (1 << 10),
	DepthStencil          = (1 << 11),

	ReadOnlyMask = Present | IndirectArgument | VertexBuffer | IndexBuffer | ShaderResource | CopySrc | DepthStencilReadOnly,
	WriteOnlyMask = RenderTarget | CopyDst,
	ReadWriteMask = UnorderedAccess | DepthStencil,
	WritableMask = RenderTarget | CopyDst | UnorderedAccess | DepthStencil,
}; 
ENUM_CLASS_OPERATORS(RenderBackendResourceState);

struct RenderBackendViewport
{
	float x;
	float y;
	float width;
	float height;
	float minDepth;
	float maxDepth;
};

struct RenderBackendScissor
{
	int32 left;
	int32 top;
	uint32 width;
	uint32 height;
};

#define REMAINING_ARRAY_LAYERS (~0u)
#define REMAINING_MIP_LEVELS (~0u)
struct TextureSubresourceRange
{
	uint32 firstLevel;
	uint32 mipLevels;
	uint32 firstLayer;
	uint32 arrayLayers; 
	FORCEINLINE bool operator==(const TextureSubresourceRange& rhs) const
	{
		return (firstLevel == rhs.firstLevel)
			&& (mipLevels == rhs.mipLevels)
			&& (firstLayer == rhs.firstLayer)
			&& (arrayLayers == rhs.arrayLayers);
	}
};

struct BufferSubresourceRange
{
	uint64 offset;
	uint64 size;
};

struct TextureSubresourceLayers
{
	uint32 mipLevel;
	uint32 firstLayer;
	uint32 arrayLayers;
};

struct RenderTargetClearValue
{
	union
	{
		struct ClearColorValue
		{
			union
			{
				float float32[4];
				int32 int32[4];
				uint32 uint32[4];
			};
		} color;
		struct ClearDepthStencilValue
		{
			float depth;
			uint32 stencil;
		} depthStencil;
	};
};
	
struct RenderStatistics
{
	uint64 nonIndexedDraws;
	uint64 indexedDraws;
	uint64 nonIndexedIndirectDraws;
	uint64 indexedIndirectDraws;
	uint64 computeDispatches;
	uint64 computeIndirectDispatches;
	uint64 traceRayDispatches;
	uint64 vertices;
	uint64 pipelines;
	uint64 transitions;
	uint64 renderPasses;
	void Add(const RenderStatistics& other)
	{
		nonIndexedDraws += other.nonIndexedDraws;
		indexedDraws += other.indexedDraws;
		nonIndexedIndirectDraws += other.nonIndexedIndirectDraws;
		indexedIndirectDraws += other.indexedIndirectDraws;
		computeDispatches += other.computeDispatches;
		computeIndirectDispatches += other.computeIndirectDispatches;
		traceRayDispatches += other.traceRayDispatches;
		vertices += other.vertices;
		pipelines += other.pipelines;
		transitions += other.transitions;
		renderPasses += other.renderPasses;
	}
};

struct RenderBackendBarrier
{
	enum class ResourceType
	{
		Texture,
		Buffer,
	};
	ResourceType type;
	RenderBackendResourceState srcState;
	RenderBackendResourceState dstState;
	union
	{
		struct
		{
			RenderBackendTextureHandle texture;
			TextureSubresourceRange textureRange;
		};
		struct
		{
			RenderBackendBufferHandle buffer;
			BufferSubresourceRange bufferRange;
		};
	};
	RenderBackendBarrier(RenderBackendTextureHandle texture, TextureSubresourceRange range, RenderBackendResourceState srcState, RenderBackendResourceState dstState)
		: type(ResourceType::Texture), texture(texture), textureRange(range), srcState(srcState), dstState(dstState) {}
	RenderBackendBarrier(RenderBackendBufferHandle buffer, BufferSubresourceRange range, RenderBackendResourceState srcState, RenderBackendResourceState dstState)
		: type(ResourceType::Buffer), buffer(buffer), bufferRange(range), srcState(srcState), dstState(dstState) {}
};

struct RenderPassInfo
{
	struct ColorRenderTarget
	{
		RenderBackendTextureHandle texture;
		uint32 mipLevel;
		uint32 arrayLayer;
		RenderTargetLoadOp loadOp;
		RenderTargetStoreOp storeOp;
	};
	struct DepthStencilRenderTarget
	{
		RenderBackendTextureHandle texture;
		RenderTargetLoadOp depthLoadOp;
		RenderTargetStoreOp depthStoreOp;
		RenderTargetLoadOp stencilLoadOp;
		RenderTargetStoreOp stencilStoreOp;
	};
	ColorRenderTarget colorRenderTargets[MaxNumSimultaneousColorRenderTargets];
	DepthStencilRenderTarget depthStencilRenderTarget;
};

struct ShaderBindingTable
{
	RenderBackendBufferHandle buffer;
	uint64 offset;
	uint64 size;
	uint64 stride;
};

}