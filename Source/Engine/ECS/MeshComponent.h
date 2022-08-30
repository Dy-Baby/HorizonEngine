#pragma once

#include "ECS/ECSCommon.h"

namespace HE
{
	struct StaticMeshComponent
	{
		std::string meshSource;

		StaticMeshComponent()
		{
			using namespace entt;
			auto factory = entt::meta<StaticMeshComponent>();
			factory.data<&StaticMeshComponent::meshSource, entt::as_ref_t>("Mesh Source"_hs)
				.prop("Name"_hs, std::string("Mesh Source"));
		}
	};
}