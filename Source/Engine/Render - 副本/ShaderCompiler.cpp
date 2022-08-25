#include "ShaderCompiler.h"
#include "Core/Logging/LoggingDefines.h"

namespace HE
{
	void LoadShaderSourceFromFile(const char* filename, std::vector<uint8>& outData)
	{
		std::ifstream file(filename, std::ios::ate | std::ios::binary);
		if (!file.is_open())
		{
			HE_LOG_ERROR("Failed to open shader source file.");
			return;
		}
		size_t fileSize = (size_t)file.tellg();
		outData.resize(fileSize);
		file.seekg(0);
		file.read((char*)outData.data(), fileSize);
		file.close();
	}

	bool CompileShader(
		ShaderCompiler* compiler,
		std::vector<uint8> source,
		const wchar* entry,
		RenderBackendShaderStage stage,
		ShaderRepresentation representation,
		const std::vector<const wchar*>& includeDirs,
		const std::vector<const wchar*>& defines,
		ShaderBlob* outBlob)
	{
		return compiler->CompileShader(compiler->instance, source, entry, stage, representation, includeDirs, defines, outBlob);
	}

	void ReleaseShaderBlob(ShaderCompiler* compiler, ShaderBlob* blob)
	{
		return compiler->ReleaseShaderBlob(compiler->instance, blob);
	}
}