#pragma once

#include "Core/Core.h"
#include "RenderEngine/RenderEngineCommon.h"
#include "RenderEngine/Mesh.h"

namespace HE
{

class EntityManager;
struct RenderBackend;
struct Material;

class Scene
{
public:
	Scene();
	~Scene();

	RenderBackend* renderBackend;

	uint32 numMeshes;
	std::vector<Mesh> meshes;
	std::vector<Matrix4x4> worldMatrices;

	uint32 numMaterials;
	std::vector<Material> materials;

	uint32 numTextures;
	std::vector<std::string> texturePaths;
	std::vector<RenderBackendTextureHandle> textures;

	RenderBackendBufferHandle worldMatrixBuffer;
	RenderBackendBufferHandle prevWorldMatrixBuffer;
	RenderBackendBufferHandle materialBuffer;

	void Update();
	void UploadResources();

	EntityManager* GetEntityManager()
	{
		return entityManager;
	}
private:
	EntityManager* entityManager;
};

}