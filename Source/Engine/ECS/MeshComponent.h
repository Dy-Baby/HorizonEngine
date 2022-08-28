#pragma once

#include "ECS/ECSCommon.h"

namespace HE
{
	struct StaticMeshComponent
	{
		std::string filename;

		StaticMeshComponent()
		{
			using namespace entt;
			auto factory = entt::meta<StaticMeshComponent>();
			factory.data<&StaticMeshComponent::filename, entt::as_ref_t>("filename"_hs)
				.prop("Name"_hs, std::string("filename"));
		}
	};
}