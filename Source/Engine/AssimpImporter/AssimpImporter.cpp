#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/Exceptional.h>

#include <map>

#include "AssimpImporter.h"

namespace HE
{
	Matrix4x4 ConvertToMatrix4x4(const aiMatrix4x4& matrix)
	{
		Matrix4x4 result;
		result[0][0] = matrix.a1; result[1][0] = matrix.a2; result[2][0] = matrix.a3; result[3][0] = matrix.a4;
		result[0][1] = matrix.b1; result[1][1] = matrix.b2; result[2][1] = matrix.b3; result[3][1] = matrix.b4;
		result[0][2] = matrix.c1; result[1][2] = matrix.c2; result[2][2] = matrix.c3; result[3][2] = matrix.c4;
		result[0][3] = matrix.d1; result[1][3] = matrix.d2; result[2][3] = matrix.d3; result[3][3] = matrix.d4;
		return result;
	}

	struct ImportAssimpTaskData
	{
		const char* filename;
		Mesh* mesh;
	};

	static void ImportAssimpNode(Mesh* mesh, const struct aiNode* aiNode, const Matrix4x4& parentTransform)
	{
		Matrix4x4 transform = parentTransform * ConvertToMatrix4x4(aiNode->mTransformation);
		for (uint32 i = 0; i < aiNode->mNumMeshes; i++)
		{
			int meshIndex = aiNode->mMeshes[i];
			auto& element = mesh->elements[meshIndex];
			element.transform = transform;
		}

		for (uint32 i = 0; i < aiNode->mNumChildren; i++)
		{
			ImportAssimpNode(mesh, aiNode->mChildren[i], transform);
		}
	}

