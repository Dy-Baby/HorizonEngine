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
};

struct TransformDirtyComponent
{

};

}
