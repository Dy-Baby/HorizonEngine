module;

#include <string>
#include <memory>
#include <filesystem>
#include <unordered_map>

export module HorizonEngine.Core.Asset;

import HorizonEngine.Core.Types;
import HorizonEngine.Core.Math;

namespace HE
{
	struct Asset
	{
		std::string filename;
	};

	class AssetImporter
	{
	public:
		virtual void ImportAsset(const char* file) = 0;
	};

	class AssetManager
	{
	public:
		void AddAsset(const std::string& path, Asset* asset)
		{
			ImportedAssets.emplace(path, asset);
		}
		template<typename T>
		static T* GetAsset(const std::string& assetHandle)
		{
			if (ImportedAssets.find(assetHandle) == assetHandle.end())
			{
				return nullptr;
			}
			return (T*)ImportedAssets[assetHandle];
		}
		static std::unordered_map<std::string, std::shared_ptr<Asset>> ImportedAssets;
	};

	struct Material
	{
		Vector4 baseColor;
		float metallic;
		float specular;
		float roughness;
		Vector4 emission;
		float emissionStrength;
		float alpha;
		std::string baseColorMap;
		std::string normalMap;
		std::string metallicRoughnessMap;
		std::string emissiveMap;
	};

	struct MeshElement
	{
		std::string name;
		uint32 baseVertex;
		uint32 baseIndex;
		uint32 materialIndex;
		uint32 numIndices;
		uint32 numVertices;
		Matrix4x4 transform;
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
		std::vector<MeshElement> elements;
	};
}