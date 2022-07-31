#pragma once

#include "Core/Core.h"

namespace HE
{
	struct CameraTransform
	{
		Matrix4x4 view = Matrix4x4(1);
		Matrix4x4 proj = Matrix4x4(1);
		Matrix4x4 viewProj = Matrix4x4(1);
		Matrix4x4 invViewProj = Matrix4x4(1);
		Vector3 position = Vector3(0, 0, 0);
		Vector3 targetPoint = Vector3(0, 0, -1);
		Quaternion rotation = Quaternion(1, 0, 0, 0); // Unit glm::quat is (w = 1, x = 0, y = 0, z = 0).
		Vector3 U = Vector3(1, 0, 0);
		Vector3 V = Vector3(0, 1, 0);
		Vector3 W = Vector3(0, 0, 1);
	};

	struct CameraParameters
	{
		float aspectRatio = 16.0f / 9.0f;
		float farZ = 1000.0f;
		float nearZ = 0.1f;
		float yFov = M_PI / 2.0f;
		float aperture = 0.0f;
		float focalLength = 1.0f;
		float focalDistance = 30.0f;
		float padding;
	};
}