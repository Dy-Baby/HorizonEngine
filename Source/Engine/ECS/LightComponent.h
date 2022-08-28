#pragma once

#include "ECS/ECSCommon.h"

namespace HE
{
	struct DirectionalLightComponent
	{
		Vector3 color;
		float intensity;

		DirectionalLightComponent()
		{
			using namespace entt;
			auto factory = entt::meta<DirectionalLightComponent>();
			factory.data<&DirectionalLightComponent::intensity, entt::as_ref_t>("Intensity"_hs)
				.prop("Name"_hs, std::string("Intensity"));
			factory.data<&DirectionalLightComponent::color, entt::as_ref_t>("Color"_hs)
				.prop("Name"_hs, std::string("Color"));
		}
	};
}
