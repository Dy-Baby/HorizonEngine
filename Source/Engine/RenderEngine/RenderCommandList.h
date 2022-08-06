#pragma once

#include "RenderEngine/RenderEngineCommon.h"
#include "RenderEngine/RenderCommands.h"
#include "RenderEngine/RenderBackend.h"

namespace HE
{

struct RenderCommandContainer
{
	uint32 numCommands = 0;
	std::vector<RenderCommandType> types = {};
	std::vector<void*> commands = {};
};

class RenderCommandListBase
{
public:
	RenderCommandListBase(MemoryArena* arena) : arena(arena) {}
	virtual ~RenderCommandListBase() {}
	template <typename T>
	FORCEINLINE T* AllocateCommand(RenderCommandType type, uint64 size = sizeof(T))
	{
		void* data = AllocateCommandInternal(size);
		container.types.push_back(type);
		container.commands.push_back(data);
		container.numCommands++;
		return (T*)data;
	}
	FORCEINLINE RenderCommandContainer* GetCommandContainer()
	{
		return &container;
	}
private:
	FORCEINLINE void* AllocateCommandInternal(uint64 size)
	{
		void* data = HE_ARENA_ALLOC(arena, size);
		return data;
	}
	MemoryArena* arena;
	RenderCommandContainer container;
};

/**
 * Render command list encodes high level commands.
 * The commands are stateless, which allows for fully parallel recording.
 */
class RenderCommandList : public RenderCommandListBase
{
public:	
	RenderCommandList(MemoryArena* arena) : RenderCommandListBase(arena) {}
	// Copy commands
	void CopyTexture2D(RenderBackendTextureHandle srcTexture, const Offset2D& srcOffset, uint32 srcMipLevel, RenderBackendTextureHandle dstTexture, const Offset2D& dstOffset, uint32 dstMipLevel, const Extent2D extent);
	void CopyBuffer(RenderBackendBufferHandle srcBuffer, uint64 srcOffset, RenderBackendBufferHandle dstBuffer, uint64 dstOffset, uint64 bytes);
	//void UpdateBuffer(RenderBackendBufferHandle buffer, uint64 offset, void* data, uint64 size);
	// Compute commands
	void Dispatch(RenderBackendShaderHandle shader, const ShaderArguments& shaderArguments, uint32 x, uint32 y, uint32 z);
	void Dispatch2D(RenderBackendShaderHandle shader, const ShaderArguments& shaderArguments, uint32 x, uint32 y);
	/*void BuildTopLevelAS();
	void BuildBottomLevelAS();
	void UpdateTopLevelAS();
	void UpdateBottomLevelAS(); */
	void TraceRays(RenderBackendRayTracingPipelineStateHandle pipelineState, RenderBackendBufferHandle shaderBindingTable, const ShaderArguments& shaderArguments, uint32 x, uint32 y, uint32 z);
	// Graphics commands
	void SetViewports(RenderBackendViewport* viewports, uint32 numViewports);
	void SetScissors(RenderBackendScissor* scissors, uint32 numScissors);
	void Transitions(RenderBackendBarrier* transitions, uint32 numTransitions);
	void BeginRenderPass(const RenderPassInfo& renderPassInfo);
	void EndRenderPass();
	void Draw(RenderBackendShaderHandle shader, const ShaderArguments& shaderArguments, uint32 numVertices, uint32 numInstances, uint32 firstVertex, uint32 firstInstance, PrimitiveTopology topology);
	void DrawIndexed(RenderBackendShaderHandle shader, const ShaderArguments& shaderArguments, RenderBackendBufferHandle indexBuffer, uint32 numIndices, uint32 numInstances, uint32 firstIndex, int32 vertexOffset, uint32 firstInstance, PrimitiveTopology topology);
	void DrawIndirect(RenderBackendShaderHandle shader, const ShaderArguments& shaderArguments, RenderBackendBufferHandle indexBuffer, RenderBackendBufferHandle argumentBuffer, uint64 offset, uint32 numDraws, uint32 stride, PrimitiveTopology topology);
	void DrawIndexedIndirect(RenderBackendShaderHandle shader, const ShaderArguments& shaderArguments, RenderBackendBufferHandle indexBuffer, RenderBackendBufferHandle argumentBuffer, uint64 offset, uint32 numDraws, uint32 stride, PrimitiveTopology topology);
	void BeginTimingQuery(RenderBackendTimingQueryHeapHandle timingQueryHeap, uint32 region);
	void EndTimingQuery(RenderBackendTimingQueryHeapHandle timingQueryPool, uint32 region);
	//void ResolveTimimgs(TimingQueryPoolHandle timingQueryPool, uint32 regionStart, uint32 regionCount);
};

}
