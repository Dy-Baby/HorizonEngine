#pragma once

#include "Core/Core.h"

namespace HE
{

/** Pixel formats. */
enum class PixelFormat : uint32
{
    Unknown,
    // Norm
    R8Unorm,
    R8Snorm,
    R16Unorm,
    R16Snorm,
    RG8Unorm,
    RG8Snorm,
    RG16Unorm,
    RG16Snorm,
    RGB16Unorm,
    RGB16Snorm,
    RGBA8Unorm,
    RGBA8Snorm,
    RGBA16Unorm,
    // UnormSrgb
    RGBA8UnormSrgb,
    // Float
    R16Float,
    RG16Float,
    RGB16Float,
    RGBA16Float,
    R32Float,
    RG32Float,
    RGB32Float,
    RGBA32Float,
    // Int
    R8Int,
    R8Uint,
    R16Int,
    R16Uint,
    R32Int,
    R32Uint,
    RG8Int,
    RG8Uint,
    RG16Int,
    RG16Uint,
    RG32Int,
    RG32Uint,
    RGB16Int,
    RGB16Uint,
    RGB32Int,
    RGB32Uint,
    RGBA8Int,
    RGBA8Uint,
    RGBA16Int,
    RGBA16Uint,
    RGBA32Int,
    RGBA32Uint,
    // BGRA
    BGRA8Unorm,
    BGRA8UnormSrgb,
    // Depth stencil
    D32Float,
    D16Unorm,
    D24UnormS8Uint,

    A2BGR10Unorm,
    // Count
    Count
};

/** Pixel format type. */
enum class PixelFormatType
{
    Unknown,        ///< Unknown
    Float,          ///< Floating-point
    Sint,           ///< Signed integer
    Uint,           ///< Unsigned integer
    Snorm,          ///< Signed normalized
    Unorm,          ///< Unsigned normalized
    UnormSrgb,      ///< Unsigned normalized sRGB
};

/** Pixel format description. */
struct PixelFormatDesc
{
    PixelFormat format;
    const char* name;
    PixelFormatType type;
    uint32 bytes;
    uint32 channels;
    struct
    {
        bool isDepth;
        bool isStencil;
    };
    uint32 channelBits[4];
};

extern const PixelFormatDesc gPixelFormatTable[];

inline bool IsDepthOnlyPixelFormat(PixelFormat format)
{
    return gPixelFormatTable[(uint32)format].isDepth && !gPixelFormatTable[(uint32)format].isStencil;
}

inline bool IsDepthStencilPixelFormat(PixelFormat format)
{
    return gPixelFormatTable[(uint32)format].isDepth || gPixelFormatTable[(uint32)format].isStencil;
}

inline PixelFormatType GetPixelFormatType(PixelFormat format)
{
    return gPixelFormatTable[(uint32)format].type;
}

inline uint32 GetPixelFormatBytes(PixelFormat format)
{
    return gPixelFormatTable[(uint32)format].bytes;
}

inline const PixelFormatDesc& GetPixelFormatDesc(PixelFormat format)
{
    return gPixelFormatTable[(uint32)format];
}

}
