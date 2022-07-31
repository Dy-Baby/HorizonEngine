#include "RenderCommandList.h"
#include "RenderCommands.h"

namespace HE
{

void RenderCommandList::CopyTexture2D(RenderBackendTextureHandle srcTexture, const Offset2D& srcOffset, uint32 srcMipLevel, RenderBackendTextureHandle dstTexture, const Offset2D& dstOffset, uint32 dstMipLevel, const Extent2D extent)
{
	RenderCommandCopyTexture* command = AllocateCommand<RenderCommandCopyTexture>(RenderCommandCopyTexture::Type);
	command->srcTexture = srcTexture;
	command->srcOffset = {
		.x = srcOffset.x,
		.y = srcOffset.y,
		.z = 0,
	};
	command->srcSubresourceLayers = {
		.mipLevel = srcMipLevel,
		.firstLayer = 0,
		.arrayLayers = 1,
	};
	command->dstTexture = dstTexture;
	command->dstOffset = {
		.x = dstOffset.x,
		.y = dstOffset.y,
		.z = 0,
	};
	command->dstSubresourceLayers = {
		.mipLevel = dstMipLevel,
		.firstLayer = 0,
		.arrayLayers = 1,
	};
	command->extent = {
		.width = extent.width,
		.height = extent.height,
		.depth = 1,
	};
}

void RenderCommandList::CopyBuffer(RenderBackendBufferHandle srcBuffer, uint64 srcOffset, RenderBackendBufferHandle dstBuffer, uint64 dstOffset, uint64 bytes)
{
	RenderCommandCopyBuffer* command = AllocateCommand<RenderCommandCopyBuffer>(RenderCommandCopyBuffer::Type);
	command->srcBuffer = srcBuffer;
	command->srcOffset = srcOffset;
	command->dstBuffer = dstBuffer;
	command->dstOffset = dstOffset;
	command->bytes = bytes;
}

//void RenderCommandList::UpdateBuffer(RenderBackendBufferHandle buffer, uint64 offset, void* data, uint64 size)
//{
//	RenderCommandUpdateBuffer* command = AllocateCommand<RenderCommandUpdateBuffer>(RenderCommandUpdateBuffer::Type);
//	command->buffer = buffer;
//	command->offset = offset;
//	command->data = data;
//	command->size = size;
//}

void RenderCommandList::Dispatch(RenderBackendShaderHandle shader, const ShaderArguments& shaderArguments, uint32 x, uint32 y, uint32 z)
{
	RenderCommandDispatch* command = AllocateCommand<RenderCommandDispatch>(RenderCommandDispatch::Type);
	command->shader = shader;
	command->threadGroupCountX = x;
	command->threadGroupCountY = y;
	command->threadGroupCountZ = z;
	memcpy(&command->shaderArguments, &shaderArguments, sizeof(ShaderArguments));
}
	
void RenderCommandList::Dispatch2D(RenderBackendShaderHandle shader, const ShaderArguments& shaderArguments, uint32 x, uint32 y)
{
	Dispatch(shader, shaderArguments, x, y, 1);
}

void RenderCommandList::TraceRay(RenderBackendShaderHandle shader, const ShaderArguments& shaderArguments, RenderBackendBufferHandle rgenSBT, RenderBackendBufferHandle rmissSBT, RenderBackendBufferHandle rchitSBT, uint32 x, uint32 y, uint32 z)
{
	RenderCommandTraceRay* command = AllocateCommand<RenderCommandTraceRay>(RenderCommandTraceRay::Type);
	command->shader = shader;
	command->rgenSBT = rgenSBT;
	command->rmissSBT = rmissSBT;
	command->rchitSBT = rchitSBT;
	command->width = x;
	command->height = y;
	command->depth = z;
	memcpy(&command->shaderArguments, &shaderArguments, sizeof(ShaderArguments));
}

void RenderCommandList::SetViewports(RenderBackendViewport* viewports, uint32 numViewports)
{
	RenderCommandSetViewport* command = AllocateCommand<RenderCommandSetViewport>(RenderCommandSetViewport::Type);
	command->numViewports = numViewports;
	memcpy(command->viewports, viewports, numViewports * sizeof(RenderBackendViewport));
}

void RenderCommandList::SetScissors(RenderBackendScissor* scissors, uint32 numScissors)
{
	RenderCommandSetScissor* command = AllocateCommand<RenderCommandSetScissor>(RenderCommandSetScissor::Type);
	command->numScissors = numScissors;
	memcpy(command->scissors, scissors, numScissors * sizeof(RenderBackendScissor));
}

void RenderCommandList::BeginRenderPass(const RenderPassInfo& renderPassInfo)
{
	RenderCommandBeginRenderPass* command = AllocateCommand<RenderCommandBeginRenderPass>(RenderCommandBeginRenderPass::Type);
	memcpy(command, &renderPassInfo, sizeof(RenderPassInfo));
}

void RenderCommandList::EndRenderPass()
{
	RenderCommandEndRenderPass* command = AllocateCommand<RenderCommandEndRenderPass>(RenderCommandEndRenderPass::Type);
}

void RenderCommandList::Transitions(RenderBackendBarrier* transitions, uint32 numTransitions)
{
	RenderCommandTransitions* command = AllocateCommand<RenderCommandTransitions>(RenderCommandTransitions::Type, sizeof(RenderCommandTransitions) + numTransitions * sizeof(RenderBackendBarrier));
	command->numTransitions = numTransitions;
	command->transitions = (RenderBackendBarrier*)(((uint8*)command) + sizeof(RenderCommandTransitions));
	memcpy(command->transitions, transitions, numTransitions * sizeof(RenderBackendBarrier));
}

void RenderCommandList::Draw(RenderBackendShaderHandle shader, const ShaderArguments& shaderArguments, uint32 numVertices, uint32 numInstances, uint32 firstVertex, uint32 firstInstance, PrimitiveTopology topology)
{
	RenderCommandDraw* command = AllocateCommand<RenderCommandDraw>(RenderCommandDraw::Type);
	command->shader = shader;
	command->numVertices = numVertices;
	command->numInstances = numInstances;
	command->firstVertex = firstVertex;
	command->firstInstance = firstInstance;
	command->topology = topology;
	command->indexBuffer = RenderBackendBufferHandle::NullHandle;
	memcpy(&command->shaderArguments, &shaderArguments, sizeof(ShaderArguments));
}

void RenderCommandList::DrawIndexed(
	RenderBackendShaderHandle shader, 
	const ShaderArguments& shaderArguments,
	RenderBackendBufferHandle indexBuffer, 
	uint32 numIndices,
	uint32 numInstances, 
	uint32 firstIndex,
	int32 vertexOffset, 
	uint32 firstInstance,
	PrimitiveTopology topology)
{
	RenderCommandDraw* command = AllocateCommand<RenderCommandDraw>(RenderCommandDraw::Type);
	command->shader = shader;
	command->indexBuffer = indexBuffer;
	command->numIndices = numIndices;
	command->numInstances = numInstances;
	command->firstIndex = firstIndex;
	command->vertexOffset = vertexOffset;
	command->firstInstance = firstInstance;
	command->topology = topology;
	memcpy(&command->shaderArguments, &shaderArguments, sizeof(ShaderArguments));
}

void RenderCommandList::DrawIndirect(
	RenderBackendShaderHandle shader,
	const ShaderArguments& shaderArguments,
	RenderBackendBufferHandle indexBuffer,
	RenderBackendBufferHandle argumentBuffer,
	uint64 offset,
	uint32 numDraws,
	uint32 stride,
	PrimitiveTopology topology)
{
	RenderCommandDrawIndirect* command = AllocateCommand<RenderCommandDrawIndirect>(RenderCommandDrawIndirect::Type);
	command->shader = shader;
	command->indexBuffer = indexBuffer;
	command->argumentBuffer = argumentBuffer;
	command->offset = offset;
	command->numDraws = numDraws;
	command->stride = stride;
	command->topology = topology;
	memcpy(&command->shaderArguments, &shaderArguments, sizeof(ShaderArguments));
}

void RenderCommandList::DrawIndexedIndirect(
	RenderBackendShaderHandle shader,
	const ShaderArguments& shaderArguments,
	RenderBackendBufferHandle indexBuffer,
	RenderBackendBufferHandle argumentBuffer,
	uint64 offset,
	uint32 numDraws,
	uint32 stride, 
	PrimitiveTopology topology)
{
	RenderCommandDrawIndirect* command = AllocateCommand<RenderCommandDrawIndirect>(RenderCommandDrawIndirect::Type);
	command->shader = shader;
	command->indexBuffer = indexBuffer;
	command->argumentBuffer = argumentBuffer;
	command->offset = offset;
	command->numDraws = numDraws;
	command->stride = stride;
	command->topology = topology;
	memcpy(&command->shaderArguments, &shaderArguments, sizeof(ShaderArguments));
}

void RenderCommandList::BeginTimingQuery(RenderBackendTimingQueryHeapHandle timingQueryPool, uint32 region)
{
	auto* command = AllocateCommand<RenderCommandBeginTimingQuery>(RenderCommandBeginTimingQuery::Type);
	command->timingQueryHeap = timingQueryPool;
	command->region = region;
}

void RenderCommandList::EndTimingQuery(RenderBackendTimingQueryHeapHandle timingQueryPool, uint32 region)
{
	auto* command = AllocateCommand<RenderCommandEndTimingQuery>(RenderCommandBeginTimingQuery::Type);
	command->timingQueryHeap = timingQueryPool;
	command->region = region;
}

//void RenderCommandList::ResolveTimimgs(TimingQueryPoolHandle timingQueryPool, uint32 regionStart, uint32 regionCount)
//{
//	auto* command = AllocateCommand<RenderCommandResolveTimings>(timingQueryPool, regionStart, regionCount);
//}

}