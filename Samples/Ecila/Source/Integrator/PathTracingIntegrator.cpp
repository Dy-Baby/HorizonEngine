#include "Integrator/PathTracingIntegrator.h"
#include "Integrator/PathTracingIntegratorLaunchParams.h"

#include <string>
#include <vector>
#include <array>
#include <fstream>

namespace Ecila
{
    static std::string LoadPTX(const char* filename)
    {
        std::string result;
        std::ifstream stream(filename, std::ios::binary);
        if (stream.good())
        {
            // Found usable source file
            std::vector<unsigned char> buffer = std::vector<unsigned char>(std::istreambuf_iterator<char>(stream), {});
            result.assign(buffer.begin(), buffer.end());
        }
        return result;
    }

    PathTracingIntegrator::PathTracingIntegrator(OptixDevice* device)
        : device(device)
    {
        char log[2048];
        size_t logSize = sizeof(log);

        // Create module
        OptixModule module = nullptr;
        OptixPipelineCompileOptions pipelineCompileOptions = {};

        {
            OptixModuleCompileOptions moduleCompileOptions = {};
            moduleCompileOptions.optLevel   = OPTIX_COMPILE_OPTIMIZATION_LEVEL_0;
            moduleCompileOptions.debugLevel = OPTIX_COMPILE_DEBUG_LEVEL_FULL;

            pipelineCompileOptions.usesMotionBlur        = false;
            pipelineCompileOptions.traversableGraphFlags = OPTIX_TRAVERSABLE_GRAPH_FLAG_ALLOW_SINGLE_GAS;
            pipelineCompileOptions.numPayloadValues      = 2;
            pipelineCompileOptions.numAttributeValues    = 2;
            pipelineCompileOptions.exceptionFlags        = OPTIX_EXCEPTION_FLAG_NONE;  // TODO: should be OPTIX_EXCEPTION_FLAG_STACK_OVERFLOW;
            pipelineCompileOptions.pipelineLaunchParamsVariableName = "launch_params";

            std::string input = LoadPTX("D:/Programming/HorizonEngine/Samples/Ecila/PTX/path_tracing_integrator.ptx");

            OPTIX_CHECK(optixModuleCreateFromPTX(
                device->GetContext(),
                &moduleCompileOptions,
                &pipelineCompileOptions,
                input.c_str(),
                input.size(),
                log,
                &logSize,
                &ptxModule));
        }

        // Create program groups
        {
            OptixProgramGroupOptions programGroupOptions = {};
            
            OptixProgramGroupDesc raygenGroupDesc = {};
            raygenGroupDesc.kind = OPTIX_PROGRAM_GROUP_KIND_RAYGEN;
            raygenGroupDesc.raygen.module = ptxModule;
            raygenGroupDesc.raygen.entryFunctionName = "__raygen__path_tracing_integrator_cast_ray";

            OPTIX_CHECK(optixProgramGroupCreate(
                device->GetContext(), 
                &raygenGroupDesc, 
                1, 
                &programGroupOptions,
                log, 
                &logSize,
                &raygenGroup));

            OptixProgramGroupDesc missGroupDesc = {};
            missGroupDesc.kind = OPTIX_PROGRAM_GROUP_KIND_MISS;
            missGroupDesc.miss.module = ptxModule;
            missGroupDesc.miss.entryFunctionName = "__miss__path_tracing_integrator";

            OPTIX_CHECK(optixProgramGroupCreate(
                device->GetContext(),
                &missGroupDesc,
                1,
                &programGroupOptions,
                log,
                &logSize,
                &missGroup));

            OptixProgramGroupDesc shadowRayMissGroupDesc = {};
            shadowRayMissGroupDesc.kind = OPTIX_PROGRAM_GROUP_KIND_MISS;
            shadowRayMissGroupDesc.miss.module = nullptr;
            shadowRayMissGroupDesc.miss.entryFunctionName = nullptr;

            OPTIX_CHECK(optixProgramGroupCreate(
                device->GetContext(),
                &shadowRayMissGroupDesc,
                1,
                &programGroupOptions,
                log,
                &logSize,
                &shadowRayMissGroup));

            OptixProgramGroupDesc primaryRayAndSecondaryRayHitGroupDesc = {};
            primaryRayAndSecondaryRayHitGroupDesc.kind = OPTIX_PROGRAM_GROUP_KIND_HITGROUP;
            primaryRayAndSecondaryRayHitGroupDesc.hitgroup.moduleCH = ptxModule;
            primaryRayAndSecondaryRayHitGroupDesc.hitgroup.entryFunctionNameCH = "__closesthit__path_tracing_integrator";

            OPTIX_CHECK(optixProgramGroupCreate(
                device->GetContext(),
                &primaryRayAndSecondaryRayHitGroupDesc,
                1,
                &programGroupOptions,
                log,
                &logSize,
                &primaryRayAndSecondaryRayHitGroup));

            OptixProgramGroupDesc shadowRayHitGroupDesc = {};
            shadowRayHitGroupDesc.kind = OPTIX_PROGRAM_GROUP_KIND_HITGROUP;
            shadowRayHitGroupDesc.hitgroup.moduleCH = ptxModule;
            shadowRayHitGroupDesc.hitgroup.entryFunctionNameCH = "__closesthit__path_tracing_integrator_shadow_ray";

            OPTIX_CHECK(optixProgramGroupCreate(
                device->GetContext(),
                &shadowRayHitGroupDesc,
                1,
                &programGroupOptions,
                log,
                &logSize,
                &shadowRayHitGroup));
        }

        // Link pipeline
        {
            const uint32 maxTraceDepth = 2;
            OptixProgramGroup programGroups[] = {
                raygenGroup,
                missGroup,
                shadowRayMissGroup,
                primaryRayAndSecondaryRayHitGroup,
                shadowRayHitGroup,
            };

            OptixPipelineLinkOptions pipelineLinkOptions = {};
            pipelineLinkOptions.maxTraceDepth = maxTraceDepth;
            pipelineLinkOptions.debugLevel = OPTIX_COMPILE_DEBUG_LEVEL_FULL;

            OPTIX_CHECK(optixPipelineCreate(
                device->GetContext(),
                &pipelineCompileOptions,
                &pipelineLinkOptions,
                programGroups,
                ARRAYSIZE(programGroups),
                log,
                &logSize,
                &pipeline));

            OptixStackSizes stackSizes = {};
            for (auto& programGroup : programGroups)
            {
                OPTIX_CHECK(optixUtilAccumulateStackSizes(programGroup, &stackSizes));
            }

            uint32 direct_callable_stack_size_from_traversal;
            uint32 direct_callable_stack_size_from_state;
            uint32 continuation_stack_size;
            OPTIX_CHECK(optixUtilComputeStackSizes(
                &stackSizes, 
                maxTraceDepth, 
                0, 
                0, 
                &direct_callable_stack_size_from_traversal, 
                &direct_callable_stack_size_from_state, 
                &continuation_stack_size));

            OPTIX_CHECK(optixPipelineSetStackSize(
                pipeline, 
                direct_callable_stack_size_from_traversal, 
                direct_callable_stack_size_from_state, 
                continuation_stack_size, 
                1));
        }

        // Setup shader binding table
        sbt = {};
        {
            RaygenRecord raygenRecord = {};
            OPTIX_CHECK(optixSbtRecordPackHeader(raygenGroup, &raygenRecord));

            CUdeviceptr raygenRecordDeviceAddress;
            const size_t raygenRecordSize = sizeof(RaygenRecord);
            CUDA_CHECK(cudaMalloc((void**)&raygenRecordDeviceAddress, raygenRecordSize));
            CUDA_CHECK(cudaMemcpy(
                (void*)raygenRecordDeviceAddress,
                &raygenRecord,
                raygenRecordSize,
                cudaMemcpyHostToDevice));

            uint32 numMissRecords = 2;
            const size_t missRecordSize = sizeof(MissRecord);

            MissRecord missRecords[2];
            OPTIX_CHECK(optixSbtRecordPackHeader(missGroup, &missRecords[0]));
            OPTIX_CHECK(optixSbtRecordPackHeader(shadowRayMissGroup, &missRecords[1]));

            CUdeviceptr missRecordDeviceAddressBase;
            CUDA_CHECK(cudaMalloc((void**)&missRecordDeviceAddressBase, numMissRecords * missRecordSize));
            CUDA_CHECK(cudaMemcpy(
                (void*)missRecordDeviceAddressBase,
                missRecords,
                numMissRecords * missRecordSize,
                cudaMemcpyHostToDevice));

            const std::array<float3, 4> g_emission_colors =
            { {
                {  0.0f,  0.0f,  0.0f },
                {  0.0f,  0.0f,  0.0f },
                {  0.0f,  0.0f,  0.0f },
                { 15.0f, 15.0f,  5.0f }

            } };

            const std::array<float3, 4> g_diffuse_colors =
            { {
                { 0.80f, 0.80f, 0.80f },
                { 0.05f, 0.80f, 0.05f },
                { 0.80f, 0.05f, 0.05f },
                { 0.50f, 0.00f, 0.00f }
            } };

            HitgroupRecord hitgropuRecords[8];
            for (int i = 0; i < 4; i++)
            {
                {
                    const int sbt_idx = i * 2 + 0;  // SBT for radiance ray-type for ith material
                    OPTIX_CHECK(optixSbtRecordPackHeader(primaryRayAndSecondaryRayHitGroup, &hitgropuRecords[sbt_idx]));
                    hitgropuRecords[sbt_idx].data.emission_color = g_emission_colors[i];
                    hitgropuRecords[sbt_idx].data.diffuse_color = g_diffuse_colors[i];
                }

                {
                    const int sbt_idx = i * 2 + 1;  // SBT for occlusion ray-type for ith material
                    OPTIX_CHECK(optixSbtRecordPackHeader(shadowRayHitGroup, &hitgropuRecords[sbt_idx]));
                }
            }

            CUdeviceptr hitgroupRecordDeviceAddressBase;
            uint32 numHitgroupRecords = 8;
            const size_t hitgroupRecordSize = sizeof(HitgroupRecord);
            CUDA_CHECK(cudaMalloc((void**)&hitgroupRecordDeviceAddressBase, numHitgroupRecords * hitgroupRecordSize));
            CUDA_CHECK(cudaMemcpy(
                (void*)hitgroupRecordDeviceAddressBase,
                hitgropuRecords,
                numHitgroupRecords * hitgroupRecordSize,
                cudaMemcpyHostToDevice));

            sbt.raygenRecord = raygenRecordDeviceAddress;
            sbt.missRecordBase = missRecordDeviceAddressBase;
            sbt.missRecordStrideInBytes = (uint32)missRecordSize;
            sbt.missRecordCount = numMissRecords;
            sbt.hitgroupRecordBase = hitgroupRecordDeviceAddressBase;
            sbt.hitgroupRecordStrideInBytes = (uint32)hitgroupRecordSize;
            sbt.hitgroupRecordCount = numHitgroupRecords;
        }
    }

