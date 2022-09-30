#include "Integrator/PathTracingIntegrator.h"
#include "Integrator/PathTracingIntegratorLaunchParams.h"

#include <string>
#include <vector>
#include <fstream>

namespace Ecila
{
    struct RayGenData
    {
        float r, g, b;
    };

    template <typename T>
    struct SbtRecord
    {
        __align__(OPTIX_SBT_RECORD_ALIGNMENT) char header[OPTIX_SBT_RECORD_HEADER_SIZE];
        T data;
    };

    typedef SbtRecord<RayGenData> RayGenSbtRecord;
    typedef SbtRecord<int>        MissSbtRecord;

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

        // Create module
        OptixModule module = nullptr;
        OptixPipelineCompileOptions pipeline_compile_options = {};
        {
            OptixModuleCompileOptions module_compile_options = {};
            module_compile_options.optLevel   = OPTIX_COMPILE_OPTIMIZATION_LEVEL_0;
            module_compile_options.debugLevel = OPTIX_COMPILE_DEBUG_LEVEL_FULL;

            pipeline_compile_options.usesMotionBlur        = false;
            pipeline_compile_options.traversableGraphFlags = OPTIX_TRAVERSABLE_GRAPH_FLAG_ALLOW_SINGLE_LEVEL_INSTANCING;
            pipeline_compile_options.numPayloadValues      = 2;
            pipeline_compile_options.numAttributeValues    = 2;
            pipeline_compile_options.exceptionFlags        = OPTIX_EXCEPTION_FLAG_NONE;  // TODO: should be OPTIX_EXCEPTION_FLAG_STACK_OVERFLOW;
            pipeline_compile_options.pipelineLaunchParamsVariableName = "launch_params";

            std::string input = LoadPTX("D:/Programming/HorizonEngine/Samples/Ecila/PTX/path_tracing_integrator.ptx");

            size_t sizeof_log = sizeof( log );

            OPTIX_CHECK(optixModuleCreateFromPTX(
                        device->GetContext(),
                        &module_compile_options,
                        &pipeline_compile_options,
                        input.c_str(),
                        input.size(),
                        log,
                        &sizeof_log,
                        &ptxModule));
        }

        // Create program groups, including NULL miss and hitgroups
        {
            OptixProgramGroupOptions program_group_options   = {}; // Initialize to zeros

            OptixProgramGroupDesc raygen_prog_group_desc  = {}; //
            raygen_prog_group_desc.kind                     = OPTIX_PROGRAM_GROUP_KIND_RAYGEN;
            raygen_prog_group_desc.raygen.module            = ptxModule;
            raygen_prog_group_desc.raygen.entryFunctionName = "__raygen__path_tracing_integrator";
            size_t sizeof_log = sizeof( log );
            OPTIX_CHECK(optixProgramGroupCreate(
                        device->GetContext(),
                        &raygen_prog_group_desc,
                        1,   // num program groups
                        &program_group_options,
                        log,
                        &sizeof_log,
                        &raygen_prog_group));

            // Leave miss group's module and entryfunc name null
            OptixProgramGroupDesc miss_prog_group_desc = {};
            miss_prog_group_desc.kind = OPTIX_PROGRAM_GROUP_KIND_MISS;
            sizeof_log = sizeof( log );
            OPTIX_CHECK(optixProgramGroupCreate(
                        device->GetContext(),
                        &miss_prog_group_desc,
                        1,   // num program groups
                        &program_group_options,
                        log,
                        &sizeof_log,
                        &miss_prog_group));
        }

