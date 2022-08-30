module;

#include <ECS/ECS.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

module HorizonEngine.Render.Scene;

namespace HE
{
	RenderScene::RenderScene()
	{
	}

	RenderScene::~RenderScene()
	{
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

	void RenderScene::UploadResources(Scene* scene)
	{
		uint32 deviceMask = ~0u;

		this->scene = scene;
		
		auto group = scene->GetEntityManager()->Get()->group<StaticMeshComponent>(entt::get<TransformComponent>);
		for (auto entity : group)
		{
			auto [transformComponent, staticMeshComponent] = group.get<TransformComponent, StaticMeshComponent>(entity);
			Mesh* meshSource = AssetManager::GetAsset<Mesh>(staticMeshComponent.meshSource);
			if (meshSource)
			{
				RenderBackendBufferDesc vertexBuffer0Desc = RenderBackendBufferDesc::CreateByteAddress(meshSource->numVertices * sizeof(Vector3));
				RenderBackendBufferHandle vertexBuffer0 = RenderBackendCreateBuffer(renderBackend, deviceMask, &vertexBuffer0Desc, "VertexPosition");
				RenderBackendWriteBuffer(renderBackend, vertexBuffer0, 0, meshSource->positions.data(), meshSource->numVertices * sizeof(Vector3));

				RenderBackendBufferDesc vertexBuffer1Desc = RenderBackendBufferDesc::CreateByteAddress(meshSource->numVertices * sizeof(Vector3));
				RenderBackendBufferHandle vertexBuffer1 = RenderBackendCreateBuffer(renderBackend, deviceMask, &vertexBuffer1Desc, "VertexNormal");
				RenderBackendWriteBuffer(renderBackend, vertexBuffer1, 0, meshSource->normals.data(), meshSource->numVertices * sizeof(Vector3));

				RenderBackendBufferDesc vertexBuffer2Desc = RenderBackendBufferDesc::CreateByteAddress(meshSource->numVertices * sizeof(Vector4));
				RenderBackendBufferHandle vertexBuffer2 = RenderBackendCreateBuffer(renderBackend, deviceMask, &vertexBuffer2Desc, "VertexTangent");
				RenderBackendWriteBuffer(renderBackend, vertexBuffer2, 0, meshSource->tangents.data(), meshSource->numVertices * sizeof(Vector4));

				RenderBackendBufferDesc vertexBuffer3Desc = RenderBackendBufferDesc::CreateByteAddress(meshSource->numVertices * sizeof(Vector2));
				RenderBackendBufferHandle vertexBuffer3 = RenderBackendCreateBuffer(renderBackend, deviceMask, &vertexBuffer3Desc, "VertexTexcoord");
				RenderBackendWriteBuffer(renderBackend, vertexBuffer3, 0, meshSource->texCoords.data(), meshSource->numVertices * sizeof(Vector2));

				RenderBackendBufferDesc indexBufferDesc = RenderBackendBufferDesc::CreateByteAddress(meshSource->numIndices * sizeof(uint32));
				RenderBackendBufferHandle indexBuffer = RenderBackendCreateBuffer(renderBackend, deviceMask, &indexBufferDesc, "IndexBuffer");
				RenderBackendWriteBuffer(renderBackend, indexBuffer, 0, meshSource->indices.data(), meshSource->numIndices * sizeof(uint32));

				uint32 vertexBufferIndex = (uint32)vertexBuffers[0].size();
				uint32 indexBufferIndex = (uint32)indexBuffers.size();

				vertexBuffers[0].push_back(vertexBuffer0);
				vertexBuffers[1].push_back(vertexBuffer1);
				vertexBuffers[2].push_back(vertexBuffer2);
				vertexBuffers[3].push_back(vertexBuffer3);
				indexBuffers.push_back(indexBuffer);

				uint32 baseMaterialIndex = (uint32)materials.size();
				for (uint32 i = 0; i < (uint32)meshSource->materials.size(); i++)
				{
					PBRMaterialShaderParameters& material = materials.emplace_back();

					material.baseColor = meshSource->materials[i].baseColor;
					material.metallic = meshSource->materials[i].metallic;
					material.roughness = meshSource->materials[i].roughness;

					if (meshSource->materials[i].baseColorMap == "")
					{
						material.baseColorMapIndex = 0;
					}
					else
					{
						RenderBackendTextureHandle texture = LoadTextureFromFile(renderBackend, meshSource->materials[i].baseColorMap.c_str());
						textures.push_back(texture);
						material.baseColorMapIndex = RenderBackendGetTextureSRVDescriptorIndex(renderBackend, deviceMask, texture);
					}
					if (meshSource->materials[i].normalMap == "")
					{
						material.normalMapIndex = 0;
					}
					else
					{
						RenderBackendTextureHandle texture = LoadTextureFromFile(renderBackend, meshSource->materials[i].normalMap.c_str());
						textures.push_back(texture);
						material.normalMapIndex = RenderBackendGetTextureSRVDescriptorIndex(renderBackend, deviceMask, texture);
					}
					if (meshSource->materials[i].metallicRoughnessMap == "")
					{
						material.metallicRoughnessMapIndex = 0;
					}
					else
					{
						RenderBackendTextureHandle texture = LoadTextureFromFile(renderBackend, meshSource->materials[i].metallicRoughnessMap.c_str());
						textures.push_back(texture);
						material.metallicRoughnessMapIndex = RenderBackendGetTextureSRVDescriptorIndex(renderBackend, deviceMask, texture);
					}
					if (meshSource->materials[i].emissiveMap == "")
					{
						material.emissiveMapIndex = 0;
					}
					else
					{
						RenderBackendTextureHandle texture = LoadTextureFromFile(renderBackend, meshSource->materials[i].emissiveMap.c_str());
						textures.push_back(texture);
						material.emissiveMapIndex = RenderBackendGetTextureSRVDescriptorIndex(renderBackend, deviceMask, texture);
					}
				}

				for (const auto& element : meshSource->elements)
				{
					uint32 transformIndex = (uint32)worldMatrices.size();
					worldMatrices.push_back(transformComponent.world);

					auto& renderable = renderables.emplace_back();
					renderable.firstVertex = element.baseVertex;
					renderable.firstIndex = element.baseIndex;
					renderable.numIndices = element.numIndices;
					renderable.numVertices = element.numVertices;
					renderable.vertexBufferIndex = vertexBufferIndex;
					renderable.indexBufferIndex = indexBufferIndex;
					renderable.materialIndex = baseMaterialIndex + element.materialIndex;
					renderable.transformIndex = transformIndex;
				}
			}
		}

		RenderBackendBufferDesc materialBufferDesc = RenderBackendBufferDesc::CreateByteAddress(materials.size() * sizeof(PBRMaterialShaderParameters));
		materialBuffer = RenderBackendCreateBuffer(renderBackend, deviceMask, &materialBufferDesc, "MaterialBuffer");
		RenderBackendWriteBuffer(renderBackend, materialBuffer, 0, materials.data(), materials.size() * sizeof(PBRMaterialShaderParameters));

		RenderBackendBufferDesc worldMatrixBufferDesc = RenderBackendBufferDesc::CreateByteAddress(worldMatrices.size() * sizeof(Matrix4x4));
		worldMatrixBuffer = RenderBackendCreateBuffer(renderBackend, deviceMask, &worldMatrixBufferDesc, "WorldMatrixBuffer");
		RenderBackendWriteBuffer(renderBackend, worldMatrixBuffer, 0, worldMatrices.data(), worldMatrices.size() * sizeof(Matrix4x4));

#if DEBUG_ONLY_RAY_TRACING_ENBALE
		std::vector<RenderBackendGeometryDesc> geometryDescs(renderables.size());
		for (uint32 i = 0; i < (uint32)geometryDescs.size(); i++)
		{
			geometryDescs[i] = {
				.type = RenderBackendGeometryType::Triangles, 
				.flags = RenderBackendGeometryFlags::Opaque, 
				.triangleDesc = {
					.numIndices = renderables[i].numIndices,
					.numVertices = renderables[i].numVertices,
					.vertexStride = 3 * sizeof(float),
					.vertexBuffer = vertexBuffers[0][renderables[i].vertexBufferIndex],
					.vertexOffset = renderables[i].firstVertex * 3 * sizeof(float),
					.indexBuffer = indexBuffers[renderables[i].indexBufferIndex],
					.indexOffset = renderables[i].firstIndex * sizeof(uint32),
					.transformBuffer = worldMatrixBuffer,
					.transformOffset = renderables[i].transformIndex * 16 * (uint32)sizeof(float),
				}
			};
		}
		
		RenderBackendBottomLevelASDesc bottomLevelASDesc = {
			.buildFlags = RenderBackendAccelerationStructureBuildFlags::PreferFastTrace,
			.numGeometries = (uint32)geometryDescs.size(),
			.geometryDescs = geometryDescs.data(),
		};
		bottomLevelAS = RenderBackendCreateBottomLevelAS(renderBackend, deviceMask, &bottomLevelASDesc, "BottomLevelAS");

		RenderBackendRayTracingInstance geometryInstance = {
			.transformMatrix = Matrix4x4(1.0),
			.instanceID = 0,
			.instanceMask = 0xff,
			.instanceContributionToHitGroupIndex = 0,
			.flags = RenderBackendRayTracingInstanceFlags::TriangleFacingCullDisable,
			.blas = bottomLevelAS,
		};

		RenderBackendTopLevelASDesc topLevelASDesc = {
			.buildFlags = RenderBackendAccelerationStructureBuildFlags::PreferFastTrace,
			.geometryFlags = RenderBackendGeometryFlags::Opaque,
			.numInstances = 1,
			.instances = &geometryInstance,
		};
		topLevelAS = RenderBackendCreateTopLevelAS(renderBackend, deviceMask, &topLevelASDesc, "TopLevelAS");
#endif
	}
}