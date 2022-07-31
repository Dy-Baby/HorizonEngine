#include "DefaultRenderPipelineCommon.h"
#include "PostProcessing.h"
#include "SkyAtmosphere.h"

namespace HE
{

struct SkyAtmosphereConstants
{
    float bottomRadius;
    float topRadius;
    Vector3 groundAlbedo;
    Vector3 rayleighScattering;
    Vector3 mieScattering;
    Vector3 mieExtinction;
    Vector3 mieAbsorption;
    float miePhaseFunctionG;
    Vector3 absorptionExtinction;
    float rayleighDensity[12];
    float mieDensity[12];
    float absorptionDensity[12];
    float cosMaxSunZenithAngle;
    float multipleScatteringFactor;
    float rayMarchMinSPP;
    float rayMarchMaxSPP;
    uint32 transmittanceLutWidth;
    uint32 transmittanceLutHeight;
    uint32 multipleScatteringLutSize;
    uint32 skyViewLutWidth;
    uint32 skyViewLutHeight;
    uint32 aerialPerspectiveVolumeSize;
};

struct SkyAtmosphereConfig
{
    uint32 transmittanceLutWidth                = 256;
    uint32 transmittanceLutHeight               = 64;
    uint32 multipleScatteringLutSize            = 32;
    uint32 skyViewLutWidth                      = 192;
    uint32 skyViewLutHeight                     = 108;
    uint32 aerialPerspectiveVolumeSize          = 32;
};

struct RenderGraphSkyAtmosphereData
{
    RenderGraphTextureHandle transmittanceLut;
    RenderGraphTextureHandle multipleScatteringLut;
    RenderGraphTextureHandle skyViewLut;
    RenderGraphTextureHandle aerialPerspectiveVolume;
};
RENDER_GRAPH_BLACKBOARD_REGISTER_STRUCT(RenderGraphSkyAtmosphereData);

struct SkyAtmosphere
{
    MemoryArena* allocator;
    RenderBackendInterface rbi;
    RenderBackendInstance renderBackend;

    SkyAtmosphereConfig config;
    SkyAtmosphereComponent* component;

    ShaderSystem* shaderSystem;
    Shader* renderTransmittanceLutShader;
    Shader* renderMultipleScatteringLutShader;
    Shader* renderSkyViewLutShader;
    Shader* renderAerialPerspectiveVolumeShader;
    Shader* renderSkyShader;

    RenderBackendShaderHandle tonemappingShader;
    RenderBackendShaderHandle transmittanceLutShader;
    RenderBackendShaderHandle multipleScatteringLutShader;
    RenderBackendShaderHandle skyViewLutShader;
    RenderBackendShaderHandle aerialPerspectiveVolumeShader;
    RenderBackendShaderHandle renderSkyShader;

    RenderBackendBufferHandle skyAtmosphereConstantBuffer;

    RenderBackendTextureDesc transmittanceLutDesc;
    RenderBackendTextureDesc multipleScatteringLutDesc;
    RenderBackendTextureDesc skyViewLutDesc;
    RenderBackendTextureDesc aerialPerspectiveVolumeDesc;

    RenderBackendTextureHandle transmittanceLut;
    RenderBackendTextureHandle multipleScatteringLut;
    RenderBackendTextureHandle skyViewLut;
    RenderBackendTextureHandle aerialPerspectiveVolume;
};

static void SetupEarthAtmosphere(AtmosphereParameters* outAtmosphere)
{
    // Values shown here are the result of integration over wavelength power spectrum integrated with paricular function.
    // Refer to https://github.com/ebruneton/precomputed_atmospheric_scattering for details.

    // All units in kilometers
    const float EarthBottomRadius = 6360.0f;
    const float EarthTopRadius = 6460.0f;   // 100km atmosphere radius, less edge visible and it contain 99.99% of the atmosphere medium https://en.wikipedia.org/wiki/K%C3%A1rm%C3%A1n_line
    const float EarthRayleighScaleHeight = 8.0f;
    const float EarthMieScaleHeight = 1.2f;

    const double maxSunZenithAngle = PI * 120.0 / 180.0;
    *outAtmosphere = AtmosphereParameters {
        // Earth
        .bottomRadius = EarthBottomRadius,
        .topRadius = EarthTopRadius,
        .groundAlbedo = { 0.0f, 0.0f, 0.0f },
        // Raleigh scattering
        .rayleighDensity = { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, -1.0f / EarthRayleighScaleHeight, 0.0f, 0.0f } },
        .rayleighScattering = { 0.005802f, 0.013558f, 0.033100f }, // 1/km
        // Mie scattering
        .mieDensity = { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, -1.0f / EarthMieScaleHeight, 0.0f, 0.0f } },
        .mieScattering = { 0.003996f, 0.003996f, 0.003996f }, // 1/km
        .mieExtinction = { 0.004440f, 0.004440f, 0.004440f }, // 1/km
        .miePhaseFunctionG = 0.8f,
        // Ozone absorption
        .absorptionDensity = { { 25.0f, 0.0f, 0.0f, 1.0f / 15.0f, -2.0f / 3.0f }, { 0.0f, 0.0f, 0.0f, -1.0f / 15.0f, 8.0f / 3.0f } },
        .absorptionExtinction = { 0.000650f, 0.001881f, 0.000085f }, // 1/km
        .cosMaxSunZenithAngle = (float)Math::Cos(maxSunZenithAngle),
    };
}

