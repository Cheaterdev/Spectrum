/**********************************************************************
Copyright (c) 2021 Advanced Micro Devices, Inc. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
********************************************************************/
#include "reflection_common.hlsl"
#include "../autogen/DenoiserReflectionResolve.h"

min16float3 FFX_DNSR_Reflections_SampleAverageRadiance(float2 uv) { return (min16float3)GetDenoiserReflectionResolve().GetG_average_radiance().SampleLevel(linearClampSampler, uv, 0.0f).xyz; }
min16float3 FFX_DNSR_Reflections_LoadRadiance(int2 pixel_coordinate) { return (min16float3)GetDenoiserReflectionResolve().GetG_in_radiance().Load(int3(pixel_coordinate, 0)).xyz; }
min16float3 FFX_DNSR_Reflections_LoadRadianceReprojected(int2 pixel_coordinate) { return (min16float3)GetDenoiserReflectionResolve().GetG_in_reprojected_radiance().Load(int3(pixel_coordinate, 0)).xyz; }
min16float FFX_DNSR_Reflections_LoadRoughness(int2 pixel_coordinate) { return (min16float)GetDenoiserReflectionResolve().GetG_normal().Load(int3(pixel_coordinate, 0)).w; }
min16float FFX_DNSR_Reflections_LoadVariance(int2 pixel_coordinate) { return (min16float)GetDenoiserReflectionResolve().GetG_in_variance().Load(int3(pixel_coordinate, 0)).x; }
min16float FFX_DNSR_Reflections_LoadNumSamples(int2 pixel_coordinate) { return (min16float)GetDenoiserReflectionResolve().GetG_in_sample_count().Load(int3(pixel_coordinate, 0)).x; }
void FFX_DNSR_Reflections_StoreTemporalAccumulation(int2 pixel_coordinate, min16float3 radiance, min16float variance) {
    GetDenoiserReflectionResolve().GetG_out_radiance()[pixel_coordinate] = radiance.xyzz;
    GetDenoiserReflectionResolve().GetG_out_variance()[pixel_coordinate] = variance.x;
}

#include "ffx-reflection-dnsr/ffx_denoiser_reflections_resolve_temporal.h"

[numthreads(8, 8, 1)]
void main(int2 group_thread_id      : SV_GroupThreadID,
                uint group_index    : SV_GroupIndex,
                uint    group_id    : SV_GroupID,
                uint2 did : SV_DispatchThreadID) {
    //uint  packed_coords               = g_denoiser_tile_list[group_id];
    //int2  dispatch_thread_id          = int2(packed_coords & 0xffffu, (packed_coords >> 16) & 0xffffu) + group_thread_id;
    //int2  dispatch_group_id           = dispatch_thread_id / 8;
    //uint2 remapped_group_thread_id    = FFX_DNSR_Reflections_RemapLane8x8(group_index);
    //uint2 remapped_dispatch_thread_id = dispatch_group_id * 8 + remapped_group_thread_id;

    //FFX_DNSR_Reflections_ResolveTemporal(remapped_dispatch_thread_id, remapped_group_thread_id, g_buffer_dimensions, g_inv_buffer_dimensions, g_temporal_stability_factor);
    FFX_DNSR_Reflections_ResolveTemporal(did, group_thread_id, GetDenoiserReflectionCommon().GetG_buffer_dimensions(),  GetDenoiserReflectionCommon().GetG_inv_buffer_dimensions(),  GetDenoiserReflectionCommon().GetG_temporal_stability_factor());
}