#include "OptixDevice.h"
#include <optix_function_table_definition.h>

bool OptixInit()
{
    // Initialize CUDA
    CUDA_CHECK(cudaFree(0));
    OPTIX_CHECK(optixInit());
    return true;
}

OptixDevice::OptixDevice(const OptixDeviceCreateInfo& info)
{
    CUcontext cuContext = 0; // Zero means take the current context
    OptixDeviceContextOptions options = {};
    options.logCallbackLevel = 4; /* Fatal = 1, Error = 2, Warning = 3, Print = 4. */
    options.logCallbackFunction = [](unsigned int level, const char* tag, const char* message, void* cbdata)
    {
        switch (level) 
        {
        case 1:
            //LOG_FATAL("{}", message);
            break;
        case 2:
            //LOG_ERROR("{}", message);
            break;
        case 3:
            //LOG_WARNING("{}", message);
            break;
        case 4:
            //LOG_INFO("{}", message);
            break;
        }
    };
    OPTIX_CHECK(optixDeviceContextCreate(cuContext, &options, &context));

    CUDA_CHECK(cudaStreamCreate(&stream));
}

OptixDevice::~OptixDevice()
{
    OPTIX_CHECK(optixDeviceContextDestroy(context));
}

OptixDevice* OptixDeviceCreate(const OptixDeviceCreateInfo& info)
{
    return new OptixDevice(info);
}

void OptixDeviceDestroy(OptixDevice* device)
{
    if (device != nullptr)
    {
        delete device;
    }
}
