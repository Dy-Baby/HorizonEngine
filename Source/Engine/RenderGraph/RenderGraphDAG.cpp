#include "RenderGraphDAG.h"
#include "RenderGraph.h"

namespace HE
{

void RenderGraphDAG::RegisterNode(RenderGraphNode* node)
{
	nodes.push_back(node);
}

}