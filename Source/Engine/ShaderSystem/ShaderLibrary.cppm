module;

#include <unordered_map>

export module HorizonEngine.ShaderSystem;

import HorizonEngine.Core;
import HorizonEngine.Render.Core;

export namespace HE
{
	struct ShaderCreateInfo
	{
		const std::vector<uint8>& code;
		const uint32 codeSize;
		 pipelineState;
	};

	/** A compiled shader. */
	class Shader
	{
	public:
		Shader(const ShaderCreateInfo& info);
		~Shader();
		uint64 GetHash() const;
	private:
		MemoryArena* allocator;
		RenderBackendInstance backend;
		RenderBackendShaderHandle handle;
		bool compiled;
	};

	struct ShaderMacro
	{
		std::string name;
		std::string value;
	};

	typedef std::vector<ShaderMacro> ShaderMacros;

	class ShaderLibrary
	{
	public:
		ShaderLibrary() {}
		~ShaderLibrary() {}
		bool LoadShader(const char* filename, const char* entry, const ShaderMacros* macros);
		bool ReloadShader();
		void UnloadShader();
		//void Clear();
		Shader* GetShader(const std::string& name) const
		{
			ASSERT(loadedShaders.find(name) != loadedShaders.end());
			return loadedShaders[name];
		}
	private:
		RenderBackend* renderBackend;
		RenderBackendShaderCompiler* shaderCompiler;
		std::unordered_map<std::string, Shader*> loadedShaders;
	};

	ShaderLibrary* GGlobalShaderLibrary = nullptr;

	ShaderLibrary* GetGlobalShaderMap()
	{
		ASSERT(GGlobalShaderLibrary);
		return GGlobalShaderLibrary;
	}
}