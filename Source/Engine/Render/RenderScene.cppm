module;

#include <vector>
#include <ECS/ECS.h>

export module HorizonEngine.Render.Scene;

import HorizonEngine.Core;
import HorizonEngine.Render.Core;
import HorizonEngine.Render.RenderGraph;
import HorizonEngine.Render.RenderPipeline;
import HorizonEngine.SceneManagement;

export namespace HE
{
	struct RenderBackend;
	
	class SkyLightRenderProxy
	{
	public:
		SkyLightRenderProxy(const SkyLightComponent* skyLightComponent)
			: component(skyLightComponent) {}

		const SkyLightComponent* component;

		RenderBackendTextureHandle environmentMap;
		RenderBackendTextureHandle irradianceEnvironmentMap;
		RenderBackendTextureHandle filteredEnvironmentMap;
	};

	struct PBRMaterialShaderParameters
	{
		int32 flags;
		Vector4 baseColor;
		float metallic;
		float roughness;
		Vector4 emission;
		uint32 baseColorMapIndex;
		uint32 normalMapIndex;
		uint32 metallicRoughnessMapIndex;
		uint32 emissiveMapIndex;
	};

	struct Renderable
	{
		uint32 firstVertex;
		uint32 firstIndex;
		uint32 numIndices;
		uint32 numVertices;
		uint32 vertexBufferIndex;
		uint32 indexBufferIndex;
		uint32 materialIndex;
		uint32 transformIndex;
	};

	class RenderScene
	{
	public:
		RenderScene();
		~RenderScene();

		Scene* scene;
		RenderBackend* renderBackend;
		
		void UpdateSkyLights();
		void UploadResources(Scene* scene);

		std::vector<Renderable> renderables;
		std::vector<RenderBackendBufferHandle> vertexBuffers[4];
		std::vector<RenderBackendBufferHandle> indexBuffers;
		std::vector<RenderBackendTextureHandle> textures;

		std::vector<PBRMaterialShaderParameters> materials;
		RenderBackendBufferHandle materialBuffer;

		std::vector<Matrix4x4> worldMatrices;
		std::vector<Matrix4x4> prevWorldMatrices;
		RenderBackendBufferHandle worldMatrixBuffer;
		RenderBackendBufferHandle prevWorldMatrixBuffer;

#if DEBUG_ONLY_RAY_TRACING_ENBALE
		RenderBackendBufferHandle worldMatrixBuffer1;
		RenderBackendRayTracingAccelerationStructureHandle bottomLevelAS;
		RenderBackendRayTracingAccelerationStructureHandle topLevelAS;
#endif
		void SetSkyLight(SkyLightRenderProxy* proxy);
		SkyLightRenderProxy* skyLight;
	};

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

	struct SceneView
	{
		SceneView() = default;
		RenderPipeline* renderPipeline;
		RenderScene* scene;
		Camera camera;
		uint32 targetWidth;
		uint32 targetHeight;
		RenderBackendTextureDesc targetDesc;
		RenderBackendTextureHandle target;
	};

	void RenderSceneView(RenderContext* renderContext, SceneView* sceneView);

	void UpdateCameraMatrices(Camera& camera, float aspectRatio)
	{
		/*Quaternion rotation = Quaternion(Math::DegreesToRadians(camera.euler));
		static Vector3 scale = Vector3(1.0f, 1.0f, 1.0f);
		camera.viewMatrix = Math::Inverse(Math::Compose(camera.position, rotation, scale));
		camera.projectionMatrix = glm::perspective(Math::DegreesToRadians(camera.fieldOfView), aspectRatio, camera.zNear, camera.zFar);*/
	}

	void RenderSceneView(
		RenderContext* renderContext,
		SceneView* view)
	{
		MemoryArena* arena = renderContext->arena;
		RenderBackend* renderBackend = renderContext->renderBackend;
		RenderPipeline* activePipeline = view->renderPipeline;
		uint32 deviceMask = ~0u;

		UpdateCameraMatrices(view->camera, (float)(view->targetWidth / view->targetHeight));
		{
			RenderGraph renderGraph(arena);
        
			activePipeline->SetupRenderGraph(view, &renderGraph);
        
			renderContext->commandLists.clear();
			renderGraph.Execute(renderContext);

			/*if (false)
			{
				HE_LOG_INFO(renderGraph.Graphviz());
			}*/
		}

		uint32 numCommandLists = (uint32)renderContext->commandLists.size();
		RenderCommandList** commandLists = renderContext->commandLists.data();
		RenderBackendSubmitRenderCommandLists(renderBackend, commandLists, numCommandLists);
	}
}