    PathTracingIntegrator::~PathTracingIntegrator()
    {

    }

    void PathTracingIntegrator::Launch(const PathTracingIntegratorLaunchParams& params, uint32 width, uint32 height)
    {
        CUdeviceptr launchParamsDeviceAddress;
        CUDA_CHECK(cudaMalloc((void**)&launchParamsDeviceAddress, sizeof(PathTracingIntegratorLaunchParams)));
        CUDA_CHECK(cudaMemcpy(
            (void*)launchParamsDeviceAddress,
            &params,
            sizeof(PathTracingIntegratorLaunchParams),
            cudaMemcpyHostToDevice));
        
        CUDA_CHECK(cudaDeviceSynchronize());
        CUDA_CHECK(cudaStreamSynchronize(device->GetStream()));

        OPTIX_CHECK(optixLaunch(
            pipeline, 
            device->GetStream(), 
            launchParamsDeviceAddress,
            sizeof(PathTracingIntegratorLaunchParams), 
            &sbt, 
            width, 
            height, 
            1));

        CUDA_CHECK(cudaDeviceSynchronize());
        CUDA_CHECK(cudaStreamSynchronize(device->GetStream()));
        CUDA_CHECK(cudaFree((void*)launchParamsDeviceAddress));
    }

    // Vector3 PathTracingIntegrator::Li(const Ray& ray, const SharedPtr<Scene>& scene) const
    // {
    //     float tMin = 0.001f;
    //     float tMax = FLOAT_MAX;

