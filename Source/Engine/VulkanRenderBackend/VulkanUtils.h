#pragma once

#include "VulkanCommon.h"
#include "VulkanMemory.h"

namespace HE
{

extern void VerifyVkResult(VkResult result, const char* vkFuntion, const char* filename, uint32 line);

#define VK_CHECK(VkFunction) { const VkResult result = VkFunction; if (result != VK_SUCCESS) { VerifyVkResult(result, #VkFunction, __FILE__, __LINE__); } }
#define VK_CHECK_RESULT(result) { if (result != VK_SUCCESS) { VerifyVkResult(result, __FUNCTION__, __FILE__, __LINE__); } }

inline VkBool32 ToVkBool(bool b)
{
	return b ? VK_TRUE : VK_FALSE;
}

inline bool IsDepthOnlyFormat(VkFormat format)
{
	return (format == VK_FORMAT_D32_SFLOAT) || (format == VK_FORMAT_D16_UNORM);
}

inline bool IsDepthStencilFormat(VkFormat format)
{
	return (IsDepthOnlyFormat(format)) || (format == VK_FORMAT_D32_SFLOAT_S8_UINT) || (format == VK_FORMAT_D24_UNORM_S8_UINT) || (format == VK_FORMAT_D16_UNORM_S8_UINT);
}

inline bool IsStencilFormat(VkFormat format)
{
	return (format == VK_FORMAT_D32_SFLOAT_S8_UINT) || (format == VK_FORMAT_D24_UNORM_S8_UINT) || (format == VK_FORMAT_D16_UNORM_S8_UINT);
}

inline uint32 AlignUp(uint32 size, uint32 alignment)
{
	return (size + alignment - 1) & ~(alignment - 1);
}

extern VkPrimitiveTopology ToVkPrimitiveTopology(PrimitiveTopology topology); 
extern VkCullModeFlags ToVkCullModeFlags(RasterizationCullMode mode);
extern VkStencilOpState ToVkStencilOpState(const StencilOpState& stencilOpState);
extern VkFormat ToVkFormat(PixelFormat format);
extern VkImageType ToVkImageType(TextureType type);
extern VkImageViewType ToVkImageViewType(TextureType type, bool isArray);
extern VkFilter ToVkFilter(TextureFilter filter);
extern VkShaderStageFlagBits ToVkShaderStageFlagBits(RenderBackendShaderStage stage);
extern VkAttachmentLoadOp ToVkAttachmentLoadOp(RenderTargetLoadOp loadOp);
extern VkAttachmentStoreOp ToVkAttachmentStoreOp(RenderTargetStoreOp storeOp);
extern VkImageAspectFlags GetVkImageAspectFlags(VkFormat format);
extern VkImageUsageFlags GetVkImageUsageFlags(TextureCreateFlags flags);
extern VkBufferUsageFlags GetVkBufferUsageFlags(BufferCreateFlags flags);
extern VmaMemoryUsage GetVmaMemoryUsage(BufferCreateFlags flags);
extern VkSamplerAddressMode ToVkSamplerAddressMode(TextureAddressMode addressMode);
extern VkBuildAccelerationStructureFlagsKHR ToVkBuildAccelerationStructureFlagsKHR(RenderBackendAccelerationStructureBuildFlags flags);
extern VkGeometryInstanceFlagsKHR ToVkGeometryInstanceFlagsKHR(RenderBackendRayTracingInstanceFlags flags);
extern VkGeometryFlagsKHR ToVkGeometryFlagsKHR(RenderBackendGeometryFlags flags);
extern void GetVkFilterAndVkSamplerMipmapMode(
	Filter filter,
	VkFilter* outMinFilter,
	VkFilter* outMagFilter,
	VkSamplerMipmapMode* outMipmapMode,
	bool* outAnisotropyEnable,
	bool* outCompareEnable);
extern void GetBarrierInfo2(
	RenderBackendResourceState srcState,
	RenderBackendResourceState dstState,
	VkImageLayout* outOldLayout,
	VkImageLayout* outNewLayout,
	VkPipelineStageFlags2* outSrcStageMask,
	VkPipelineStageFlags2* outDstStageMask,
	VkAccessFlags2* outSrcAccessMask,
	VkAccessFlags2* outDstAccessMask);

}
