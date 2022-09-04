module;

export module HorizonEngine.Render.Enviroment;

import HorizonEngine.Core;
import HorizonEngine.Render.Core;

export namespace HE
{
	uint32 GIrradianceEnviromentMapSize = 32;

	void GenerateCubemapMips(RenderCommandList& commandList, RenderBackendTextureHandle cubemap, uint32 numMipLevels)
	{
		// TODO
		for (uint32 mipLevel = 1; mipLevel < numMipLevels; mipLevel++)
		{

		}
	}

	void ComputeEnviromentIrradiance(RenderCommandList& commandList, RenderBackendTextureHandle enviromentMap, uint32 mipLevel, RenderBackendTextureHandle irradianceEnviromentMap)
	{
		RenderBackendBarrier transition(irradianceEnviromentMap, RenderBackendTextureSubresourceRange(0, REMAINING_MIP_LEVELS, 0, REMAINING_ARRAY_LAYERS), RenderBackendResourceState::Undefined, RenderBackendResourceState::UnorderedAccess);
		commandList.Transitions(&transition, 1);

		uint32 dispatchX = CEIL_DIV(GIrradianceEnviromentMapSize, 8);
		uint32 dispatchY = CEIL_DIV(GIrradianceEnviromentMapSize, 8);
		uint32 dispatchZ = 6;

		ShaderArguments shaderArguments = {};
		shaderArguments.BindTextureSRV(0, RenderBackendTextureSRVDesc::Create(enviromentMap));
		shaderArguments.BindTextureUAV(1, RenderBackendTextureUAVDesc::Create(irradianceEnviromentMap));

		RenderBackendShaderHandle computeShader;
		commandList.Dispatch(
			computeShader,
			shaderArguments,
			dispatchX,
			dispatchY,
			dispatchZ);

		transition = RenderBackendBarrier(irradianceEnviromentMap, RenderBackendTextureSubresourceRange(0, REMAINING_MIP_LEVELS, 0, REMAINING_ARRAY_LAYERS), RenderBackendResourceState::UnorderedAccess, RenderBackendResourceState::ShaderResource);
		commandList.Transitions(&transition, 1);
	}

	void FilterEnviromentMap(RenderCommandList& commandList, RenderBackendTextureHandle enviromentMap, uint32 numMipLevels, RenderBackendTextureHandle filteredEnviromentMap)
	{
		RenderBackendBarrier transition(filteredEnviromentMap, RenderBackendTextureSubresourceRange(0, REMAINING_MIP_LEVELS, 0, REMAINING_ARRAY_LAYERS), RenderBackendResourceState::Undefined, RenderBackendResourceState::UnorderedAccess);
		commandList.Transitions(&transition, 1);

		for (uint32 mipLevel = 0; mipLevel < numMipLevels; mipLevel++)
		{
			uint32 dispatchX = CEIL_DIV(1 << (numMipLevels - mipLevel - 1), 8);
			uint32 dispatchY = CEIL_DIV(1 << (numMipLevels - mipLevel - 1), 8);
			uint32 dispatchZ = 6;

			float roughness = (float)mipLevel / (float)(numMipLevels - 1);

			ShaderArguments shaderArguments = {};
			shaderArguments.BindTextureSRV(0, RenderBackendTextureSRVDesc::Create(enviromentMap));
			shaderArguments.BindTextureUAV(1, RenderBackendTextureUAVDesc::Create(filteredEnviromentMap, mipLevel));
			shaderArguments.PushConstants(0, roughness);

			RenderBackendShaderHandle computeShader;
			commandList.Dispatch(
				computeShader,
				shaderArguments,
				dispatchX,
				dispatchY,
				dispatchZ);
		}

		transition = RenderBackendBarrier(filteredEnviromentMap, RenderBackendTextureSubresourceRange(0, REMAINING_MIP_LEVELS, 0, REMAINING_ARRAY_LAYERS), RenderBackendResourceState::UnorderedAccess, RenderBackendResourceState::ShaderResource);
		commandList.Transitions(&transition, 1);
	}

	void ComputeEnviromentCubemaps(RenderCommandList& commandList, RenderBackendTextureHandle enviromentMap, uint32 cubemapSize, RenderBackendTextureHandle irradianceEnviromentMap, RenderBackendTextureHandle filteredEnviromentMap)
	{
		const uint32 numMipLevels = Math::MaxMipLevelCount(cubemapSize);

		GenerateCubemapMips(commandList, enviromentMap, numMipLevels);

		const uint32 numIrradianceEnviromentMapMipLevels = Math::MaxMipLevelCount(GIrradianceEnviromentMapSize) + 1;
		const uint32 sourceMipLevel = Math::Max<uint32>(0, numMipLevels - numIrradianceEnviromentMapMipLevels);

		ComputeEnviromentIrradiance(commandList, enviromentMap, sourceMipLevel, irradianceEnviromentMap);

		FilterEnviromentMap(commandList, enviromentMap, numMipLevels, filteredEnviromentMap);
	}
}