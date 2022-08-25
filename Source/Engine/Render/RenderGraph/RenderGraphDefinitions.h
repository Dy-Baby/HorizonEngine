#pragma once

#define RENDER_GRAPH_ENABLE_DEBUG 1

namespace HE
{
    enum
    {
        RenderGraphPassMaxNumReadResources = 32,
        RenderGraphPassMaxNumWriteResources = 16,
        RenderGraphPassMaxCreateTransientResources = 16,
    };
}