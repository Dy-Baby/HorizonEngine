#include "Entity.h"

namespace HE
{
	EntityHandle EntityManager::CreateEntity(const char* name)
	{
		EntityHandle entity(registry.create());
		return entity;
	}

	void EntityManager::DestroyEntity(EntityHandle entity)
	{
		registry.destroy(entity);
	}

	void EntityManager::Clear()
	{
		registry.clear();
	}

	bool EntityManager::HasEntity(EntityHandle entity)
	{
		return registry.has(entity);
	}
}