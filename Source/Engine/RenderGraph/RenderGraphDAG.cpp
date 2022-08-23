#include "RenderGraphDAG.h"

import HorizonEngine.Render.RenderGraph;

namespace HE
{

void RenderGraphDAG::RegisterNode(RenderGraphNode* node)
{
	nodes.push_back(node);
}

}