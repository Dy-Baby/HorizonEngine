module HorizonEngine.Render.RenderGraph;

namespace HE
{
	RenderGraphTextureHandle RenderGraphBuilder::CreateTransientTexture(const RenderGraphTextureDesc& desc, const char* name)
	{
		return RenderGraphTextureHandle();
	}

	RenderGraphBufferHandle RenderGraphBuilder::CreateTransientBuffer(const RenderGraphBufferDesc& desc, const char* name)
	{
		return RenderGraphBufferHandle();
	}

	RenderGraphTextureHandle RenderGraphBuilder::ReadTexture(RenderGraphTextureHandle handle, RenderBackendResourceState finalState, const RenderGraphTextureSubresourceRange& range)
	{
		pass->textureStates.push_back(RenderGraphPass::TextureState{
			.texture = renderGraph->textures[handle.GetIndex()],
			.state = finalState,
			.subresourceRange = range
			});
		// renderGraph->textures[handle.GetIndex()].refCount++;
		return handle;
	}

	RenderGraphTextureHandle RenderGraphBuilder::WriteTexture(RenderGraphTextureHandle handle, RenderBackendResourceState finalState, const RenderGraphTextureSubresourceRange& range)
	{
		pass->textureStates.push_back(RenderGraphPass::TextureState{
			.texture = renderGraph->textures[handle.GetIndex()],
			.state = finalState,
			.subresourceRange = range
			});
		/*handle = RenderGraphTextureHandle::CreateNewVersion(handle);
		pass->refCount++;*/
		return handle;
	}

	RenderGraphTextureHandle RenderGraphBuilder::ReadWriteTexture(RenderGraphTextureHandle handle, RenderBackendResourceState finalState, const RenderGraphTextureSubresourceRange& range)
	{
		pass->textureStates.push_back(RenderGraphPass::TextureState{
			.texture = renderGraph->textures[handle.GetIndex()],
			.state = finalState,
			.subresourceRange = range
			});
		//pass->readResourceInfos[pass->numReadResourceInfos++] = {
		//	.type = RenderGraphResourceType::Texture,
		//	.readTexture = {
		//		.handle = handle,
		//		.finalState = finalState,
		//		.subresourceRange = range,
		//	}
		//};
		//renderGraph->textures[handle.GetIndex()].refCount++;
		//handle = RenderGraphTextureHandle::CreateNewVersion(handle);
		//pass->writeResourceInfos[pass->numWriteResourceInfos++] = {
		//	.type = RenderGraphResourceType::Texture,
		//	.writeTexture = {
		//		.handle = handle,
		//		.finalState = finalState,
		//		.subresourceRange = range,
		//	}
		//};
		//pass->refCount++;
		return handle;
	}

	RenderGraphBufferHandle RenderGraphBuilder::ReadBuffer(RenderGraphBufferHandle handle, RenderBackendResourceState initalState)
	{
		return RenderGraphBufferHandle();
	}

	RenderGraphBufferHandle RenderGraphBuilder::WriteBuffer(RenderGraphBufferHandle handle, RenderBackendResourceState initalState)
	{
		return RenderGraphBufferHandle();
	}

	RenderGraphBufferHandle RenderGraphBuilder::ReadWriteBuffer(RenderGraphBufferHandle handle, RenderBackendResourceState initalState)
	{
		return RenderGraphBufferHandle();
	}

	void RenderGraphBuilder::BindColorTarget(uint32 slot, RenderGraphTextureHandle handle, RenderTargetLoadOp loadOp, RenderTargetStoreOp storeOp, uint32 mipLevel, uint32 arraylayer)
	{
		pass->colorTargets[slot] = {
			.texture = handle,
			.loadOp = loadOp,
			.storeOp = storeOp,
			.mipLevel = mipLevel,
			.arrayLayer = arraylayer,
		};
	}

	void RenderGraphBuilder::BindDepthStencilTarget(RenderGraphTextureHandle handle, RenderTargetLoadOp depthLoadOp, RenderTargetStoreOp depthStoreOp, RenderTargetLoadOp stencilLoadOp, RenderTargetStoreOp stencilStoreOp)
	{
		pass->depthStentcilTarget = {
			.texture = handle,
			.depthLoadOp = depthLoadOp,
			.depthStoreOp = depthStoreOp,
			.stencilLoadOp = stencilLoadOp,
			.stencilStoreOp = stencilStoreOp,
		};
	}
}