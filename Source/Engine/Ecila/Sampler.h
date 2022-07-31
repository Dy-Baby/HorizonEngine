#pragma once

#include "LinkedHorizonCommon.h"
#include "LinkedHorizonMath.h"
#include "SequenceGenerator.h"

#include <type_traits>
#include <utility>

namespace Ecila
{
    namespace Sobol
    {
        constexpr uint32_t reverseBits(uint32_t x)
        {
            x = ((x & 0xaaaaaaaau) >> 1) | ((x & 0x55555555u) << 1);
            x = ((x & 0xccccccccu) >> 2) | ((x & 0x33333333u) << 2);
            x = ((x & 0xf0f0f0f0u) >> 4) | ((x & 0x0f0f0f0fu) << 4);
            x = ((x & 0xff00ff00u) >> 8) | ((x & 0x00ff00ffu) << 8);
            return (x >> 16) | (x << 16);
        }
    }

    namespace detail
    {
        template<class T, T... inds, class F>
        constexpr void unrolledLoop(std::integer_sequence<T, inds...>, F&& f)
        {
            (f(std::integral_constant<T, inds>{}), ...);
        }
    }

    template<class T, T count, class F>
    constexpr void unrolledLoop(F&& f)
    {
        detail::unrolledLoop(std::make_integer_sequence<T, count>{}, std::forward<F>(f));
    }
    
    struct Sampler
    {
        Sampler() = delete;

        // Non-type template dimension-range and loop-unrolling is about 4% faster 
        // for my use, but probably doesn't work well for more Sobol dimensions.
        template<int START_DIM, int NUM_DIMS = 1>
        static auto get()
        {
            std::array<double, NUM_DIMS> res;
            unrolledLoop<int, NUM_DIMS>([&](auto i)
            {
                constexpr int DIM = START_DIM + i;
                res[i] = scramble(Sobol::bitReversedSample<DIM>(shuffled_index), hashCombine(seed, hash(DIM))) * 0x1p-32;
            });
            return res;
        }

        // Called with e.g. linear pixel index before sampling pixel
        static void initiate(uint32_t start_seed)
        {
            base_seed = hashCombine(global_seed, hash(start_seed));
        }

        // Called with e.g. ray path index before each pixel sample
        static void setIndex(uint32_t index)
        {
            sequence = 0u;
            seed = base_seed;
            bit_reversed_index = Sobol::reverseBits(index);
            shuffled_index = index;
        }

        // Called at the beginning of e.g. each ray bounce/scatter to effectively 
        // decorrelate the previously sampled dimensions from the next ("padding").
        static void shuffle()
        {
            seed = hashCombine(base_seed, hash(++sequence));
            shuffled_index = scramble(bit_reversed_index, seed);
        }

    private:
        inline thread_local static uint32_t base_seed = 0u, seed = 0u, sequence = 0u,
            bit_reversed_index = 0u, shuffled_index = 0u;

        inline static const uint32_t global_seed = std::random_device{}();

        // nested_uniform_scramble, but mostly avoids the first bit-reversal.
        static constexpr uint32_t scramble(uint32_t bit_reversed_x, uint32_t seed)
        {
            // Improved Laine-Karras hash by Nathan Vegdahl
            // https://psychopath.io/post/2021_01_30_building_a_better_lk_hash
            bit_reversed_x ^= bit_reversed_x * 0x3d20adea;
            bit_reversed_x += seed;
            bit_reversed_x *= (seed >> 16) | 1;
            bit_reversed_x ^= bit_reversed_x * 0x05526c56;
            bit_reversed_x ^= bit_reversed_x * 0x53a22864;

            return Sobol::reverseBits(bit_reversed_x);
        }

        // 2-round constants with lowest bias from:
        // https://github.com/skeeto/hash-prospector
        static constexpr uint32_t hash(uint32_t x)
        {
            x ^= x >> 15;
            x *= 0xd168aaad;
            x ^= x >> 15;
            x *= 0xaf723597;
            x ^= x >> 15;
            return x;
        }

        // Boost hash combine
        static constexpr uint32_t hashCombine(uint32_t seed, uint32_t v)
        {
            return seed ^ (v + 0x9e3779b9 + (seed << 6) + (seed >> 2));
        }
    };
}
