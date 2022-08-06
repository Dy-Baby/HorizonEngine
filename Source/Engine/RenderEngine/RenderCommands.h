#pragma once

#include "RenderEngine/RenderEngineCommon.h"
#include "RenderEngine/RenderBackend.h"

namespace HE
{

enum class RenderCommandQueueType : uint8
{
	None     = 0,
	Copy     = (1 << 0),
	Compute  = (1 << 1),
	Graphics = (1 << 2),
	All = Copy | Compute | Graphics,
};

enum class RenderCommandType
{
	CopyBuffer,
	CopyTexture,
	Barriers,
	Transitions,
	BeginTiming,
	EndTiming,
	ResolveTimings,
	Dispatch,
	DispatchIndirect,
	UpdateBottomLevelAS,
	UpdateTopLevelAS,
	TraceRays,
	SetViewport,
	SetScissor,
	BeginRenderPass,
	EndRenderPass,
	Draw,
	DrawIndirect,
	Count,
};

template<RenderCommandType commandType, RenderCommandQueueType queueType>
struct RenderCommand
{
	static const RenderCommandType Type = commandType;
	static const RenderCommandQueueType QueueType = queueType;
};

struct RenderCommandCopyBuffer : RenderCommand<RenderCommandType::CopyBuffer, RenderCommandQueueType::All>
{
	RenderBackendBufferHandle srcBuffer;
	uint64 srcOffset;
	RenderBackendBufferHandle dstBuffer;
	uint64 dstOffset;
	uint64 bytes;
};

struct RenderCommandCopyTexture : RenderCommand<RenderCommandType::CopyTexture, RenderCommandQueueType::All>
{
	RenderBackendTextureHandle srcTexture;
	Offset3D srcOffset;
	TextureSubresourceLayers srcSubresourceLayers;
	RenderBackendTextureHandle dstTexture;
	Offset3D dstOffset;
	TextureSubresourceLayers dstSubresourceLayers;
	Extent3D extent;
};

struct RenderCommandBarriers : RenderCommand<RenderCommandType::Barriers, RenderCommandQueueType::All>
{
	uint32 numBarriers;
};

struct RenderCommandTransitions : RenderCommand<RenderCommandType::Transitions, RenderCommandQueueType::All>
{
	uint32 numTransitions;
	RenderBackendBarrier* transitions;
};

struct RenderCommandBeginTimingQuery : RenderCommand<RenderCommandType::BeginTiming, RenderCommandQueueType::All>
{
	RenderBackendTimingQueryHeapHandle timingQueryHeap;
	uint32 region;
};

struct RenderCommandEndTimingQuery : RenderCommand<RenderCommandType::EndTiming, RenderCommandQueueType::All>
{
	RenderBackendTimingQueryHeapHandle timingQueryHeap;
	uint32 region;
};

struct RenderCommandResolveTimings : RenderCommand<RenderCommandType::ResolveTimings, RenderCommandQueueType::Graphics>
{
	//RenderBackendTimingQueryHeapHandle timingQueryHeap;
	//uint32 regionStart;
	//uint32 regionCount;
};

struct RenderCommandDispatch : RenderCommand<RenderCommandType::Dispatch, RenderCommandQueueType::Compute>
{
	RenderBackendShaderHandle shader;
	ShaderArguments shaderArguments;
	uint32 threadGroupCountX;
	uint32 threadGroupCountY;
	uint32 threadGroupCountZ;
};
	
struct RenderCommandDispatchIndirect : RenderCommand<RenderCommandType::DispatchIndirect, RenderCommandQueueType::Compute>
{
	RenderBackendShaderHandle shader;
	ShaderArguments shaderArguments;
	RenderBackendBufferHandle argumentBuffer;
	uint64 argumentOffset;
};

struct RenderCommandTraceRays : RenderCommand<RenderCommandType::TraceRays, RenderCommandQueueType::Graphics>
{
	RenderBackendRayTracingPipelineStateHandle pipelineState;
	RenderBackendBufferHandle shaderBindingTable;
	ShaderArguments shaderArguments;
	uint32 width;
	uint32 height;
	uint32 depth;
};

struct RenderCommandUpdateBottomLevelAS : RenderCommand<RenderCommandType::UpdateBottomLevelAS, RenderCommandQueueType::Compute>
{
	RenderBackendRayTracingAccelerationStructureHandle srcBLAS;
	RenderBackendRayTracingAccelerationStructureHandle dstBLAS;
};

struct RenderCommandUpdateTopLevelAS : RenderCommand<RenderCommandType::UpdateTopLevelAS, RenderCommandQueueType::Compute>
{
	RenderBackendRayTracingAccelerationStructureHandle srcTLAS;
	RenderBackendRayTracingAccelerationStructureHandle dstTLAS;
};

struct RenderCommandSetViewport : RenderCommand<RenderCommandType::SetViewport, RenderCommandQueueType::Graphics>
{
	uint32 numViewports;
	RenderBackendViewport viewports[MaxNumViewports];
};

struct RenderCommandSetScissor : RenderCommand<RenderCommandType::SetScissor, RenderCommandQueueType::Graphics>
{
	uint32 numScissors;
	RenderBackendScissor scissors[MaxNumViewports];
};

struct RenderCommandBeginRenderPass : RenderCommand<RenderCommandType::BeginRenderPass, RenderCommandQueueType::Graphics>
{
	RenderPassInfo renderPassInfo;
};

struct RenderCommandEndRenderPass : RenderCommand<RenderCommandType::EndRenderPass, RenderCommandQueueType::Graphics>
{
		
};

struct RenderCommandDraw : RenderCommand<RenderCommandType::Draw, RenderCommandQueueType::Graphics>
{
	RenderBackendShaderHandle shader;
	ShaderArguments shaderArguments;
	RenderBackendBufferHandle indexBuffer;
	union
	{
		struct
		{
			uint32 numVertices;
			uint32 numInstances;
			uint32 firstVertex;
			uint32 firstInstance;
		};
		struct
		{
			uint32 numIndices;
			uint32 numInstances;
			uint32 firstIndex;
			int32 vertexOffset;
			uint32 firstInstance;
		};
	};
	PrimitiveTopology topology;
};

struct RenderCommandDrawIndirect : RenderCommand<RenderCommandType::DrawIndirect, RenderCommandQueueType::Graphics>
{
	RenderBackendShaderHandle shader;
	ShaderArguments shaderArguments;
	RenderBackendBufferHandle indexBuffer;
	RenderBackendBufferHandle argumentBuffer;
	uint64 offset;
	uint32 numDraws;
	uint32 stride;
	PrimitiveTopology topology;
};

}
