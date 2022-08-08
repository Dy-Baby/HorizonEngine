#pragma once

#include "RenderGraph/RenderGraphCommon.h"
#include "RenderGraph/RenderGraphDAG.h"
#include "RenderGraph/RenderGraphRegistry.h"
#include "RenderGraph/RenderGraphResources.h"

namespace HE
{

enum class RenderGraphPassFlags : uint8
{
    None           = 0,
    Raster         = (1 << 0),
    Compute        = (1 << 1),
    AsyncCompute   = (1 << 2),
    RayTrace       = (1 << 3),
    NeverGetCulled = (1 << 4),
    SkipRenderPass = (1 << 6),
};
ENUM_CLASS_OPERATORS(RenderGraphPassFlags);

class RenderGraphPass : public RenderGraphNode
{
public:
    virtual ~RenderGraphPass() = default;
    bool IsAsyncCompute() const 
    { 
        return HAS_ANY_FLAGS(flags, RenderGraphPassFlags::AsyncCompute);
    }
    RenderGraphPassFlags GetFlags() const 
    { 
        return flags;
    }
    void Graphviz(const std::stringstream& stream) const;
    virtual void Execute(RenderGraphRegistry& registry, RenderCommandList& commandList) = 0;
protected:
    friend class RenderGraph;
    friend class RenderGraphBuilder;

    RenderGraphPass(const char* name, RenderGraphPassFlags flags)
        : RenderGraphNode(name, RenderGraphNodeType::Pass)
        , flags(flags) {}

    RenderGraphPassFlags flags;

    struct TextureState
    {
        RenderGraphTexture* texture;
        RenderBackendResourceState state;
        RenderGraphTextureSubresourceRange subresourceRange;
    };

    struct BufferState
    {
        RenderGraphBuffer* buffer;
        RenderBackendResourceState state;
    };

    std::vector<TextureState> textureStates;
    std::vector<BufferState> bufferStates;

    std::vector<RenderBackendBarrier> barriers;

    struct ColorRenderTarget
    {
        RenderGraphTextureHandle texture;
        RenderTargetLoadOp loadOp;
        RenderTargetStoreOp storeOp;
        uint32 mipLevel;
        uint32 arrayLayer;
    };
    struct DepthStencilRenderTarget
    {
        RenderGraphTextureHandle texture;
        RenderTargetLoadOp depthLoadOp;
        RenderTargetStoreOp depthStoreOp;
        RenderTargetLoadOp stencilLoadOp;
        RenderTargetStoreOp stencilStoreOp;
    };
    ColorRenderTarget colorTargets[MaxNumSimultaneousColorRenderTargets];
    DepthStencilRenderTarget depthStentcilTarget;
};

class RenderGraphLambdaPass : public RenderGraphPass
{
public:
    using Lambda = std::function<void(RenderGraphRegistry&, RenderCommandList&)>;
    ~RenderGraphLambdaPass() = default;
private:
    friend class RenderGraph;
    RenderGraphLambdaPass(const char* name, RenderGraphPassFlags flags) : RenderGraphPass(name, flags) {}
    void SetExecuteCallback(Lambda&& execute) { executeCallback = std::move(execute); }
    void Execute(RenderGraphRegistry& registry, RenderCommandList& commandList) override
    {   
        ASSERT(executeCallback);
        executeCallback(registry, commandList);
    }
    Lambda executeCallback;
};

}
