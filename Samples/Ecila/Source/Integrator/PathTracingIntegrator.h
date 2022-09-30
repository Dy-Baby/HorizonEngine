#pragma once

#include "EcilaCommon.h"
#include "Optix/OptixDevice.h"

namespace Ecila
{
    class PathTracingIntegrator
    {
    public:
        PathTracingIntegrator(OptixDevice* device);
        ~PathTracingIntegrator();
        void Launch(uchar4* device_pixels, uint32 width, uint32 height);
    protected:
        OptixDevice* device;
        OptixPipeline pipeline;
        OptixModule ptxModule;
        OptixProgramGroup raygen_prog_group;
        OptixProgramGroup miss_prog_group;
        OptixShaderBindingTable sbt;
    };
}
