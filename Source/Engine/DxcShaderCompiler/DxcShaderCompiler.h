#pragma once

#include "Core/Core.h"
#include "RenderEngine/RenderEngine.h"

namespace HE
{

extern ShaderCompiler* CreateDxcShaderCompiler();
extern void DestroyDxcShaderCompiler(ShaderCompiler* compiler);

}