static void Update(RenderGraph& renderGraph, SkyAtmosphere& skyAtmosphere, float multipleScatteringFactor)
{
    UpdateSkyAtmosphereConstants(skyAtmosphere, multipleScatteringFactor);

    auto& skyAtmosphereData = renderGraph.blackboard.CreateSingleton<RenderGraphSkyAtmosphereData>();
    skyAtmosphereData.transmittanceLut = renderGraph.ImportExternalTexture("Sky Atmosphere Transmittance Lut", skyAtmosphere.transmittanceLut, skyAtmosphere.transmittanceLutDesc);
    skyAtmosphereData.multipleScatteringLut = renderGraph.ImportExternalTexture("Sky Atmosphere Multiple Scattering Lut", skyAtmosphere.multipleScatteringLut, skyAtmosphere.multipleScatteringLutDesc);
    skyAtmosphereData.skyViewLut = renderGraph.ImportExternalTexture("Sky Atmosphere Sky View Lut", skyAtmosphere.skyViewLut, skyAtmosphere.skyViewLutDesc);
    skyAtmosphereData.aerialPerspectiveVolume = renderGraph.ImportExternalTexture("Sky Atmosphere Aerial Perspective Volume", skyAtmosphere.aerialPerspectiveVolume, skyAtmosphere.aerialPerspectiveVolumeDesc);
}

static void UpdateSkyAtmosphereConstants(const SkyAtmosphere& skyAtmosphere, float multipleScatteringFactor)
{
    const AtmosphereParameters& atmosphere = skyAtmosphere.component->atmosphere;
    const SkyAtmosphereConfig& config = skyAtmosphere.config;
    const float rayleighScatteringScale = 1.0f;

    SkyAtmosphereConstants constants;
    memset(&constants, 0xBA, sizeof(SkyAtmosphereConstants));
    constants.solarIrradiance = { 1.474000f, 1.850400f, 1.911980f };
    constants.solarAngularRadius = 0.004675f;
    constants.bottomRadius = atmosphere.bottomRadius;
    constants.topRadius = atmosphere.topRadius;
    constants.groundAlbedo = atmosphere.groundAlbedo;
    constants.rayleighScattering = rayleighScatteringScale * atmosphere.rayleighScattering;
    constants.mieScattering = atmosphere.mieScattering;
    constants.mieExtinction = atmosphere.mieExtinction;
    constants.mieAbsorption = atmosphere.mieExtinction - atmosphere.mieScattering;
    constants.mieAbsorption.x = Math::Max(constants.mieAbsorption.x, 0.0f);
    constants.mieAbsorption.y = Math::Max(constants.mieAbsorption.y, 0.0f);
    constants.mieAbsorption.z = Math::Max(constants.mieAbsorption.z, 0.0f);
    constants.miePhaseFunctionG = atmosphere.miePhaseFunctionG;
    constants.absorptionExtinction = atmosphere.absorptionExtinction;
    memcpy(constants.rayleighDensity, &atmosphere.rayleighDensity, sizeof(atmosphere.rayleighDensity));
    memcpy(constants.mieDensity, &atmosphere.mieDensity, sizeof(atmosphere.mieDensity));
    memcpy(constants.absorptionDensity, &atmosphere.absorptionDensity, sizeof(atmosphere.absorptionDensity));
    constants.cosMaxSunZenithAngle = atmosphere.cosMaxSunZenithAngle;
    constants.multipleScatteringFactor = multipleScatteringFactor;
    constants.transmittanceLutWidth = config.transmittanceLutWidth;
    constants.transmittanceLutHeight = config.transmittanceLutHeight;
    constants.multipleScatteringLutSize = config.multipleScatteringLutSize;
    constants.skyViewLutWidth = config.skyViewLutWidth;
    constants.skyViewLutHeight = config.skyViewLutHeight;
    constants.aerialPerspectiveVolumeSize = config.aerialPerspectiveVolumeSize;

    skyAtmosphere.rbi.WriteBuffer(skyAtmosphere.renderBackend, skyAtmosphere.skyAtmosphereConstantBuffer, 0, &constants, sizeof(constants));
}

