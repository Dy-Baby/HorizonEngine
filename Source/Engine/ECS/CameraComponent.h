#pragma once

#include "ECS/ECSCommon.h"

namespace HE
{

enum class CameraType
{
	Perpective,
	Orthographic,
};

struct CameraComponent
{
	CameraType type;
	float nearPlane;
	float farPlane;
	float fieldOfView;
	float aspectRatio;
};

//void UpdateCameraViewMatrix(CameraComponent* camera, const Transform& transform);
//void UpdateCameraProjectionMatrix(CameraComponent* camera);

}
