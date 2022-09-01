module;

#include <sstream>

module HorizonEngine.Render.RenderGraph;

import HorizonEngine.Core;
import HorizonEngine.Render.Core;

namespace HE
{
	struct RenderBackendTransientResource
	{

	};

	RenderGraph::RenderGraph(MemoryArena* arena)
		: blackboard(arena)
		, arena(arena)
	{

	}

	RenderGraph::~RenderGraph()
	{

	}

	std::string RenderGraph::Graphviz() const
	{
		std::stringstream stream;
		stream << "digraph render_graph {\n\trankdir = LR;\n"
			   << "\tsubgraph cluster_0 {\n"
			   << "\t\tstyle=filled;\n"
			   << "\t\tcolor = lightgrey;\n"
			   << "\t\tlabel = \"Graphics\"\n"
			   << "\t\tnode[shape = rect, style = filled, color = white];\n";
		for (const auto& pass : passes)
		{
			if (pass->IsCulled())
			{
				continue;
			}
			pass->Graphviz(stream);
		}
		stream << "\t}\n}\n";
		return std::move(stream.str());
	}

	RenderGraphTextureHandle RenderGraph::CreateTexture(const RenderGraphTextureDesc& desc, const char* name)
	{
		uint32 index = (uint32)textures.size();
		RenderGraphTextureHandle handle = RenderGraphTextureHandle(index, 0);
		RenderGraphTexture* texture = AllocObject<RenderGraphTexture>(name, desc);
		textures.push_back(texture);
		dag.RegisterNode(texture);
		return handle;
	}

	RenderGraphBufferHandle RenderGraph::CreateBuffer(const RenderGraphBufferDesc& desc, const char* name)
	{
		uint32 index = (uint32)buffers.size();
		RenderGraphBufferHandle handle = RenderGraphBufferHandle(index, 0);
		RenderGraphBuffer* buffer = AllocObject<RenderGraphBuffer>(name, desc);
		buffers.push_back(buffer);
		dag.RegisterNode(buffer);
		return handle;
	}

	//RenderGraphTextureSRVHandle RenderGraph::CreateSRV(RenderGraphTextureHandle texture, const RenderGraphTextureSRVDesc& desc)
	//{
	//	uint32 index = (uint32)textureSRVs.size();
	//	RenderGraphTextureSRVHandle handle = RenderGraphTextureSRVHandle(index, 0);
	//	RenderGraphTextureSRV* srv = AllocObject<RenderGraphTextureSRV>(texture, desc);
	//	textureSRVs.push_back(srv);
	//	return handle;
	//}
	//
	//RenderGraphTextureUAVHandle RenderGraph::CreateUAV(RenderGraphTextureHandle texture, uint32 mipLevel)
	//{
	//	uint32 index = (uint32)textureUAVs.size();
	//	RenderGraphTextureUAVHandle handle = RenderGraphTextureUAVHandle(index, 0);
	//	RenderGraphTextureUAV* uav = AllocObject<RenderGraphTextureUAV>(texture, mipLevel);
	//	textureUAVs.push_back(uav);
	//	return handle;
	//}
	//
	//RenderGraphBufferUAVHandle RenderGraph::CreateUAV(RenderGraphBufferHandle buffer)
	//{
	//	uint32 index = (uint32)bufferUAVs.size();
	//	RenderGraphBufferUAVHandle handle = RenderGraphBufferUAVHandle(index, 0);
	//	RenderGraphBufferUAV* uav = AllocObject<RenderGraphBufferUAV>(buffer);
	//	bufferUAVs.push_back(uav);
	//	return handle;
	//}

	RenderGraphTextureHandle RenderGraph::ImportExternalTexture(RenderBackendTextureHandle renderBackendTexture, const RenderBackendTextureDesc& desc, RenderBackendResourceState initialState, char const* name)
	{
		uint32 index = (uint32)textures.size();
		RenderGraphTextureHandle handle = RenderGraphTextureHandle(index, 0);
		RenderGraphTexture* texture = AllocObject<RenderGraphTexture>(name, desc);
		texture->SetRenderBackendTexture(renderBackendTexture, initialState);
		textures.push_back(texture);
		dag.RegisterNode(texture);
		externalTextures.emplace(renderBackendTexture, handle);
		return handle;
	}

	RenderGraphBufferHandle RenderGraph::ImportExternalBuffer(RenderBackendBufferHandle renderBackendBuffer, const RenderBackendBufferDesc& desc, RenderBackendResourceState initialState, char const* name)
	{
		uint32 index = (uint32)buffers.size();
		RenderGraphBufferHandle handle = RenderGraphBufferHandle(index, 0);
		RenderGraphBuffer* buffer = AllocObject<RenderGraphBuffer>(name, desc);
		buffer->SetRenderBackendBuffer(renderBackendBuffer, initialState);
		buffers.push_back(buffer);
		dag.RegisterNode(buffer);
		externalBuffers.emplace(renderBackendBuffer, handle);
		return handle;
	}

