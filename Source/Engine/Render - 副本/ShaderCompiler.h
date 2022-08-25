#pragma once

import HorizonEngine.Core;
import HorizonEngine.Render.Core;

namespace HE
{
	enum class ShaderRepresentation
	{
		DXIL,
		SPIRV,
	};

	struct ShaderCompiler
	{
		void* instance;
		bool (*CompileShader)(
			void* instance,
			std::vector<uint8> source,
			const wchar* entry,
			RenderBackendShaderStage stage,
			ShaderRepresentation representation,
			const std::vector<const wchar*>& includeDirs,
			const std::vector<const wchar*>& defines,
			ShaderBlob* outBlob);
		void (*ReleaseShaderBlob)(void* instance, ShaderBlob* blob);
	};

	void LoadShaderSourceFromFile(const char* filename, std::vector<uint8>& outData);

	bool CompileShader(
		ShaderCompiler* compiler,
		std::vector<uint8> source,
		const wchar* entry,
		RenderBackendShaderStage stage,
		ShaderRepresentation representation,
		const std::vector<const wchar*>& includeDirs,
		const std::vector<const wchar*>& defines,
		ShaderBlob* outBlob);

	void ReleaseShaderBlob(ShaderCompiler* compiler, ShaderBlob* blob); 
}