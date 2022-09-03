#pragma once

#include "ECS/ECSCommon.h"
#include "ECS/Entity.h"

namespace HE
{
	struct SceneHierarchyComponent
	{
		uint32 depth;
		uint32 numChildren;
		EntityHandle parent;
		EntityHandle first;
		EntityHandle next;
		EntityHandle prev;
		SceneHierarchyComponent()
			: depth(0) 
			, numChildren(0)
			, parent()
			, first()
			, next()
			, prev() 
		{

		}
	};
}
