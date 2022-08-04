#pragma once

#include "Core/Core.h"
#include "RenderEngine/RenderEngine.h"

#define DEBUG_ONLY_RAY_TRACING_ENBALE 1

namespace HE
{
	extern "C"
	{
		enum VulkanRenderBackendCreateFlags
		{
			VULKAN_RENDER_BACKEND_CREATE_FLAGS_NONE = 0,
			VULKAN_RENDER_BACKEND_CREATE_FLAGS_VALIDATION_LAYERS = (1 << 1),
			VULKAN_RENDER_BACKEND_CREATE_FLAGS_SURFACE = (1 << 2),
		};

		RenderBackend* VulkanRenderBackendCreateBackend(int flags);
		void VulkanRenderBackendDestroyBackend(RenderBackend* backend);
	}
}