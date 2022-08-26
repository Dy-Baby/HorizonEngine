module;

#include <vector>

export module HorizonEngine.Render.Scene;

import HorizonEngine.Core;
import HorizonEngine.Render.Core;
import HorizonEngine.Render.RenderGraph;
import HorizonEngine.Render.RenderPipeline;

export namespace HE
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
	class EntityManager;
	struct RenderBackend;
	struct Material
	{
		Vector4 baseColor;
		float metallic;
		float specular;
		float roughness;
		Vector4 emission;
		float emissionStrength;
		float alpha;
		int32 baseColorMapIndex;
		int32 normalMapIndex;
		int32 metallicRoughnessMapIndex;
	};

	struct MaterialInstanceData
	{
		Vector4 baseColor;
		float metallic;
		float roughness;
		uint32 baseColorMapIndex;
		uint32 normalMapIndex;
		uint32 metallicRoughnessMapIndex;
	};

	struct Mesh
	{
		std::vector<Vector3> positions;
		std::vector<Vector3> normals;
		std::vector<Vector4> tangents;
		std::vector<Vector2> texCoords;
		std::vector<uint32> indices;

		RenderBackendBufferHandle vertexBuffers[4];
		RenderBackendBufferHandle indexBuffer;

		uint32 numIndices;
		uint32 numVertices;
		uint32 vertexStrides[4];
		uint32 materialID;
	};

	class RenderScene
	{
	public:
		RenderScene();
		~RenderScene();

		RenderBackend* renderBackend;

		uint32 numMeshes = 0;
		std::vector<Mesh> meshes;
		std::vector<Matrix4x4> worldMatrices;

		uint32 numMaterials = 0;
		std::vector<Material> materials;

		uint32 numTextures = 0;
		std::vector<std::string> texturePaths;
		std::vector<RenderBackendTextureHandle> textures;

		RenderBackendBufferHandle worldMatrixBuffer;
		RenderBackendBufferHandle prevWorldMatrixBuffer;
		RenderBackendBufferHandle materialBuffer;

		RenderBackendRayTracingAccelerationStructureHandle bottomLevelAS;
		RenderBackendRayTracingAccelerationStructureHandle topLevelAS;

		void Update();

		void UploadResources();

		EntityManager* GetEntityManager()
		{
			return entityManager;
		}
	private:
		EntityManager* entityManager;
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