#include "PixelFormat.h"

namespace HE
{

const PixelFormatDesc gPixelFormatTable[] =
{
    // format                         name                 type                           bytes  channels  { depth, stencil }   channelBits
    { PixelFormat::Unknown,           "Unknown",           PixelFormatType::Unknown,      0,     0,        { false, false },    { 0,  0,  0,  0  } },
                                                                                                           
    { PixelFormat::R8Unorm,           "R8Unorm",           PixelFormatType::Unorm,        1,     1,        { false, false },    { 8,  0,  0,  0  } },
    { PixelFormat::R8Snorm,           "R8Snorm",           PixelFormatType::Snorm,        1,     1,        { false, false },    { 8,  0,  0,  0  } },
    { PixelFormat::R16Unorm,          "R16Unorm",          PixelFormatType::Unorm,        2,     1,        { false, false },    { 16, 0,  0,  0  } },
    { PixelFormat::R16Snorm,          "R16Snorm",          PixelFormatType::Snorm,        2,     1,        { false, false },    { 16, 0,  0,  0  } },
    { PixelFormat::RG8Unorm,          "RG8Unorm",          PixelFormatType::Unorm,        2,     2,        { false, false },    { 8,  8,  0,  0  } },
    { PixelFormat::RG8Snorm,          "RG8Snorm",          PixelFormatType::Snorm,        2,     2,        { false, false },    { 8,  8,  0,  0  } },
    { PixelFormat::RG16Unorm,         "RG16Unorm",         PixelFormatType::Unorm,        4,     2,        { false, false },    { 16, 16, 0,  0  } },
    { PixelFormat::RG16Snorm,         "RG16Snorm",         PixelFormatType::Snorm,        4,     2,        { false, false },    { 16, 16, 0,  0  } },
    { PixelFormat::RGB16Unorm,        "RGB16Unorm",        PixelFormatType::Unorm,        6,     3,        { false, false },    { 16, 16, 16, 0  } },
    { PixelFormat::RGB16Snorm,        "RGB16Snorm",        PixelFormatType::Snorm,        6,     3,        { false, false },    { 16, 16, 16, 0  } },
    { PixelFormat::RGBA8Unorm,        "RGBA8Unorm",        PixelFormatType::Unorm,        4,     4,        { false, false },    { 8,  8,  8,  8  } },
    { PixelFormat::RGBA8Snorm,        "RGBA8Snorm",        PixelFormatType::Snorm,        4,     4,        { false, false },    { 8,  8,  8,  8  } },
    { PixelFormat::RGBA16Unorm,       "RGBA16Unorm",       PixelFormatType::Unorm,        8,     4,        { false, false },    { 16, 16, 16, 16 } },
                                                                                                         
    { PixelFormat::RGBA8UnormSrgb,    "RGBA8UnormSrgb",    PixelFormatType::UnormSrgb,    4,     4,        { false, false },    { 8,  8,  8,  8  } },
                                                                                                           
    { PixelFormat::R16Float,          "R16Float",          PixelFormatType::Float,        2,     1,        { false, false },    { 16, 0,  0,  0  } },
    { PixelFormat::RG16Float,         "RG16Float",         PixelFormatType::Float,        4,     2,        { false, false },    { 16, 16, 0,  0  } },
    { PixelFormat::RGB16Float,        "RGB16Float",        PixelFormatType::Float,        6,     3,        { false, false },    { 16, 16, 16, 0  } },
    { PixelFormat::RGBA16Float,       "RGBA16Float",       PixelFormatType::Float,        8,     4,        { false, false },    { 16, 16, 16, 16 } },
    { PixelFormat::R32Float,          "R32Float",          PixelFormatType::Float,        4,     1,        { false, false },    { 32, 0,  0,  0  } },
    { PixelFormat::RG32Float,         "RG32Float",         PixelFormatType::Float,        8,     2,        { false, false },    { 32, 32, 0,  0  } },
    { PixelFormat::RGB32Float,        "RGB32Float",        PixelFormatType::Float,        12,    3,        { false, false },    { 32, 32, 32, 0  } },
    { PixelFormat::RGBA32Float,       "RGBA32Float",       PixelFormatType::Float,        16,    4,        { false, false },    { 32, 32, 32, 32 } },
                                                                                                            
    { PixelFormat::R8Int,             "R8Int",             PixelFormatType::Sint,         1,     1,        { false, false },    { 8,  0,  0,  0  } },
    { PixelFormat::R8Uint,            "R8Uint",            PixelFormatType::Uint,         1,     1,        { false, false },    { 8,  0,  0,  0  } },
    { PixelFormat::R16Int,            "R16Int",            PixelFormatType::Sint,         2,     1,        { false, false },    { 16, 0,  0,  0  } },
    { PixelFormat::R16Uint,           "R16Uint",           PixelFormatType::Uint,         2,     1,        { false, false },    { 16, 0,  0,  0  } },
    { PixelFormat::R32Int,            "R32Int",            PixelFormatType::Sint,         4,     1,        { false, false },    { 32, 0,  0,  0  } },
    { PixelFormat::R32Uint,           "R32Uint",           PixelFormatType::Uint,         4,     1,        { false, false },    { 32, 0,  0,  0  } },
    { PixelFormat::RG8Int,            "RG8Int",            PixelFormatType::Sint,         2,     2,        { false, false },    { 8,  8,  0,  0  } },
    { PixelFormat::RG8Uint,           "RG8Uint",           PixelFormatType::Uint,         2,     2,        { false, false },    { 8,  8,  0,  0  } },
    { PixelFormat::RG16Int,           "RG16Int",           PixelFormatType::Sint,         4,     2,        { false, false },    { 16, 16, 0,  0  } },
    { PixelFormat::RG16Uint,          "RG16Uint",          PixelFormatType::Uint,         4,     2,        { false, false },    { 16, 16, 0,  0  } },
    { PixelFormat::RG32Int,           "RG32Int",           PixelFormatType::Sint,         8,     2,        { false, false },    { 32, 32, 0,  0  } },
    { PixelFormat::RG32Uint,          "RG32Uint",          PixelFormatType::Uint,         8,     2,        { false, false },    { 32, 32, 0,  0  } },
    { PixelFormat::RGB16Int,          "RGB16Int",          PixelFormatType::Sint,         6,     3,        { false, false },    { 16, 16, 16, 0  } },
    { PixelFormat::RGB16Uint,         "RGB16Uint",         PixelFormatType::Uint,         6,     3,        { false, false },    { 16, 16, 16, 0  } },
    { PixelFormat::RGB32Int,          "RGB32Int",          PixelFormatType::Sint,         12,    3,        { false, false },    { 32, 32, 32, 0  } },
    { PixelFormat::RGB32Uint,         "RGB32Uint",         PixelFormatType::Uint,         12,    3,        { false, false },    { 32, 32, 32, 0  } },
    { PixelFormat::RGBA8Int,          "RGBA8Int",          PixelFormatType::Sint,         4,     4,        { false, false },    { 8,  8,  8,  8  } },
    { PixelFormat::RGBA8Uint,         "RGBA8Uint",         PixelFormatType::Uint,         4,     4,        { false, false },    { 8,  8,  8,  8  } },
    { PixelFormat::RGBA16Int,         "RGBA16Int",         PixelFormatType::Sint,         8,     4,        { false, false },    { 16, 16, 16, 16 } },
    { PixelFormat::RGBA16Uint,        "RGBA16Uint",        PixelFormatType::Uint,         8,     4,        { false, false },    { 16, 16, 16, 16 } },
    { PixelFormat::RGBA32Int,         "RGBA32Int",         PixelFormatType::Sint,         16,    4,        { false, false },    { 32, 32, 32, 32 } },
    { PixelFormat::RGBA32Uint,        "RGBA32Uint",        PixelFormatType::Uint,         16,    4,        { false, false },    { 32, 32, 32, 32 } },
                                                                                                              
    { PixelFormat::BGRA8Unorm,        "BGRA8Unorm",        PixelFormatType::Unorm,        4,     4,        { false, false },    { 8,  8,  8,  8  } },
    { PixelFormat::BGRA8UnormSrgb,    "BGRA8UnormSrgb",    PixelFormatType::UnormSrgb,    4,     4,        { false, false },    { 8,  8,  8,  8  } },
                                                                                                              
    { PixelFormat::D32Float,          "D32Float",          PixelFormatType::Float,        4,     1,        { true,  false },    { 32, 0,  0,  0  } },
    { PixelFormat::D16Unorm,          "D16Unorm",          PixelFormatType::Unorm,        2,     1,        { true,  false },    { 16, 0,  0,  0  } },
    { PixelFormat::D24UnormS8Uint,    "D24UnormS8Uint",    PixelFormatType::Unorm,        4,     2,        { true,  true  },    { 24, 8,  0,  0  } },

    { PixelFormat::A2BGR10Unorm,      "A2BGR10Unorm",      PixelFormatType::Unorm,        4,     4,        { false, false },    { 2,  10, 10, 10 } },
};
static_assert(ARRAY_SIZE(gPixelFormatTable) == (uint32)PixelFormat::Count);

}