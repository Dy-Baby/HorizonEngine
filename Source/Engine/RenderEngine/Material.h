#pragma once

#include "RenderEngine/RenderEngineCommon.h"

namespace HE
{
	//enum class BlendMode : uint32
	//{
	//	/// Alpha value is ignored
	//	Opaque,
	//	/// Either full opaque or fully transparent
	//	Mask,
	//	/// Output is combined with the background
	//	Blend
	//};

	//union TextureFlags
	//{
	//	struct
	//	{
	//		uint32 hasAlbedoTexture : 1;
	//		uint32 hasMetallicRoughnessTexture : 1;
	//		uint32 hasNormalTexture : 1;
	//		uint32 hasEmissiveTexture : 1;
	//		uint32 hasAoTexture : 1;

	//	};
	//	uint32 extras;
	//};

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

	//enum class TextureType
	//{
	//	Albedo,
	//	MetallicRoughness,
	//	Normal,
	//	AO,
	//	Emissive,
	//};

	//class Material
	//{
	//public:
	//	static SharedPtr<Material> Create();
	//	Material() = default;
	//	~Material();
	//	bool HasTexture(TextureType type);
	//	Texture* GetTexture(TextureType type);
	//	void SetTexture(TextureType type, const SharedPtr<Texture>& texture);
	//	MaterialData mData;
	//private:
	//	void SetTextureFlags_Internal(TextureType type);
	//	bool mTwoSided;
	//	SharedPtr<DescriptorSet> mDescriptorSet;
	//	SharedPtr<Buffer> mMaterialDataUniformBuffer;
	//	std::unordered_map<TextureType, SharedPtr<Texture>> mTextures;
	//};
}
