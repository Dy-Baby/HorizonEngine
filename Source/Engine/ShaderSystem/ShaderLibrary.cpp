#include "ShaderLibrary.h"

namespace HE
{
	bool ShaderLibrary::LoadShader(const char* filename, const char* entry)
	{
		ShaderCreateInfo shaderInfo = {};
		
		std::vector<const wchar*> includeDirs;
		std::vector<const wchar*> defines;
		includeDirs.push_back(HE_TEXT("../../../Shaders"));
		includeDirs.push_back(HE_TEXT("../../../Shaders/HybridRenderPipeline"));

		RenderBackendShaderDesc brdfLutShaderDesc;
		LoadShaderSourceFromFile(filename, shaderInfo.code);

		CompileShader(
			shaderCompiler,
			source,
			HE_TEXT("BRDFLutCS"),
			RenderBackendShaderStage::Compute,
			ShaderRepresentation::SPIRV,
			includeDirs,
			defines,
			&brdfLutShaderDesc.stages[(uint32)RenderBackendShaderStage::Compute]);
		brdfLutShaderDesc.entryPoints[(uint32)RenderBackendShaderStage::Compute] = "BRDFLutCS";
		brdfLutShader = RenderBackendCreateShader(renderBackend, deviceMask, &brdfLutShaderDesc, "BRDFLutShader");

		Shader* shader = new Shader(shaderInfo);
		ASSERT(loadedShaders.find(name) == loadedShaders.end());
		loadedShaders[name] = shader;
		return true;
	}
}