	static void ImportAssimpTask(void* data)
	{
		ImportAssimpTaskData* taskData = (ImportAssimpTaskData*)data;

		HE_LOG_INFO("Import scene: {0}", taskData->filename);

		static const uint32_t meshImportFlags =
			aiProcess_CalcTangentSpace |        // Create binormals/tangents just in case
			aiProcess_Triangulate |             // Make sure we're triangles
			aiProcess_SortByPType |             // Split meshes by primitive type
			aiProcess_GenNormals |              // Make sure we have legit normals
			aiProcess_GenUVCoords |             // Convert UVs if required 
			aiProcess_OptimizeMeshes |          // Batch draws where possible
			aiProcess_JoinIdenticalVertices |
			aiProcess_ValidateDataStructure;    // Validation

		std::unique_ptr<Assimp::Importer> importer = std::make_unique<Assimp::Importer>();
		const aiScene* aiScene = importer->ReadFile(taskData->filename, meshImportFlags);

		std::string dir = std::string(taskData->filename).substr(0, std::string(taskData->filename).find_last_of('/'));

		Mesh* mesh = taskData->mesh;

		if (!aiScene)
		{
			HE_LOG_INFO("Failed to load mesh file: {0}", taskData->filename);
		}

		if (aiScene->HasMaterials())
		{
			for (uint32 i = 0; i < aiScene->mNumMaterials; i++)
			{
				const aiMaterial* aiMaterial = aiScene->mMaterials[i];

				Material& material = mesh->materials.emplace_back();
				material.name = aiMaterial->GetName().C_Str();

				Vector4 baseColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
				aiMaterial->Get(AI_MATKEY_BASE_COLOR, baseColor);
				material.baseColor = baseColor;

				Vector4 emission = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
				aiMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, emission);
				material.emission = emission;

				float metallic = 1.0f;
				aiMaterial->Get(AI_MATKEY_METALLIC_FACTOR, metallic);
				material.metallic = metallic;

				float roughness = 1.0f;
				aiMaterial->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness);
				material.roughness = roughness;

				aiString aiTexPath;

				aiReturn result = aiMaterial->GetTexture(aiTextureType_BASE_COLOR, 0, &aiTexPath);
				if (result == aiReturn_SUCCESS)
				{
					material.baseColorMap = dir + '/' + aiTexPath.C_Str();
					// material.baseColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
				}
				else
				{
					material.baseColorMap = "";
				}

				result = aiMaterial->GetTexture(aiTextureType_NORMALS, 0, &aiTexPath);
				if (result == aiReturn_SUCCESS)
				{
					material.normalMap = dir + '/' + aiTexPath.C_Str();
				}
				else
				{
					material.normalMap = "";
				}

				result = aiMaterial->GetTexture(aiTextureType_UNKNOWN, 0, &aiTexPath);
				if (result == aiReturn_SUCCESS)
				{
					material.metallicRoughnessMap = dir + '/' + aiTexPath.C_Str();
					material.metallic = 1.0f;
					material.roughness = 1.0f;
				}
				else
				{
					material.metallicRoughnessMap = "";
					material.metallic = 0.0f;
					material.roughness = 1.0f;
				}

				result = aiMaterial->GetTexture(aiTextureType_EMISSIVE, 0, &aiTexPath);
				if (result == aiReturn_SUCCESS)
				{
					material.emissiveMap = dir + '/' + aiTexPath.C_Str();
				}
				else
				{
					material.emissiveMap = "";
				}
			}
		}

		mesh->numVertices = 0;
		mesh->numIndices = 0;

		if (aiScene->HasMeshes())
		{
			for (uint32 i = 0; i < aiScene->mNumMeshes; i++)
			{
				const aiMesh* aiMesh = aiScene->mMeshes[i];

				if (!aiMesh->HasPositions() || !aiMesh->HasNormals() || !aiMesh->HasTangentsAndBitangents())
				{
					HE_LOG_ERROR("Failed to import mesh {}.", i);
					return;
				}

				MeshElement& element = mesh->elements.emplace_back();
				element.name = aiMesh->mName.C_Str();
				element.baseVertex = mesh->numVertices;
				element.baseIndex = mesh->numIndices;
				element.materialIndex = aiMesh->mMaterialIndex;
				element.numVertices = aiMesh->mNumVertices;
				element.numIndices = aiMesh->mNumFaces * 3;
				element.transform = Matrix4x4(1.0f);

				mesh->numVertices += element.numVertices;
				mesh->numIndices += element.numIndices;

				for (uint32 vertexID = 0; vertexID < aiMesh->mNumVertices; vertexID++)
				{
					mesh->positions.emplace_back(Vector3(aiMesh->mVertices[vertexID].x, aiMesh->mVertices[vertexID].y, aiMesh->mVertices[vertexID].z));
					mesh->normals.emplace_back(Vector3(aiMesh->mNormals[vertexID].x, aiMesh->mNormals[vertexID].y, aiMesh->mNormals[vertexID].z));
					mesh->tangents.emplace_back(Vector4(aiMesh->mTangents[vertexID].x, aiMesh->mTangents[vertexID].y, aiMesh->mTangents[vertexID].z, 1.0f));
					if (aiMesh->HasTextureCoords(0))
					{
						mesh->texCoords.emplace_back(Vector2(aiMesh->mTextureCoords[0][vertexID].x, aiMesh->mTextureCoords[0][vertexID].y));
					}
					else
					{
						mesh->texCoords.emplace_back(Vector2(0.0f));
					}
				}

				for (uint32 faceIndex = 0; faceIndex < aiMesh->mNumFaces; faceIndex++)
				{
					ASSERT(aiMesh->mFaces[faceIndex].mNumIndices == 3);
					mesh->indices.push_back(aiMesh->mFaces[faceIndex].mIndices[0]);
					mesh->indices.push_back(aiMesh->mFaces[faceIndex].mIndices[1]);
					mesh->indices.push_back(aiMesh->mFaces[faceIndex].mIndices[2]);
				}
			}
		}

		ImportAssimpNode(mesh, aiScene->mRootNode, Matrix4x4(1.0f));
	}

	void AssimpImporter::ImportAsset(const char* filename)
	{
		Mesh* mesh = new Mesh();
		ImportAssimpTaskData taskData = {
			.filename = filename,
			.mesh = mesh,
		};
		ImportAssimpTask(&taskData);
		AssetManager::AddAsset(filename, mesh);
	}

	bool ImportOBJ(const char* filename, OBJImportSettings settings, RenderScene* scene)
	{
		return false;
	}

	bool ImportFBX(const char* filename, FBXImportSettings settings, RenderScene* scene)
	{
		return false;
	}

	bool ImportGLTF2(const char* filename, GLTF2ImportSettings settings, RenderScene* scene)
	{
		//HE_LOG_INFO("Import scene: {0}", filename);

		//static const uint32_t meshImportFlags =
		//	aiProcess_CalcTangentSpace |        // Create binormals/tangents just in case
		//	aiProcess_Triangulate |             // Make sure we're triangles
		//	aiProcess_SortByPType |             // Split meshes by primitive type
		//	aiProcess_GenNormals |              // Make sure we have legit normals
		//	aiProcess_GenUVCoords |             // Convert UVs if required 
		//	aiProcess_OptimizeMeshes |          // Batch draws where possible
		//	aiProcess_JoinIdenticalVertices |
		//	aiProcess_ValidateDataStructure;    // Validation

		//std::unique_ptr<Assimp::Importer> importer = std::make_unique<Assimp::Importer>();
		//const aiScene* aiScene = importer->ReadFile(filename, meshImportFlags);

		//if (!aiScene || !aiScene->HasMeshes())
		//{
		//	HE_LOG_INFO("Failed to load mesh file: {0}", filename);
		//	return false;
		//}

		//std::map<std::string, int32> importedTextures;

		//if (aiScene->HasMaterials())
		//{
		//	for (uint32 i = 0; i < aiScene->mNumMaterials; i++)
		//	{
		//		const aiMaterial* aiMaterial = aiScene->mMaterials[i];
		//		/*const aiString materialName = aiMaterial->GetName();
		//		float roughness = 0.5f;
		//		float metallic = 0.5f;
		//		Vector4 baseColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		//		aiReturn res = aiMaterial->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness);
		//		res = aiMaterial->Get(AI_MATKEY_METALLIC_FACTOR, metallic);
		//		res = aiMaterial->Get(AI_MATKEY_BASE_COLOR, baseColor);*/

		//		Material material;

		//		aiString aiTexPath;
		//		std::string dir = std::string(filename).substr(0, std::string(filename).find_last_of('/'));

		//		aiReturn result = aiMaterial->GetTexture(aiTextureType_BASE_COLOR, 0, &aiTexPath);
		//		if (result == aiReturn_SUCCESS)
		//		{
		//			material.baseColorMapIndex = scene->numTextures;
		//			material.baseColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

		//			scene->texturePaths.push_back(dir + '/' + aiTexPath.C_Str());
		//			scene->numTextures++;
		//		}
		//		else
		//		{
		//			material.baseColorMapIndex = -1;
		//			material.baseColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		//		}

		//		result = aiMaterial->GetTexture(aiTextureType_NORMALS, 0, &aiTexPath);
		//		if (result == aiReturn_SUCCESS)
		//		{
		//			material.normalMapIndex = scene->numTextures;

		//			scene->texturePaths.push_back(dir + '/' + aiTexPath.C_Str());
		//			scene->numTextures++;
		//		}
		//		else
		//		{
		//			material.normalMapIndex = -1;
		//		}

		//		result = aiMaterial->GetTexture(aiTextureType_UNKNOWN, 0, &aiTexPath);
		//		if (result == aiReturn_SUCCESS)
		//		{
		//			material.metallicRoughnessMapIndex = scene->numTextures;
		//			material.metallic = 1.0f;
		//			material.roughness = 1.0f;

		//			scene->texturePaths.push_back(dir + '/' + aiTexPath.C_Str());
		//			scene->numTextures++;
		//		}
		//		else
		//		{
		//			material.metallicRoughnessMapIndex = -1;
		//			material.metallic = 0.0f;
		//			material.roughness = 1.0f;
		//		}
		//		scene->materials.emplace_back(material);
		//	}
		//	scene->numMaterials += aiScene->mNumMaterials;
		//}

		//for (uint32 i = 0; i < aiScene->mNumMeshes; i++)
		//{
		//	const aiMesh* aiMesh = aiScene->mMeshes[i];

		//	if (!aiMesh->HasPositions() || !aiMesh->HasNormals() || !aiMesh->HasTangentsAndBitangents())
		//	{
		//		HE_LOG_ERROR("Failed to import mesh {}.", i);
		//		return false;
		//	}

		//	Mesh mesh;

		//	mesh.materialID = scene->numMaterials - aiScene->mNumMaterials + aiMesh->mMaterialIndex;

		//	mesh.numVertices = aiMesh->mNumVertices;
		//	mesh.numIndices = aiMesh->mNumFaces * 3;

		//	mesh.positions.resize(aiMesh->mNumVertices);
		//	mesh.normals.resize(aiMesh->mNumVertices);
		//	mesh.tangents.resize(aiMesh->mNumVertices);
		//	mesh.texCoords.resize(aiMesh->mNumVertices);

		//	for (uint32 vertexID = 0; vertexID < aiMesh->mNumVertices; vertexID++)
		//	{
		//		mesh.positions[vertexID] = Vector3(aiMesh->mVertices[vertexID].x, aiMesh->mVertices[vertexID].y, aiMesh->mVertices[vertexID].z);
		//		mesh.normals[vertexID] = Vector3(aiMesh->mNormals[vertexID].x, aiMesh->mNormals[vertexID].y, aiMesh->mNormals[vertexID].z);
		//		mesh.tangents[vertexID] = Vector4(aiMesh->mTangents[vertexID].x, aiMesh->mTangents[vertexID].y, aiMesh->mTangents[vertexID].z, 1.0f);
		//		if (aiMesh->HasTextureCoords(0))
		//		{
		//			mesh.texCoords[vertexID] = Vector2(aiMesh->mTextureCoords[0][vertexID].x, aiMesh->mTextureCoords[0][vertexID].y);
		//		}
		//		else
		//		{
		//			mesh.texCoords[vertexID] = Vector2(0.0f);
		//		}
		//	}

		//	for (uint32 faceIndex = 0; faceIndex < aiMesh->mNumFaces; faceIndex++)
		//	{
		//		ASSERT(aiMesh->mFaces[faceIndex].mNumIndices == 3);
		//		mesh.indices.push_back(aiMesh->mFaces[faceIndex].mIndices[0]);
		//		mesh.indices.push_back(aiMesh->mFaces[faceIndex].mIndices[1]);
		//		mesh.indices.push_back(aiMesh->mFaces[faceIndex].mIndices[2]);
		//	}

		//	scene->meshes.emplace_back(mesh);
		//}
		//scene->numMeshes += aiScene->mNumMeshes;

		return true;
	}
}