static void RenderTransmittanceLut(RenderGraph& renderGraph, SkyAtmosphere& skyAtomosphere)
{
    RenderGraphBlackboard& blackboard = renderGraph.blackboard;
    renderGraph.AddPass("Render Transmittance Lut Pass", RenderGraphPassFlags::AsyncCompute,
    [&](RenderGraphBuilder& builder)
    {
        auto& skyAtmosphereData = blackboard.Get<RenderGraphSkyAtmosphereData>();
        auto& transmittanceLut = skyAtmosphereData.transmittanceLut = builder.WriteTexture(skyAtmosphereData.transmittanceLut, RenderBackendResourceState::UnorderedAccess);

        return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
        {
            uint32 dispatchWidth = CEIL_DIV(skyAtomosphere.config.transmittanceLutWidth, 8);
            uint32 dispatchHeight = CEIL_DIV(skyAtomosphere.config.transmittanceLutHeight, 8);
            commandList.Dispatch2D(
                data.renderTransmittanceLutShader,
                ShaderArgument(registry.CreateShaderViews()),
                dispatchWidth,
                dispatchHeight);
        };
    });
}

static void RenderMultipleScatteringLut(RenderGraph& renderGraph, SkyAtmosphere& skyAtomosphere)
{
    RenderGraphBlackboard& blackboard = renderGraph.blackboard;
    renderGraph.AddPass("Render Multilpe Scattering Lut Pass", RenderGraphPassFlags::AsyncCompute,
    [&](RenderGraphBuilder& builder)
    { 
        auto& skyAtmosphereData = blackboard.Get<RenderGraphSkyAtmosphereData>();
        auto& multipleScatteringLut = skyAtmosphereData.multipleScatteringLut = builder.WriteTexture(skyAtmosphereData.multipleScatteringLut, RenderBackendResourceState::UnorderedAccess);

        return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
        {
            uint32 dispatchWidth = CEIL_DIV(skyAtomosphere.config.multipleScatteringLutSize, 8);
            uint32 dispatchHeight = CEIL_DIV(skyAtomosphere.config.multipleScatteringLutSize, 8);
            commandList.Dispatch2D(
                renderMultipleScatteringLutShader,
                ShaderArgument(registry.CreateShaderViews()),
                dispatchWidth,
                dispatchHeight);
        };
    });
}

static void RenderSkyViewLut(RenderGraph& renderGraph, SkyAtmosphere& skyAtomosphere)
{
    RenderGraphBlackboard& blackboard = renderGraph.blackboard;
    renderGraph.AddPass("Render Sky View Lut Pass", RenderGraphPassFlags::AsyncCompute,
    [&](RenderGraphBuilder& builder)
    {  
        auto& skyAtmosphereData = blackboard.Get<RenderGraphSkyAtmosphereData>();
        auto& skyViewLut = skyAtmosphereData.skyViewLut = builder.WriteTexture(skyAtmosphereData.skyViewLut, RenderBackendResourceState::UnorderedAccess);

        return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
        {
            uint32 dispatchWidth = CEIL_DIV(skyAtomosphere.config.skyViewLutWidth, 8);
            uint32 dispatchHeight = CEIL_DIV(skyAtomosphere.config.skyViewLutHeight, 8);
            commandList.Dispatch2D(
                renderSkyViewLutShader,
                ShaderArgument(registry.CreateShaderViews()),
                dispatchWidth,
                dispatchHeight);
        };
    });
}

