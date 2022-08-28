module;

#include <string>
#include <map>
#include <string>
#include <memory>
#include <filesystem>

#include "ECS/ECS.h"

export module HorizonEngine.SceneManagement;

import HorizonEngine.Core;

export namespace HE
{
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

	struct MeshElement
	{
		std::string name;
		uint32 baseVertex;
		uint32 baseIndex;
		uint32 materialIndex;
		uint32 numIndices;
		uint32 numVertices;
		Matrix4x4 localToWorld;
	};

	class Mesh : public Asset
	{
	public:

	private:
		std::vector<Vector3> positions;
		std::vector<Vector3> normals;
		std::vector<Vector4> tangents;
		std::vector<Vector2> texCoords;
		std::vector<uint32> indices;

		std::vector<Material> materials;
		std::vector<std::string> textures;
		std::vector<MeshElement> elements;
	};

	/**
	 * Run-time data structure for *.horizon file.
	 */
	class Scene
	{
	public:
		Scene();
		~Scene();
		std::string name;
		std::string url;
		bool isLoaded;
		bool isDirty;
		EntityManager* GetEntityManager()
		{
			return entityManager;
		}
	private:
		friend class SceneSerializer;
		Guid guid;
		EntityManager* entityManager;
	};

	/**
	 * Scene management at runtime.
	 */
	class SceneManager
	{
	public:
		static uint32 LoadedSceneCount;
		static Scene* ActiveScene;
		static std::map<std::string, std::shared_ptr<Scene>> SceneMapByName;
		static Scene* CreateScene(const std::string& name);
		static Scene* GetActiveScene();
		static Scene* GetSceneByName(const std::string& name);
		static void SetActiveScene(Scene* scene);
		static void LoadScene(const std::string& name);
		static void LoadSceneAsync(const std::string& name);
		static void UnloadSceneAsync(Scene* scene);
		static void MergeScenes(Scene* dstScene, Scene* srcScene);
	};

	class SceneSerializer
	{
	public:
		inline static std::string_view DefaultExtension = ".horizon";
		SceneSerializer(Scene* scene) : scene(scene) {}
		void Serialize(const std::filesystem::path& filename);
		bool Deserialize(const std::filesystem::path& filename);
	private:
		Scene* scene;
	};
}