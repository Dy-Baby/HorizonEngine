#pragma once

#include "Core/Core.h"
#include "RenderEngine/RenderEngineCommon.h"
#include "RenderEngine/RenderBackend.h"

namespace HE
{

struct Camera
{
	float fieldOfView;
	float aspectRatio;
	float zNear;
	float zFar;
	Vector3 position;
	Vector3 euler;
	Matrix4x4 viewMatrix;
	Matrix4x4 invViewMatrix;
	Matrix4x4 projectionMatrix;
	Matrix4x4 invProjectionMatrix;
};

struct RenderContext;
class RenderPipeline;
class Scene;
struct SceneView
{
	SceneView() = default;
	RenderPipeline* renderPipeline;
	Scene* scene;
	Camera camera;
	uint32 targetWidth;
	uint32 targetHeight;
	RenderBackendTextureDesc targetDesc;
	RenderBackendTextureHandle target;
};

extern void RenderSceneView(
	RenderContext* renderContext,
	SceneView* sceneView);

}