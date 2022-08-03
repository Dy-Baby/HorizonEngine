#include "Scene.h"
#include "ECS/Entity.h"
#include "RenderBackend.h"
#include "Material.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace HE
{
	Scene::Scene()
	{
		entityManager = new EntityManager();
	}

	Scene::~Scene()
	{
		delete entityManager;
	}

	RenderBackendTextureHandle LoadTextureFromFile(RenderBackend* renderBackend, const char* filename)
	{
		int iw = 0, ih = 0, c = 0;
		stbi_set_flip_vertically_on_load(true);
		unsigned char* data = stbi_load(filename, &iw, &ih, &c, STBI_default);
		if (data == nullptr)
		{
			return RenderBackendTextureHandle::NullHandle;
		}
		uint64 bufferSize = iw * ih * 4;
		unsigned char* buffer = (unsigned char*)_aligned_malloc(bufferSize, 32);

		for (uint32 y = 0; y < (uint32)ih; y++)
		{
			for (uint32 x = 0; x < (uint32)iw; x++)
			{
				uint32 idx = x + y * iw;
				switch (c)
				{
				case STBI_grey:
				{
					buffer[idx * 4 + 0] = data[idx];
					buffer[idx * 4 + 2] = buffer[idx * 4 + 1] = buffer[idx * 4 + 0];
					buffer[idx * 4 + 3] = 255;
					break;
				}
				case STBI_grey_alpha:
				{
					buffer[idx * 4 + 0] = data[idx * 2 + 0];
					buffer[idx * 4 + 2] = buffer[idx * 4 + 1] = buffer[idx * 4 + 0];
					buffer[idx * 4 + 3] = data[idx * 2 + 1];
					break;
				}
				case STBI_rgb:
				{
					buffer[idx * 4 + 0] = data[idx * 3 + 0];
					buffer[idx * 4 + 1] = data[idx * 3 + 1];
					buffer[idx * 4 + 2] = data[idx * 3 + 2];
					buffer[idx * 4 + 3] = 255;
					break;
				}
				case STBI_rgb_alpha:
				{
					buffer[idx * 4 + 0] = data[idx * 4 + 0];
					buffer[idx * 4 + 1] = data[idx * 4 + 1];
					buffer[idx * 4 + 2] = data[idx * 4 + 2];
					buffer[idx * 4 + 3] = data[idx * 4 + 3];
					break;
				}
				default: break;
				}
			}
		}

		stbi_image_free(data);

		RenderBackendTextureDesc desc = RenderBackendTextureDesc::Create2D(iw, ih, PixelFormat::RGBA8Unorm, TextureCreateFlags::ShaderResource);
		RenderBackendTextureHandle texture = RenderBackendCreateTexture(renderBackend, ~0u, &desc, buffer, filename);

		_aligned_free(buffer);
		
		return texture;
	}

	void Scene::UploadResources()
	{
		uint32 deviceMask = ~0u;

		for (uint32 i = 0; i < numMeshes; i++)
		{
			RenderBackendBufferDesc vertexBuffer0Desc = RenderBackendBufferDesc::CreateByteAddress(meshes[i].numVertices * sizeof(Vector3));
			meshes[i].vertexBuffers[0] = RenderBackendCreateBuffer(renderBackend, deviceMask, &vertexBuffer0Desc, "VertexPosition");
			RenderBackendWriteBuffer(renderBackend, meshes[i].vertexBuffers[0], 0, meshes[i].positions.data(), meshes[i].numVertices * sizeof(Vector3));

			RenderBackendBufferDesc vertexBuffer1Desc = RenderBackendBufferDesc::CreateByteAddress(meshes[i].numVertices * sizeof(Vector3));
			meshes[i].vertexBuffers[1] = RenderBackendCreateBuffer(renderBackend, deviceMask, &vertexBuffer1Desc, "VertexNormal");
			RenderBackendWriteBuffer(renderBackend, meshes[i].vertexBuffers[1], 0, meshes[i].normals.data(), meshes[i].numVertices * sizeof(Vector3));

			RenderBackendBufferDesc vertexBuffer2Desc = RenderBackendBufferDesc::CreateByteAddress(meshes[i].numVertices * sizeof(Vector4));
			meshes[i].vertexBuffers[2] = RenderBackendCreateBuffer(renderBackend, deviceMask, &vertexBuffer2Desc, "VertexTangent");
			RenderBackendWriteBuffer(renderBackend, meshes[i].vertexBuffers[2], 0, meshes[i].tangents.data(), meshes[i].numVertices * sizeof(Vector4));

			RenderBackendBufferDesc vertexBuffer3Desc = RenderBackendBufferDesc::CreateByteAddress(meshes[i].numVertices * sizeof(Vector2));
			meshes[i].vertexBuffers[3] = RenderBackendCreateBuffer(renderBackend, deviceMask, &vertexBuffer3Desc, "VertexTexcoord");
			RenderBackendWriteBuffer(renderBackend, meshes[i].vertexBuffers[3], 0, meshes[i].texCoords.data(), meshes[i].numVertices * sizeof(Vector2));

			RenderBackendBufferDesc indexBufferDesc = RenderBackendBufferDesc::CreateByteAddress(meshes[i].numIndices * sizeof(uint32));
			meshes[i].indexBuffer = RenderBackendCreateBuffer(renderBackend, deviceMask, &indexBufferDesc, "IndexBuffer");
			RenderBackendWriteBuffer(renderBackend, meshes[i].indexBuffer, 0, meshes[i].indices.data(), meshes[i].numIndices * sizeof(uint32));
		}

		textures.resize(numTextures);
		for (uint32 i = 0; i < numTextures; i++)
		{
			textures[i] = LoadTextureFromFile(renderBackend, texturePaths[i].c_str());
		}

		RenderBackendBufferDesc materialBufferDesc = RenderBackendBufferDesc::CreateByteAddress(materials.size() * sizeof(MaterialInstanceData));
		materialBuffer = RenderBackendCreateBuffer(renderBackend, deviceMask, &materialBufferDesc, "MaterialBuffer");

		std::vector<MaterialInstanceData> materialInstances(numMaterials);
		for (uint32 i = 0; i < (uint32)materials.size(); i++)
		{
			Material& material = materials[i];

			materialInstances[i].baseColor = material.baseColor;
			materialInstances[i].metallic = material.metallic;
			materialInstances[i].roughness = material.roughness;

			if (material.baseColorMapIndex < 0)
			{
				materialInstances[i].baseColorMapIndex = 0;
			}
			else
			{
				materialInstances[i].baseColorMapIndex = RenderBackendGetTextureSRVDescriptorIndex(renderBackend, deviceMask, textures[material.baseColorMapIndex]);
			}
			if (material.normalMapIndex < 0)
			{
				materialInstances[i].normalMapIndex = 0;
			}
			else
			{
				materialInstances[i].normalMapIndex = RenderBackendGetTextureSRVDescriptorIndex(renderBackend, deviceMask, textures[material.normalMapIndex]);
			}
			if (material.metallicRoughnessMapIndex < 0)
			{
				materialInstances[i].metallicRoughnessMapIndex = 0;
			}
			else
			{
				materialInstances[i].metallicRoughnessMapIndex = RenderBackendGetTextureSRVDescriptorIndex(renderBackend, deviceMask, textures[material.metallicRoughnessMapIndex]);
			}
			//RenderBackendTextureSRVHandle baseColorMapSRV = CreateTextureSRV(renderBackend, deviceMask, &RenderBackendTextureSRVDesc::Create(textures[material.baseColorMapIndex]), "BaseColorMapSRV");
			//RenderBackendTextureSRVHandle normalMapSRV = CreateTextureSRV(renderBackend, deviceMask, &RenderBackendTextureSRVDesc::Create(textures[material.normalMapIndex]), "NormalMapSRV");
			//RenderBackendTextureSRVHandle metallicRoughnessMapSRV = CreateTextureSRV(renderBackend, deviceMask, &RenderBackendTextureSRVDesc::Create(textures[material.metallicRoughnessMapIndex]), "metallicRoughnessMapSRV");
		}
		RenderBackendWriteBuffer(renderBackend, materialBuffer, 0, materialInstances.data(), numMaterials * sizeof(MaterialInstanceData));

		RenderBackendBufferDesc worldMatrixBufferDesc = RenderBackendBufferDesc::CreateByteAddress(numMeshes * sizeof(Matrix4x4));
		worldMatrixBuffer = RenderBackendCreateBuffer(renderBackend, deviceMask, &worldMatrixBufferDesc, "WorldMatrixBuffer");
		
		worldMatrices.resize(numMeshes);
		for (uint32 i = 0; i < numMeshes; i++)
		{
			worldMatrices[i] = HE::Math::Compose(HE::Vector3(0.0f, 0.0f, 0.0f), HE::Quaternion(HE::Math::DegreesToRadians(HE::Vector3(180.0f, 0.0f, 0.0f))), HE::Vector3(1.0f, 1.0f, 1.0f));
		}
		RenderBackendWriteBuffer(renderBackend, worldMatrixBuffer, 0, worldMatrices.data(), numMeshes * sizeof(Matrix4x4));
	}

	void Scene::Update()
	{

	}
}