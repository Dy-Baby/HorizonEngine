#include "EcilaCommon.h"
#include "Optix/OptixDevice.h"
#include "Integrator/PathTracingIntegrator.h"

#include <vector>
#include <fstream>

namespace Ecila
{
    struct Image
    {
        unsigned int width = 0;
        unsigned int height = 0;
        void* data = nullptr;
    };

    static bool SaveAsPPM(const char* filename, const unsigned char* data, int width, int height, int channels)
    {
        if (data == nullptr || width < 1 || height < 1)
        {
            return false;
        }

        if (channels != 1 && channels != 3 && channels != 4)
        {
            return false;
        }

        std::ofstream stream(filename, std::ios::out | std::ios::binary);
        if (!stream.is_open())
        {
            return false;
        }

        bool is_float = false;
        stream << 'P';
        stream << ((channels == 1 ? (is_float ? 'Z' : '5') : (channels == 3 ? (is_float ? '7' : '6') : '8'))) << std::endl;
        stream << width << " " << height << std::endl << 255 << std::endl;

        stream.write(reinterpret_cast<char*>(const_cast<unsigned char*>(data)), width * height * channels * (is_float ? 4 : 1));
        stream.close();

        return true;
    }

    void RenderScene(OptixDevice* device)
    {
        uint32 width = 1280;
        uint32 height = 720;

        uchar4* device_pixels = nullptr;
        uint64 size = width * height * sizeof(uchar4);
        CUDA_CHECK(cudaMalloc(&device_pixels, size));

        PathTracingIntegrator integrator(device);
        integrator.Launch(device_pixels, width, height);

        std::vector<uchar4> host_pixels(width * height);

        CUDA_CHECK(cudaMemcpy(
            host_pixels.data(),
            device_pixels,
            width * height * sizeof(uchar4),
            cudaMemcpyDeviceToHost));
        CUDA_CHECK(cudaStreamSynchronize(device->GetStream()));

        Image image;
        image.width = width;
        image.width = height;
        image.data = host_pixels.data();

        std::vector<unsigned char> pixels(width * height * 3);
        {
            for(int j = height - 1; j >= 0; j--)
            {
                for(int i = 0; i < width; i++)
                {
                    const int32_t dst_idx = 3*width*(height-j-1) + 3*i;
                    const int32_t src_idx = 4*width*j            + 4*i;
                    pixels[dst_idx + 0] = reinterpret_cast<uint8_t*>( image.data )[ src_idx+0 ];
                    pixels[dst_idx + 1] = reinterpret_cast<uint8_t*>( image.data )[ src_idx+1 ];
                    pixels[dst_idx + 2] = reinterpret_cast<uint8_t*>( image.data )[ src_idx+2 ];
                }
            }
        }

        SaveAsPPM("D:/test.ppm", pixels.data(), width, height, 3);
    }

    int EcilaMain(int argc, const char** argv)
    {
        if (!OptixInit())
        {
            return -1;
        }

        OptixDeviceCreateInfo deviceInfo = {};
        OptixDevice* device = OptixDeviceCreate(deviceInfo);

        RenderScene(device);

        OptixDeviceDestroy(device);

        return 0;
    }
}

int main(int argc, const char** argv)
{
    int exitCode = 0;
    exitCode = Ecila::EcilaMain(argc, argv);
    return exitCode;
}
