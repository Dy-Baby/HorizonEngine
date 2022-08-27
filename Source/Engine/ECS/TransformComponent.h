#pragma once

#include "ECS/ECSCommon.h"

namespace HE
{
	struct TransformComponent
	{
		Vector3 position;
		Vector3 rotation;
		Vector3 scale;
		Matrix4x4 world;

		TransformComponent()
		{
			using namespace entt;
			auto factory = entt::meta<TransformComponent>();
			factory.data<&TransformComponent::scale, entt::as_ref_t>("Scale"_hs)
				.prop("Name"_hs, std::string("Scale"));
			factory.data<&TransformComponent::rotation, entt::as_ref_t>("Rotation"_hs)
				.prop("Name"_hs, std::string("Rotation"));
			factory.data<&TransformComponent::position, entt::as_ref_t>("Position"_hs)
				.prop("Name"_hs, std::string("Position"));
		}
	};

	struct TransformDirtyComponent
	{

	};
}
