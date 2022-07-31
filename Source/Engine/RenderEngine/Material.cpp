#include "Material.h"

namespace HE
{
	//SharedPtr<Material> Material::Create()
	//{
	//	return SharedPtr<Material>(new Material);
	//}

	//Material::~Material()
	//{

	//}

	//VkDescriptorSet Material::GetDescriptorSetHandle() const 
	//{
	//	return mDescriptorSet->GetHandle();
	//}

	//bool Material::HasTexture(TextureType type)
	//{
	//	// TODO
	//	return false;
	//}

	//Texture* Material::GetTexture(TextureType type)
	//{
	//	return mTextures[type].get();
	//}

	//void Material::SetTextureFlags_Internal(TextureType type)
	//{
	//	switch (type)
	//	{
	//		case TextureType::Albedo:
	//		{
	//			mData.hasAlbedoTexture = 1.0f;
	//			break;
	//		}
	//		case TextureType::MetallicRoughness:
	//		{
	//			mData.hasMetallicRoughnessTexture = 1.0f;
	//			break;
	//		}
	//		case TextureType::Normal:
	//		{
	//			mData.hasNormalTexture = 1.0f;
	//			break;
	//		}
	//		case TextureType::Emissive:
	//		{
	//			mData.hasEmissiveTexture = 1.0f;
	//			break;
	//		}
	//		case TextureType::AO:
	//		{
	//			mData.hasAoTexture = 1.0f;
	//			break;
	//		}
	//		default:
	//		{
	//			HORIZON_INVALID_ENUM_VALUE();
	//			break;
	//		}
	//	}
	//}

	//void Material::SetTexture(TextureType type, const SharedPtr<Texture>& texture)
	//{
	//	mTextures[type] = texture;
	//	SetTextureFlags_Internal(type);
	//}
}