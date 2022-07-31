#pragma once

#include "Core/Core.h"

namespace HE
{

struct SceneView;
class RenderGraph;

class RenderPipeline
{
public:
	virtual void SetupRenderGraph(SceneView* view, RenderGraph* renderGraph) = 0;
};

}