#pragma once

#include "LinkedHorizonCommon.h"
#include "LinkedHorizonMath.h"
#include "Sampler.h"
#include "Ray.h"

namespace Ecila
{
	class Integrator
	{
	public:
        virtual Vector3 Li(const Ray& ray, const Scene& scene) = 0;
	};

    class PathTracingIntegrator : public Integrator
    {
    public:
        Vector3 Li(const Ray& ray, const Scene& scene) const;
    protected:
        uint32 maxDepth = 4;
    };
}
