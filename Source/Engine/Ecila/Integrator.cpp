#include "Integrator.h"
#include "Interaction.h"
#include "RenderScene.h"
#include "BSDF.h"
#include "MathUtil.h"

namespace Ecila
{
    Vector3 PathTracingIntegrator::Li(const Ray& ray, const SharedPtr<Scene>& scene) const
    {
        float tMin = 0.001f;
        float tMax = FLOAT_MAX;

        Vector3 L = { 0.0f, 0.0f, 0.0f };
        Vector3 attenuation = { 1.0f, 1.0f, 1.0f };
        uint32 depth = 0;

        while (true)
        {
            HitRecord hitRecord = scene->TraversalAccelerations(ray, tMin, tMax);
            if (hitRecord.tHit < 0.0f || hitRecord.tHit == FLOAT_MAX)
            {
                // SkyColor();
                break;
            }

            //Vector3 Le = interaction.Le(-ray.mDirection);
            //if (Le)
            //{
            //    if (depth == 0 || specularBounce)
            //    {
            //        L += attenuation * Le;
            //    }
            //    else
            //    {
            //        // Compute MIS weight for area light
            //        Light areaLight(si->intr.areaLight);
            //        Float lightPDF =
            //            lightSampler.PDF(prevIntrCtx, areaLight) *
            //            areaLight.PDF_Li(prevIntrCtx, ray.d, LightSamplingMode::WithMIS);
            //        Float weight = PowerHeuristic(1, bsdfPDF, 1, lightPDF);
            //        L += attenuation * weight * Le;
            //    }
            //}

            BSDF bsdf = isect.GetBSDF(ray, lambda, camera, scratchBuffer, sampler);
            if (!bsdf)
            {
                isect.SkipIntersection(&ray, si->tHit);
                continue;
            }

            if (depth == maxDepth)
            {
                break;
            }
            depth++;

            Vector3 wo = -ray.direction;
            Vector3 p = hitRecord.point;
            Vector3 n = hitRecord.normal;

            float u = sampler.Get1D();
            std::optional<BSDFSample> bs = bsdf.Sample_f(wo, u, sampler.Get2D());
            if (!bs)
            {
                break;
            }
            attenuation *= bs->f * AbsDot(bs->wi, isect.shading.n) / bs->pdf;
            specularBounce = bs->IsSpecular();
            ray = isect.SpawnRay(bs->wi);

            float pdf = 0;

            ray = interaction.SpawnRay(ray, bsdf, bs->wi, bs->flags, bs->eta);

            // Russian roulette
            if (depth > 1)
            {
                float q = std::max(0.05f, 1.0f - Math::Max(attenuation));
                if (sampler.Get1D() < q)
                {
                    break;
                }
                attenuation /= 1 - q;
                ASSERT(!std::isinf(attenuation));
                // TODO: Check beta is not inf.
            }
        }

        return L;
    }
   
}