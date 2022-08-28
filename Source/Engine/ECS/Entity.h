#pragma once

#include "ECS/ECSCommon.h"

namespace HE
{
	class EntityHandle
	{
	public:
		EntityHandle() : handle(entt::null) {}
		EntityHandle(const entt::entity& entity) : handle(entity) {}
		~EntityHandle() {}
		FORCEINLINE constexpr operator bool() const
		{
			return handle == entt::null;
		}
		FORCEINLINE constexpr operator entt::entity() const
		{
			return handle;
		}
	private:
		entt::entity handle;
	};

	class EntityManager
	{
	public:

		EntityManager() {}
		~EntityManager() {}

		void Clear();

		EntityHandle CreateEntity(const char* name);

		void DestroyEntity(EntityHandle entity);

		bool HasEntity(EntityHandle entity);

		template<typename... Components>
		auto GetGroup() const
		{ 
			return registry.group<Components ...>(); 
		}

		template<typename... Components>
		auto GetView() const
		{ 
			return registry.view<Components ...>(); 
		}

		template<typename Component>
		auto OnConstruct() 
		{ 
			return registry.on_construct<Component>();
		}

		template<typename Component>
		auto OnUpdate() 
		{ 
			return registry.on_update<Component>();
		}

		template<typename Component>
		bool HasComponent(EntityHandle entity)
		{
			return registry.try_get<Component>(entity);
		}

		template<typename... Component>
		bool HasAllComponents(EntityHandle entity) 
		{ 
			return registry.all_of<Component ...>(entity);
		}

		template<typename... Component>
		bool HasAnyComponent(EntityHandle entity) 
		{ 
			return registry.any_of<Component ...>(entity);
		}

		template<typename Component, typename... Args>
		Component& AddComponent(EntityHandle entity, Args&&... args)
		{
			return registry.emplace<Component>(entity, std::forward<Args>(args)...);
		}

		template<typename Component, typename... Args>
		void AddOrReplaceComponent(EntityHandle entity, Args&&... args) 
		{ 
			registry.emplace_or_replace<Component>(entity, std::forward<Args>(args)...);
		}

		template<typename Component, typename... Args>
		void ReplaceComponent(EntityHandle entity, Args&&... args) 
		{
			registry.replace<Component>(entity, std::forward<Args>(args)...);
		}

		template<typename Component>
		Component& GetComponent(EntityHandle entity) const 
		{ 
			return registry.get<Component>(entity);
		}

		template<typename Component>
		Component& GetComponent(EntityHandle entity)
		{
			return registry.get<Component>(entity);
		}

		template<typename... Component>
		decltype(auto) GetComponents(EntityHandle entity) const
		{ 
			return registry.get<Component ...>(entity);
		}

		template<typename... Component>
		decltype(auto) GetComponents(EntityHandle entity)
		{
			return registry.get<Component ...>(entity);
		}

		template<typename Component, typename... Args>
		Component& GetOrAddComponent(EntityHandle entity, Args&&... args)
		{ 
			return registry.get_or_emplace<Component>(entity, std::forward<Args>(args)...);
		}

		template<typename Component>
		Component* TryGetComponent(EntityHandle entity) 
		{ 
			return registry.try_get<Component>(entity);
		}

		template<typename... Component>
		auto TryGetComponents(EntityHandle entity) 
		{ 
			return registry.try_get<Component ...>(entity);
		}

		template<typename Component>
		void RemoveComponent(EntityHandle entity) 
		{ 
			return registry.remove<Component>(entity);
		}

	private:
		entt::registry registry;
	};
}
