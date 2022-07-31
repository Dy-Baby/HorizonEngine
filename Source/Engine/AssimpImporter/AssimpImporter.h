#pragma once

#include "Core/Core.h"

namespace HE
{
    class Scene;

    struct OBJImportSettings
    {

    };

    extern bool ImportOBJ(const char* filename, OBJImportSettings settings, Scene* scene);

    struct FBXImportSettings
    {

    };

    extern bool ImportFBX(const char* filename, FBXImportSettings settings, Scene* scene);
    
    struct GLTF2ImportSettings
    {

    };

    extern bool ImportGLTF2(const char* filename, GLTF2ImportSettings settings, Scene* scene);
}
