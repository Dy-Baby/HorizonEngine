#pragma once

#include "ECS/ECSCommon.h"

namespace HE
{

enum class LightType
{
	Directional,
	Count
};

struct LightComponent
{
	LightType type;
	Vector3 color;
	float intensity;
	// Directional Light
	Vector3 direction;
};

}
