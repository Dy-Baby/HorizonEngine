#pragma once

#include <HorizonEngine.h>

namespace HE
{

#ifdef HIGHT_QUALITY_SKY_ATMOSPHERE
#define SKY_ATMOSPHERE_DEFAULT_LUT_FORMAT PixelFormat::RGBA32Float
#else
#define SKY_ATMOSPHERE_DEFAULT_LUT_FORMAT PixelFormat::RGBA16Float
#endif

struct AtmosphereEditorProperties
{
    float miePhaseFunctionG;
    int numScatteringOrder;
    Vector3 mieScatteringColor;
    float mieScatteringLength;
    Vector3 mieAbsColor;
    float mieAbsLength;
    Vector3 rayleighScatteringColor;
    float rayleighScatteringLength;
    Vector3 absorptionColor;
    float absorptionLength;
    float planetRadius;
    float atmosphereHeight;
    float mieScaleHeight;
    float mayleighScaleHeight;
    Vector3 groundAbledo;
};

struct DensityProfileLayer
{
    float width;
    float expTerm;
    float expScale;
    float linearTerm;
    float constantTerm;
};

struct AtmosphereParameters 
{
    /// The distance between the planet center and the bottom of the atmosphere.
    float bottomRadius;
    /// The distance between the planet center and the top of the atmosphere.
    float topRadius;
    /// The average albedo of the ground.
    Vector3 groundAlbedo;
    /// The density profile of air molecules, i.e. a function from altitude to
    /// dimensionless values between 0 (null density) and 1 (maximum density).
    DensityProfileLayer rayleighDensity[2];
    /// The scattering coefficient of air molecules at the altitude where their
    /// density is maximum (usually the bottom of the atmosphere), as a function of
    /// wavelength. The scattering coefficient at altitude h is equal to
    /// 'rayleigh_scattering' times 'rayleigh_density' at this altitude.
    Vector3 rayleighScattering;
    /// The density profile of aerosols, i.e. a function from altitude to
    /// dimensionless values between 0 (null density) and 1 (maximum density).
    DensityProfileLayer mieDensity[2];
    /// The scattering coefficient of aerosols at the altitude where their density
    /// is maximum (usually the bottom of the atmosphere), as a function of
    /// wavelength. The scattering coefficient at altitude h is equal to
    /// 'mie_scattering' times 'mie_density' at this altitude.
    Vector3 mieScattering;
    /// The extinction coefficient of aerosols at the altitude where their density
    /// is maximum (usually the bottom of the atmosphere), as a function of
    /// wavelength. The extinction coefficient at altitude h is equal to
    /// 'mie_extinction' times 'mie_density' at this altitude.
    Vector3 mieExtinction;
    /// The asymetry parameter for the Cornette-Shanks phase function for the aerosols.
    float miePhaseFunctionG;
    /// The density profile of air molecules that absorb light (e.g. ozone), i.e.
    /// a function from altitude to dimensionless values between 0 (null density)
    /// and 1 (maximum density).
    DensityProfileLayer absorptionDensity[2];
    /// The extinction coefficient of molecules that absorb light (e.g. ozone) at
    /// the altitude where their density is maximum, as a function of wavelength.
    /// The extinction coefficient at altitude h is equal to
    /// 'absorption_extinction' times 'absorption_density' at this altitude.
    Vector3 absorptionExtinction;
    /// The cosine of the maximum Sun zenith angle for which atmospheric scattering
    /// must be precomputed (for maximum precision, use the smallest Sun zenith
    /// angle yielding negligible sky light radiance values. For instance, for the
    /// Earth case, 102 degrees is a good choice - yielding mu_s_min = -0.2).
    float cosMaxSunZenithAngle;
};

//struct AtmosphereParameters
//{
//    /// Radius of the planet (center to ground)
//    float bottomRadius;
//    /// Maximum considered atmosphere height (center to atmosphere top)
//    float topRadius;
//    /// Rayleigh scattering exponential distribution scale in the atmosphere
//    float rayleighDensityExpScale;
//    /// Rayleigh scattering coefficients
//    Vector3 rayleighScattering;
//    /// Mie scattering exponential distribution scale in the atmosphere
//    float mieDensityExpScale;
//    /// Mie scattering coefficients
//    Vector3 mieScattering;
//    /// Mie extinction coefficients
//    Vector3 mieExtinction;
//    /// Mie absorption coefficients
//    Vector3 mieAbsorption;
//    /// Mie phase function excentricity
//    float miePhaseG;
//    /// Another medium type in the atmosphere
//    float absorptionDensity0LayerWidth;
//    float absorptionDensity0ConstantTerm;
//    float absorptionDensity0LinearTerm;
//    float absorptionDensity1ConstantTerm;
//    float absorptionDensity1LinearTerm;
//    /// This other medium only absorb light, e.g. useful to represent ozone in the earth atmosphere
//    Vector3 absorptionExtinction;
//    /// The albedo of the ground.
//    Vector3 groundAlbedo;
//};

struct SkyAtmosphereComponent
{
	AtmosphereParameters atmosphere;
    EntityHandle sun;
    float viewRayMarchMinSPP;
    float viewRayMarchMaxSPP;
};

void RenderSky(RenderGraph& renderGraph, SkyAtmosphere, );

}