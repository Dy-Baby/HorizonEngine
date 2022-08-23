#pragma once

#include "Core/Core.h"
#include "RenderEngine/RenderEngineCommon.h"

namespace HE
{
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
}
