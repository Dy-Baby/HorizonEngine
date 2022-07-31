#pragma once

#include "Core/Core.h"

#define HE_MGPU         1 
#define HE_MAX_NUM_GPUS 16

namespace HE
{
	struct RenderBackendGpuMask
	{
	public:	
		FORCEINLINE static const RenderBackendGpuMask All()
		{ 
			return RenderBackendGpuMask(~0u);
		}

		FORCEINLINE RenderBackendGpuMask() : RenderBackendGpuMask(RenderBackendGpuMask::All()) {}

		FORCEINLINE bool operator ==(const RenderBackendGpuMask& rhs) const { return mask == rhs.mask; }
		FORCEINLINE bool operator !=(const RenderBackendGpuMask& rhs) const { return mask != rhs.mask; }
		void operator |=(const RenderBackendGpuMask& rhs) { mask |= rhs.mask; }
		void operator &=(const RenderBackendGpuMask& rhs) { mask &= rhs.mask; }
		FORCEINLINE RenderBackendGpuMask operator &(const RenderBackendGpuMask& rhs) const
		{
			return RenderBackendGpuMask(mask & rhs.mask);
		}
		FORCEINLINE RenderBackendGpuMask operator |(const RenderBackendGpuMask& rhs) const
		{
			return RenderBackendGpuMask(mask | rhs.mask);
		}

		struct Iterator
		{
			static uint32 CountTrailingZeros(uint32 value)
			{
				if (value == 0)
				{
					return 32;
				}
				unsigned long result;
				_BitScanForward(&result, value);
				return (uint32)result;
			}
			FORCEINLINE explicit Iterator(uint32 mask) : mask(mask), firstNonZeroBit(0)
			{
				firstNonZeroBit = CountTrailingZeros(mask);
			}

			FORCEINLINE explicit Iterator(const RenderBackendGpuMask& gpuMask) : Iterator(gpuMask.mask)
			{
			}

			FORCEINLINE Iterator& operator++()
			{
				mask &= ~(1 << firstNonZeroBit);
				firstNonZeroBit = CountTrailingZeros(mask);
				return *this;
			}

			FORCEINLINE Iterator operator++(int)
			{
				Iterator copy(*this);
				++*this;
				return copy;
			}

			FORCEINLINE uint32 operator*() const { return firstNonZeroBit; }
			FORCEINLINE bool operator !=(const Iterator& rhs) const { return mask != rhs.mask; }
			FORCEINLINE explicit operator bool() const { return mask != 0; }
			FORCEINLINE bool operator !() const { return !(bool)*this; }

		private:
			uint32 mask;
			uint32 firstNonZeroBit;
		};

		FORCEINLINE friend RenderBackendGpuMask::Iterator begin(const RenderBackendGpuMask& gpuMask) { return RenderBackendGpuMask::Iterator(gpuMask.mask); }
		FORCEINLINE friend RenderBackendGpuMask::Iterator end(const RenderBackendGpuMask& gpuMask) { return RenderBackendGpuMask::Iterator(0); }

	private:
		FORCEINLINE explicit RenderBackendGpuMask(uint32 mask) : mask(mask) {}
		uint32 mask;
	};
}