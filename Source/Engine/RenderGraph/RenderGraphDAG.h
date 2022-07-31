#pragma once

#include "RenderGraph/RenderGraphCommon.h"

namespace HE
{

enum class RenderGraphNodeType
{
	Pass,
	Resource,
};

class RenderGraphNode
{
public:
	RenderGraphNode(const char* name, RenderGraphNodeType type)
		: name(name), type(type) {}
	virtual ~RenderGraphNode() = default;
	void NeverCull()
	{ 
		refCount = InfRefCount;
	}
	bool IsCulled() const 
	{ 
		return refCount == 0; 
	}
	char const* GetName() const 
	{
		return name; 
	}
	uint32 GetRefCount() const 
	{ 
		return refCount;
	}
	const std::pmr::vector<RenderGraphNode*>& GetInputs() const 
	{ 
		return inputs;
	}
	const std::pmr::vector<RenderGraphNode*>& GetOutputs() const
	{
		return outputs;
	}
protected:
	std::pmr::vector<RenderGraphNode*> inputs;
	std::pmr::vector<RenderGraphNode*> outputs;
private:
	friend class RenderGraph;
	friend class RenderGraphDAG;
	friend class RenderGraphBuilder;
	static const uint32 InfRefCount = (uint32)-1;
	const char* name;
	RenderGraphNodeType type;
	uint32 refCount = 0;
};

class RenderGraphDAG
{
public:
	RenderGraphDAG() = default;
	~RenderGraphDAG() = default;
	void Clear()
	{
		nodes.clear();
	}
	const std::pmr::vector<RenderGraphNode*>& GetNodes() const 
	{
		return nodes; 
	}
private:
	friend class RenderGraph;
	void RegisterNode(RenderGraphNode* node);
	std::pmr::vector<RenderGraphNode*> nodes;
};

}