	void RenderGraph::ExportTextureDeferred(RenderGraphTextureHandle texture, RenderGraphPersistentTexture* outPersistentTexture)
	{
	}

	bool RenderGraph::Compile()
	{
		if (passes.empty())
		{
			return false;
		}

		auto& nodes = dag.nodes;

		// Push nodes with a 0 reference count on a stack.
		std::vector<RenderGraphNode*> nodesToCull;
		for (RenderGraphNode* node : nodes)
		{
			if (node->GetRefCount() == 0)
			{
				nodesToCull.push_back(node);
			}
		}

		while (!nodesToCull.empty())
		{
			RenderGraphNode* node = nodesToCull.back();
			nodesToCull.pop_back();
			for (RenderGraphNode* input : node->inputs)
			{
				input->refCount--;
				if (input->GetRefCount() == 0)
				{
					nodesToCull.push_back(input);
				}
			}
		}

		return true;
	}

	void RenderGraph::Clear()
	{
		dag.Clear();
		passes.clear();
		textures.clear();
		buffers.clear();
		externalTextures.clear();
		externalBuffers.clear();
	}

	void RenderGraph::Execute(RenderContext* context)
	{
		Compile();

		std::string temp = Graphviz();
		HE_LOG_INFO("{}", temp);

		RenderBackend* renderBackend = context->renderBackend;
		RenderCommandList* commandList = AllocObject<RenderCommandList>(arena);

		for (auto& texture : textures)
		{
			if (!texture->IsImported())
				// if (!texture->IsCulled() && !texture->IsImported() && !texture->HasRenderBackendTexture())
			{
				RenderBackendTextureHandle handle = gRenderGraphResourcePool->FindOrCreateTexture(renderBackend, &texture->GetDesc(), texture->GetName());
				texture->SetRenderBackendTexture(handle, RenderBackendResourceState::Undefined);
			}
		}

		for (auto& pass : passes)
		{
			/*if (pass->IsCulled())
			{
				continue;
			}*/
			for (auto& state : pass->textureStates)
			{
				RenderGraphTexture* texture = state.texture;
				if (state.state != texture->tempState)
				{
					TextureSubresourceRange range = {
						.firstLevel = 0,
						.mipLevels = REMAINING_MIP_LEVELS,
						.firstLayer = 0,
						.arrayLayers = REMAINING_ARRAY_LAYERS,
					};
					RenderBackendBarrier barrier = RenderBackendBarrier(
						texture->GetRenderBackendTexture(),
						range,
						texture->tempState,
						state.state);
					texture->tempState = state.state;
					pass->barriers.push_back(barrier);
				}
			}
		}

		for (auto& pass : passes)
		{
			/*if (pass->IsCulled())
			{
				continue;
			}
			*/
			RenderGraphPassFlags flags = pass->GetFlags();
			RenderGraphRegistry registry(this, pass);

			// commandList.BeginTimingQuery(, pass->GetName());

			if (!pass->barriers.empty())
			{
				commandList->Transitions(pass->barriers.data(), (uint32)pass->barriers.size());
			}

			if (HAS_ANY_FLAGS(flags, RenderGraphPassFlags::Raster) && !HAS_ANY_FLAGS(flags, RenderGraphPassFlags::SkipRenderPass))
			{
				RenderPassInfo renderPass = {};
				for (uint32 i = 0; i < MaxNumSimultaneousColorRenderTargets; i++)
				{
					if (pass->colorTargets[i].texture)
					{
						renderPass.colorRenderTargets[i] = {
							.texture = registry.GetRenderBackendTexture(pass->colorTargets[i].texture),
							.mipLevel = pass->colorTargets[i].mipLevel,
							.arrayLayer = pass->colorTargets[i].arrayLayer,
							.loadOp = pass->colorTargets[i].loadOp,
							.storeOp = pass->colorTargets[i].storeOp,
						};
					}
				}
				if (pass->depthStentcilTarget.texture)
				{
					renderPass.depthStencilRenderTarget = {
						.texture = registry.GetRenderBackendTexture(pass->depthStentcilTarget.texture),
						.depthLoadOp = pass->depthStentcilTarget.depthLoadOp,
						.depthStoreOp = pass->depthStentcilTarget.depthStoreOp,
						.stencilLoadOp = pass->depthStentcilTarget.stencilLoadOp,
						.stencilStoreOp = pass->depthStentcilTarget.stencilStoreOp,
					};
				}
				commandList->BeginRenderPass(renderPass);
			}

			pass->Execute(registry, *commandList);

			if (HAS_ANY_FLAGS(flags, RenderGraphPassFlags::Raster) && !HAS_ANY_FLAGS(flags, RenderGraphPassFlags::SkipRenderPass))
			{
				commandList->EndRenderPass();
			}
			// commandList.EndTimingQuery();
		}

		context->commandLists.push_back(commandList);

		Clear();
	}
}