static void RenderAerialPerspectiveVolume(RenderGraph& renderGraph, SkyAtmosphere& skyAtomosphere)
{
    RenderGraphBlackboard& blackboard = renderGraph.blackboard;
    renderGraph.AddPass("Render Aerial Perspective Volume Pass", RenderGraphPassFlags::AsyncCompute,
    [&](RenderGraphBuilder& builder)
    { 
        auto& skyAtmosphereData = blackboard.Get<RenderGraphSkyAtmosphereData>();
        auto& aerialPerspectiveVolume = skyAtmosphereData.aerialPerspectiveVolume = builder.WriteTexture(skyAtmosphereData.aerialPerspectiveVolume, RenderBackendResourceState::UnorderedAccess);

        return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
        {
            uint32 dispatchX = skyAtomosphere.config.aerialPerspectiveVolumeSize;
            uint32 dispatchY = skyAtomosphere.config.aerialPerspectiveVolumeSize;
            uint32 dispatchZ = 1;
            commandList.Dispatch(
                data.renderAerialPerspectiveVolumeShader,
                ShaderArgument(registry.CreateShaderViews()),
                dispatchX,
                dispatchY,
                dispatchZ);
        };
    });
}

static void RenderSkyRayMarching(RenderGraph& renderGraph, SkyAtmosphere& skyAtomosphere)
{
    RenderGraphBlackboard& blackboard = renderGraph.blackboard;
    renderGraph.AddPass("Render Sky Ray Marching Pass", RenderGraphPassFlags::AsyncCompute,
    [&](RenderGraphBuilder& builder)
    {
        auto& skyAtmosphereData = blackboard.Get<RenderGraphSkyAtmosphereData>();
        const auto& viewData = blackboard.Get<RenderGraphViewData>();
        auto& colorBufferData = blackboard.Get<RenderGraphColorBuffer>();
        auto& colorBuffer = colorBufferData.colorBuffer = builder.WriteTexture(colorBufferData.colorBuffer, RenderBackendResourceState::UnorderedAccess);

        return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
        {
            uint32 dispatchWidth = CEIL_DIV(viewData.viewWidth, POST_PROCESSING_THREAD_GROUP_SIZE);
            uint32 dispatchHeight = CEIL_DIV(viewData.viewHeight, POST_PROCESSING_THREAD_GROUP_SIZE);
            commandList.Dispatch2D(
                skyAtmosphere.renderSkyShader,
                ShaderArgument(registry.CreateShaderViews()),
                dispatchWidth,
                dispatchHeight);
        };
    });
}

void RenderSky(RenderGraph& renderGraph, SkyAtmosphere skyAtmosphere)
{
    Update(renderGraph);
    RenderTransmittanceLut(renderGraph, skyAtmosphere);
    RenderMultipleScatteringLut(renderGraph, skyAtmosphere);
    RenderSkyViewLut(renderGraph, skyAtmosphere);
    RenderAerialPerspectiveVolume(renderGraph, skyAtmosphere);
    RenderSkyRayMarching(renderGraph, skyAtmosphere);
}

