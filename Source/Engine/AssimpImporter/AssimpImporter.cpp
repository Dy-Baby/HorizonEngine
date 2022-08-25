#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/Exceptional.h>

#include "AssimpImporter.h"
#include "Render/Render.h"
#include "Core/Logging/LoggingDefines.h"

namespace HE
{

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
	HE_LOG_INFO("Import scene: {0}", filename);

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
	const aiScene* aiScene = importer->ReadFile(filename, meshImportFlags);

	if (!aiScene || !aiScene->HasMeshes())
	{
		HE_LOG_INFO("Failed to load mesh file: {0}", filename);
		return false;
	}
	
	std::map<std::string, int32> importedTextures;

	if (aiScene->HasMaterials())
	{
		for (uint32 i = 0; i < aiScene->mNumMaterials; i++)
		{
			const aiMaterial* aiMaterial = aiScene->mMaterials[i];
			/*const aiString materialName = aiMaterial->GetName();
			float roughness = 0.5f;
			float metallic = 0.5f;
			Vector4 baseColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
			aiReturn res = aiMaterial->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness);
			res = aiMaterial->Get(AI_MATKEY_METALLIC_FACTOR, metallic);
			res = aiMaterial->Get(AI_MATKEY_BASE_COLOR, baseColor);*/

			Material material;

			aiString aiTexPath;
			std::string dir = std::string(filename).substr(0, std::string(filename).find_last_of('/'));

			aiReturn result = aiMaterial->GetTexture(aiTextureType_BASE_COLOR, 0, &aiTexPath);
			if (result == aiReturn_SUCCESS)
			{
				material.baseColorMapIndex = scene->numTextures;
				material.baseColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

				scene->texturePaths.push_back(dir + '/' + aiTexPath.C_Str());
				scene->numTextures++;
			}
			else
			{
				material.baseColorMapIndex = -1;
				material.baseColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
			}

			result = aiMaterial->GetTexture(aiTextureType_NORMALS, 0, &aiTexPath);
			if (result == aiReturn_SUCCESS)
			{
				material.normalMapIndex = scene->numTextures;

				scene->texturePaths.push_back(dir + '/' + aiTexPath.C_Str());
				scene->numTextures++;
			}
			else
			{
				material.normalMapIndex = -1;
			}

			result = aiMaterial->GetTexture(aiTextureType_UNKNOWN, 0, &aiTexPath);
			if (result == aiReturn_SUCCESS)
			{
				material.metallicRoughnessMapIndex = scene->numTextures;
				material.metallic = 1.0f;
				material.roughness = 1.0f;

				scene->texturePaths.push_back(dir + '/' + aiTexPath.C_Str());
				scene->numTextures++;
			}
			else
			{
				material.metallicRoughnessMapIndex = -1;
				material.metallic = 0.0f;
				material.roughness = 1.0f;
			}
			scene->materials.emplace_back(material);
		}
		scene->numMaterials += aiScene->mNumMaterials;
	}

	for (uint32 i = 0; i < aiScene->mNumMeshes; i++)
	{
		const aiMesh* aiMesh = aiScene->mMeshes[i];
		
		if (!aiMesh->HasPositions() || !aiMesh->HasNormals() || !aiMesh->HasTangentsAndBitangents())
		{
			HE_LOG_ERROR("Failed to import mesh {}.", i);
			return false;
		}

		Mesh mesh;

		mesh.materialID = scene->numMaterials - aiScene->mNumMaterials + aiMesh->mMaterialIndex;

		mesh.numVertices = aiMesh->mNumVertices;
		mesh.numIndices = aiMesh->mNumFaces * 3;
		
		mesh.positions.resize(aiMesh->mNumVertices);
		mesh.normals.resize(aiMesh->mNumVertices);
		mesh.tangents.resize(aiMesh->mNumVertices);
		mesh.texCoords.resize(aiMesh->mNumVertices);

		for(uint32 vertexID = 0; vertexID < aiMesh->mNumVertices; vertexID++)
		{
			mesh.positions[vertexID] = Vector3(aiMesh->mVertices[vertexID].x, aiMesh->mVertices[vertexID].y, aiMesh->mVertices[vertexID].z);
			mesh.normals[vertexID] = Vector3(aiMesh->mNormals[vertexID].x, aiMesh->mNormals[vertexID].y, aiMesh->mNormals[vertexID].z);
			mesh.tangents[vertexID] = Vector4(aiMesh->mTangents[vertexID].x, aiMesh->mTangents[vertexID].y, aiMesh->mTangents[vertexID].z, 1.0f);
			if (aiMesh->HasTextureCoords(0))
			{
				mesh.texCoords[vertexID] = Vector2(aiMesh->mTextureCoords[0][vertexID].x, aiMesh->mTextureCoords[0][vertexID].y);
			}
			else
			{
				mesh.texCoords[vertexID] = Vector2(0.0f);
			}
		}

		for (uint32 faceIndex = 0; faceIndex < aiMesh->mNumFaces; faceIndex++)
		{
			ASSERT(aiMesh->mFaces[faceIndex].mNumIndices == 3);
			mesh.indices.push_back(aiMesh->mFaces[faceIndex].mIndices[0]);
			mesh.indices.push_back(aiMesh->mFaces[faceIndex].mIndices[1]);
			mesh.indices.push_back(aiMesh->mFaces[faceIndex].mIndices[2]);
		}

		scene->meshes.emplace_back(mesh);
	}
	scene->numMeshes += aiScene->mNumMeshes;

	return true;
}
}