    //     Vector3 L = { 0.0f, 0.0f, 0.0f };
    //     Vector3 attenuation = { 1.0f, 1.0f, 1.0f };
    //     uint32 depth = 0;

    //     while (true)
    //     {
    //         HitRecord hitRecord = scene->TraversalAccelerations(ray, tMin, tMax);
    //         if (hitRecord.tHit < 0.0f || hitRecord.tHit == FLOAT_MAX)
    //         {
    //             // SkyColor();
    //             break;
    //         }

    //         //Vector3 Le = interaction.Le(-ray.mDirection);
    //         //if (Le)
    //         //{
    //         //    if (depth == 0 || specularBounce)
    //         //    {
    //         //        L += attenuation * Le;
    //         //    }
    //         //    else
    //         //    {
    //         //        // Compute MIS weight for area light
    //         //        Light areaLight(si->intr.areaLight);
    //         //        Float lightPDF =
    //         //            lightSampler.PDF(prevIntrCtx, areaLight) *
    //         //            areaLight.PDF_Li(prevIntrCtx, ray.d, LightSamplingMode::WithMIS);
    //         //        Float weight = PowerHeuristic(1, bsdfPDF, 1, lightPDF);
    //         //        L += attenuation * weight * Le;
    //         //    }
    //         //}