static void CreateSkyAtmosphere()
{
    RenderBackendBufferDesc skyAtmosphereConstantBufferDesc = RenderBackendBufferDesc::CreateByteAddress(sizeof(SkyAtmosphereConstants));
    skyAtmosphereConstantBuffer = vulkanRBI.CreateBuffer(renderBackend, deviceMask, &skyAtmosphereConstantBufferDesc, "SkyAtmosphereConstantBuffer");
    RenderBackendTextureDesc transmittanceLutDesc = RenderBackendTextureDesc::Create2D(
        256,
        64,
        PixelFormat::RGBA16Float,
        TextureCreateFlags::UnorderedAccess | TextureCreateFlags::ShaderResource);
    transmittanceLut = vulkanRBI.CreateTexture(renderBackend, deviceMask, &transmittanceLutDesc, nullptr, "TransmittanceLUT");
    RenderBackendTextureDesc multipleScatteringLutDesc = RenderBackendTextureDesc::Create2D(
        32,
        32,
        PixelFormat::RGBA16Float,
        TextureCreateFlags::UnorderedAccess | TextureCreateFlags::ShaderResource);
    multipleScatteringLut = vulkanRBI.CreateTexture(renderBackend, deviceMask, &multipleScatteringLutDesc, nullptr, "MultipleScatteringLut");
    RenderBackendTextureDesc skyViewLutDesc = RenderBackendTextureDesc::Create2D(
        192,
        108,
        PixelFormat::RGBA16Float,
        TextureCreateFlags::UnorderedAccess | TextureCreateFlags::ShaderResource);
    skyViewLut = vulkanRBI.CreateTexture(renderBackend, deviceMask, &skyViewLutDesc, nullptr, "SkyViewLut");
    RenderBackendTextureDesc aerialPerspectiveVolumeDesc = RenderBackendTextureDesc::Create3D(
        32, 32, 32,
        PixelFormat::RGBA16Float,
        TextureCreateFlags::UnorderedAccess | TextureCreateFlags::ShaderResource);
    aerialPerspectiveVolume = vulkanRBI.CreateTexture(renderBackend, deviceMask, &aerialPerspectiveVolumeDesc, nullptr, "AerialPerspectiveVolume");

    RenderBackendShaderDesc transmittanceLutShaderDesc;
    shaderCompilerInterface->CompileShader(
        shaderCompiler,
        "D:/Programming/Projects/Horizon/Shaders/SkyAtmosphereTransmittanceLut.hsf",
        TEXT("SkyAtmosphereTransmittanceLutCS"),
        RenderBackendShaderStage::Compute,
        &transmittanceLutShaderDesc.stages[(uint32)RenderBackendShaderStage::Compute]);
    transmittanceLutShaderDesc.entryPoints[(uint32)RenderBackendShaderStage::Compute] = "SkyAtmosphereTransmittanceLutCS";
    transmittanceLutShader = vulkanRBI.CreateShader(renderBackend, deviceMask, &transmittanceLutShaderDesc, "SkyAtmosphereTransmittanceLutCS");
    RenderBackendShaderDesc multipleScatteringLutShaderDesc;
    shaderCompilerInterface->CompileShader(
        shaderCompiler,
        "D:/Programming/Projects/Horizon/Shaders/SkyAtmosphereMultipleScatteringLut.hsf",
        TEXT("SkyAtmosphereMultipleScatteringLutCS"),
        RenderBackendShaderStage::Compute,
        &multipleScatteringLutShaderDesc.stages[(uint32)RenderBackendShaderStage::Compute]);
    multipleScatteringLutShaderDesc.entryPoints[(uint32)RenderBackendShaderStage::Compute] = "SkyAtmosphereMultipleScatteringLutCS";
    multipleScatteringLutShader = vulkanRBI.CreateShader(renderBackend, deviceMask, &multipleScatteringLutShaderDesc, "SkyAtmosphereMultipleScatteringLutCS");
    RenderBackendShaderDesc skyViewLutShaderDesc;
    shaderCompilerInterface->CompileShader(
        shaderCompiler,
        "D:/Programming/Projects/Horizon/Shaders/SkyAtmosphereSkyViewLut.hsf",
        TEXT("SkyAtmosphereSkyViewLutCS"),
        RenderBackendShaderStage::Compute,
        &skyViewLutShaderDesc.stages[(uint32)RenderBackendShaderStage::Compute]);
    skyViewLutShaderDesc.entryPoints[(uint32)RenderBackendShaderStage::Compute] = "SkyAtmosphereSkyViewLutCS";
    skyViewLutShader = vulkanRBI.CreateShader(renderBackend, deviceMask, &skyViewLutShaderDesc, "SkyAtmosphereSkyViewLutCS");
    RenderBackendShaderDesc aerialPerspectiveVolumeShaderDesc;
    shaderCompilerInterface->CompileShader(
        shaderCompiler,
        "D:/Programming/Projects/Horizon/Shaders/SkyAtmosphereAerialPerspectiveVolume.hsf",
        TEXT("SkyAtmosphereAerialPerspectiveVolumeCS"),
        RenderBackendShaderStage::Compute,
        &aerialPerspectiveVolumeShaderDesc.stages[(uint32)RenderBackendShaderStage::Compute]);
    aerialPerspectiveVolumeShaderDesc.entryPoints[(uint32)RenderBackendShaderStage::Compute] = "SkyAtmosphereAerialPerspectiveVolumeCS";
    aerialPerspectiveVolumeShader = vulkanRBI.CreateShader(renderBackend, deviceMask, &aerialPerspectiveVolumeShaderDesc, "SkyAtmosphereAerialPerspectiveVolumeCS");
    RenderBackendShaderDesc renderSkyShaderDesc;
    shaderCompilerInterface->CompileShader(
        shaderCompiler,
        "D:/Programming/Projects/Horizon/Shaders/SkyAtmosphereRenderSky.hsf",
        TEXT("SkyAtmosphereRenderSkyCS"),
        RenderBackendShaderStage::Compute,
        &renderSkyShaderDesc.stages[(uint32)RenderBackendShaderStage::Compute]);
    renderSkyShaderDesc.entryPoints[(uint32)RenderBackendShaderStage::Compute] = "SkyAtmosphereRenderSkyCS";
    renderSkyShader = vulkanRBI.CreateShader(renderBackend, deviceMask, &renderSkyShaderDesc, "SkyAtmosphereRenderSkyCS");


    SkyAtmosphere* skyAtmosphere = ;
    RenderBackendInterface rbi = ;
    RenderBackendInstance renderBackend;

    *skyAtmosphere = {
        .
    };

    skyAtmosphere->skyAtmosphereConstantBuffer = rbi.CreateBuffer(
        renderBackend, 
        HE_RENDER_BACKEND_DEVICES_MASK_ALL,
        &RenderBackendBufferDesc::CreateByteAddress(sizeof(SkyAtmosphereConstants)),
        "Sky Atmosphere Constant Buffer");

    skyAtmosphere->transmittanceLutDesc = RenderBackendTextureDesc::Create2D(
        skyAtmosphere->config.transmittanceLutWidth,
        skyAtmosphere->config.transmittanceLutHeight,
        SKY_ATMOSPHERE_DEFAULT_LUT_FORMAT,
        TextureCreateFlags::UnorderedAccess);
    skyAtmosphere->multipleScatteringLutDesc = RenderBackendTextureDesc::Create2D(
        skyAtmosphere->config.multipleScatteringLutSize,
        skyAtmosphere->config.multipleScatteringLutSize,
        SKY_ATMOSPHERE_DEFAULT_LUT_FORMAT,
        TextureCreateFlags::UnorderedAccess);
    skyAtmosphere->skyViewLutDesc = RenderBackendTextureDesc::Create2D(
        skyAtmosphere->config.skyViewLutWidth,
        skyAtmosphere->config.skyViewLutHeight,
        SKY_ATMOSPHERE_DEFAULT_LUT_FORMAT,
        TextureCreateFlags::UnorderedAccess);
    skyAtmosphere->aerialPerspectiveVolumeDesc = RenderBackendTextureDesc::Create3D(
        skyAtmosphere->config.aerialPerspectiveVolumeSize,
        skyAtmosphere->config.aerialPerspectiveVolumeSize,
        skyAtmosphere->config.aerialPerspectiveVolumeSize,
        SKY_ATMOSPHERE_DEFAULT_LUT_FORMAT,
        TextureCreateFlags::UnorderedAccess);

    skyAtmosphere->transmittanceLut        = rbi.CreateTexture(renderBackend, HE_RENDER_BACKEND_DEVICES_MASK_ALL, &skyAtmosphere->transmittanceLutDesc,        "Sky Atmosphere Transmittance Lut");
    skyAtmosphere->multipleScatteringLut   = rbi.CreateTexture(renderBackend, HE_RENDER_BACKEND_DEVICES_MASK_ALL, &skyAtmosphere->multipleScatteringLutDesc,   "Sky Atmosphere Multiple Scattering Lut");
    skyAtmosphere->skyViewLut              = rbi.CreateTexture(renderBackend, HE_RENDER_BACKEND_DEVICES_MASK_ALL, &skyAtmosphere->skyViewLutDesc,              "Sky Atmosphere Sky View Lut");
    skyAtmosphere->aerialPerspectiveVolume = rbi.CreateTexture(renderBackend, HE_RENDER_BACKEND_DEVICES_MASK_ALL, &skyAtmosphere->aerialPerspectiveVolumeDesc, "Sky Atmosphere Aerial Perspective Volume");

    skyAtmosphere->renderTransmittanceLutShader        = skyAtmosphere->shaderSystem->LoadShader("SkyAtmosphereTransmittanceLut.hsf",        "SkyAtmosphereTransmittanceLutCS",        nullptr);
    skyAtmosphere->renderMultipleScatteringLutShader   = skyAtmosphere->shaderSystem->LoadShader("SkyAtmosphereMultipleScatteringLut.hsf",   "SkyAtmosphereSkyViewLutCS",              nullptr);
    skyAtmosphere->renderSkyViewLutShader              = skyAtmosphere->shaderSystem->LoadShader("SkyAtmosphereSkyViewLut.hsf",              "SkyAtmosphereMultipleScatteringLutCS",   nullptr);
    skyAtmosphere->renderAerialPerspectiveVolumeShader = skyAtmosphere->shaderSystem->LoadShader("SkyAtmosphereAerialPerspectiveVolume.hsf", "SkyAtmosphereAerialPerspectiveVolumeCS", nullptr);
    skyAtmosphere->renderSkyShader                     = skyAtmosphere->shaderSystem->LoadShader("SkyAtmosphereRenderSky.hsf",               "SkyAtmosphereRenderSkyCS",               nullptr);
}

static void DestroySkyAtmosphere()
{

}

}