        // Link pipeline
        {
            const uint32_t    max_trace_depth  = 0;
            OptixProgramGroup program_groups[] = { raygen_prog_group };

            OptixPipelineLinkOptions pipeline_link_options = {};
            pipeline_link_options.maxTraceDepth          = max_trace_depth;
            pipeline_link_options.debugLevel             = OPTIX_COMPILE_DEBUG_LEVEL_FULL;
            size_t sizeof_log = sizeof( log );
            OPTIX_CHECK(optixPipelineCreate(
                        device->GetContext(),
                        &pipeline_compile_options,
                        &pipeline_link_options,
                        program_groups,
                        sizeof( program_groups ) / sizeof( program_groups[0] ),
                        log,
                        &sizeof_log,
                        &pipeline));

            OptixStackSizes stack_sizes = {};
            for( auto& prog_group : program_groups )
            {
                OPTIX_CHECK( optixUtilAccumulateStackSizes( prog_group, &stack_sizes ) );
            }

            uint32_t direct_callable_stack_size_from_traversal;
            uint32_t direct_callable_stack_size_from_state;
            uint32_t continuation_stack_size;
            OPTIX_CHECK( optixUtilComputeStackSizes( &stack_sizes, max_trace_depth,
                                                        0,  // maxCCDepth
                                                        0,  // maxDCDEpth
                                                        &direct_callable_stack_size_from_traversal,
                                                        &direct_callable_stack_size_from_state, &continuation_stack_size ) );
            OPTIX_CHECK( optixPipelineSetStackSize( pipeline, direct_callable_stack_size_from_traversal,
                                                    direct_callable_stack_size_from_state, continuation_stack_size,
                                                    2  // maxTraversableDepth
                                                    ) );
        }

        // Setup shader binding table
        sbt = {};
        {
            CUdeviceptr  raygen_record;
            const size_t raygen_record_size = sizeof( RayGenSbtRecord );
            CUDA_CHECK( cudaMalloc( reinterpret_cast<void**>( &raygen_record ), raygen_record_size ) );
            RayGenSbtRecord rg_sbt;
            OPTIX_CHECK( optixSbtRecordPackHeader( raygen_prog_group, &rg_sbt ) );
            rg_sbt.data = {0.462f, 0.725f, 0.f};
            CUDA_CHECK( cudaMemcpy(
                        reinterpret_cast<void*>( raygen_record ),
                        &rg_sbt,
                        raygen_record_size,
                        cudaMemcpyHostToDevice
                        ) );

            CUdeviceptr miss_record;
            size_t      miss_record_size = sizeof( MissSbtRecord );
            CUDA_CHECK( cudaMalloc( reinterpret_cast<void**>( &miss_record ), miss_record_size ) );
            RayGenSbtRecord ms_sbt;
            OPTIX_CHECK( optixSbtRecordPackHeader( miss_prog_group, &ms_sbt ) );
            CUDA_CHECK( cudaMemcpy(
                        reinterpret_cast<void*>( miss_record ),
                        &ms_sbt,
                        miss_record_size,
                        cudaMemcpyHostToDevice
                        ) );

            sbt.raygenRecord                = raygen_record;
            sbt.missRecordBase              = miss_record;
            sbt.missRecordStrideInBytes     = sizeof( MissSbtRecord );
            sbt.missRecordCount             = 1;
        }
    }

    PathTracingIntegrator::~PathTracingIntegrator()
    {

    }

    void PathTracingIntegrator::Launch(uchar4* frame_buffer, uint32 width, uint32 height)
    {
        PathTracingIntegratorLaunchParams params;
        params.frame_buffer = frame_buffer;
        params.frame_buffer_width = width;
        params.frame_buffer_height = height;

        CUdeviceptr d_param;
        CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_param), sizeof(PathTracingIntegratorLaunchParams)));
        CUDA_CHECK(cudaMemcpy(
            reinterpret_cast<void*>(d_param),
            &params, 
            sizeof(params),
            cudaMemcpyHostToDevice));

        OPTIX_CHECK(optixLaunch(
            pipeline, 
            device->GetStream(), 
            d_param, 
            sizeof(PathTracingIntegratorLaunchParams), 
            &sbt, 
            width, 
            height, 
            1));

        CUDA_CHECK(cudaDeviceSynchronize());
        CUDA_CHECK(cudaStreamSynchronize(device->GetStream()));

        CUDA_CHECK(cudaFree(reinterpret_cast<void*>(d_param)));
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