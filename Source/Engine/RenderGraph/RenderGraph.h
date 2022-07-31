#pragma once

#include "RenderGraph/RenderGraphDefinitions.h"
#include "RenderGraph/RenderGraphHandles.h"
#include "RenderGraph/RenderGraphDAG.h"
#include "RenderGraph/RenderGraphBlackboard.h"
#include "RenderGraph/RenderGraphBuilder.h"
#include "RenderGraph/RenderGraphRegistry.h"
#include "RenderGraph/RenderGraphPass.h"
#include "RenderGraph/RenderGraphResources.h"
#include "RenderGraph/RenderGraphResourcePool.h"

namespace HE
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
	RenderGraphTextureSRVHandle CreateTextureSRV(RenderGraphTextureHandle texture, const RenderGraphTextureSRVDesc& desc);
	RenderGraphTextureUAVHandle CreateTextureUAV(RenderGraphTextureHandle texture, uint32 mipLevel);
	RenderGraphTextureHandle ImportExternalTexture(RenderBackendTextureHandle renderBackendTexture, const RenderBackendTextureDesc& desc, RenderBackendResourceState initialState, char const* name);
	RenderGraphBufferHandle ImportExternalBuffer(RenderBackendBufferHandle renderBackendBuffer, const RenderBackendBufferDesc& desc, RenderBackendResourceState initialState, char const* name);

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