    //         BSDF bsdf = isect.GetBSDF(ray, lambda, camera, scratchBuffer, sampler);
    //         if (!bsdf)
    //         {
    //             isect.SkipIntersection(&ray, si->tHit);
    //             continue;
    //         }

    //         if (depth == maxDepth)
    //         {
    //             break;
    //         }
    //         depth++;

    //         Vector3 wo = -ray.direction;
    //         Vector3 p = hitRecord.point;
    //         Vector3 n = hitRecord.normal;

    //         float u = sampler.Get1D();
    //         std::optional<BSDFSample> bs = bsdf.Sample_f(wo, u, sampler.Get2D());
    //         if (!bs)
    //         {
    //             break;
    //         }
    //         attenuation *= bs->f * AbsDot(bs->wi, isect.shading.n) / bs->pdf;
    //         specularBounce = bs->IsSpecular();
    //         ray = isect.SpawnRay(bs->wi);

    //         float pdf = 0;

    //         ray = interaction.SpawnRay(ray, bsdf, bs->wi, bs->flags, bs->eta);

    //         // Russian roulette
    //         if (depth > 1)
    //         {
    //             float q = std::max(0.05f, 1.0f - Math::Max(attenuation));
    //             if (sampler.Get1D() < q)
    //             {
    //                 break;
    //             }
    //             attenuation /= 1 - q;
    //             ASSERT(!std::isinf(attenuation));
    //             // TODO: Check beta is not inf.
    //         }
    //     }
    //     return L;
    // }
}