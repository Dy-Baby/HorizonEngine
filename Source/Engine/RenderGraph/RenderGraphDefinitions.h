#pragma once

#include "Core/CoreCommon.h"

namespace HE
{

#define RENDER_GRAPH_ENABLE_DEBUG 1

enum
{
    RenderGraphPassMaxNumReadResources = 32,
    RenderGraphPassMaxNumWriteResources = 16,
    RenderGraphPassMaxCreateTransientResources = 16,
};

}