export module HorizonEngine.Render.RenderGraph;

export import "Render/RenderGraph/RenderGraphDefinitions.h";
export import "Render/RenderGraph/RenderGraphHandles.h";
export import "Render/RenderGraph/RenderGraphDAG.h";
export import "Render/RenderGraph/RenderGraphBlackboard.h";
export import "Render/RenderGraph/RenderGraphBuilder.h";
export import "Render/RenderGraph/RenderGraphRegistry.h";
export import "Render/RenderGraph/RenderGraphPass.h";
export import "Render/RenderGraph/RenderGraphResources.h";
export import "Render/RenderGraph/RenderGraphResourcePool.h";

import HorizonEngine.Render.Core;

export namespace HE
{
	class RenderGraph
	{
	public:
		RenderGraph(MemoryArena* arena);
		RenderGraph(const RenderGraph& other) = delete;
		virtual ~RenderGraph();

		template<typename SetupLambdaType>
		void AddPass(const char* name, RenderGraphPassFlags flags, SetupLambdaType setup);

		void Execute(RenderContext* contex);

		void Clear();

		/**
		 * @brief Create a string using the Graphviz format.
		 * @note Compile() should be called before calling this function.
		 * @return std::pmr::string in the Graphviz format.
		 */
		std::string Graphviz() const;

		RenderGraphTextureHandle CreateTexture(const RenderGraphTextureDesc& desc, const char* name);
		RenderGraphBufferHandle CreateBuffer(const RenderGraphBufferDesc& desc, const char* name);
		//RenderGraphTextureSRVHandle CreateTextureSRV(RenderGraphTextureHandle texture, const RenderGraphTextureSRVDesc& desc);
		//RenderGraphTextureUAVHandle CreateTextureUAV(RenderGraphTextureHandle texture, uint32 mipLevel);
		RenderGraphTextureHandle ImportExternalTexture(RenderBackendTextureHandle renderBackendTexture, const RenderBackendTextureDesc& desc, RenderBackendResourceState initialState, char const* name);
		RenderGraphBufferHandle ImportExternalBuffer(RenderBackendBufferHandle renderBackendBuffer, const RenderBackendBufferDesc& desc, RenderBackendResourceState initialState, char const* name);
		void ExportTextureDeferred(RenderGraphTextureHandle texture, RenderGraphPersistentTexture* outPersistentTexture);

		RenderGraphBlackboard blackboard;

	private:
		friend class RenderGraphBuilder;
		friend class RenderGraphRegistry;
	
		bool Compile();

		void* Alloc(uint32 size)
		{
			return HE_ARENA_ALLOC(arena, size);
		}

		void* AlignedAlloc(uint32 size, uint32 alignment)
		{
			return HE_ARENA_ALIGNED_ALLOC(arena, size, alignment);
		}

		template <typename ObjectType, typename... Args>
		FORCEINLINE ObjectType* AllocObject(Args&&... args)
		{
			ObjectType* result = (ObjectType*)HE_ARENA_ALLOC(arena, sizeof(ObjectType));
			ASSERT(result);
			result = new(result) ObjectType(std::forward<Args>(args)...);
			return result;
		}

		MemoryArena* arena;

		RenderGraphDAG dag;

		std::vector<RenderGraphPass*> passes;

		std::vector<RenderGraphTexture*> textures;
		std::vector<RenderGraphBuffer*> buffers;

		std::map<RenderBackendTextureHandle, RenderGraphTextureHandle> externalTextures;
		std::map<RenderBackendBufferHandle, RenderGraphBufferHandle> externalBuffers;
	};

	template<typename SetupLambdaType>
	void RenderGraph::AddPass(const char* name, RenderGraphPassFlags flags, SetupLambdaType setup)
	{
		RenderGraphLambdaPass* pass = AllocObject<RenderGraphLambdaPass>(name, flags);
		RenderGraphBuilder builder(this, pass);
		const auto& execute = setup(builder);
		pass->SetExecuteCallback(std::move(execute));
		passes.emplace_back(pass);
		dag.RegisterNode(pass);
	}
}
