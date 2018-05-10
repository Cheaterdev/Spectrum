#include "pch.h"

DX11_DeviceContext DX11::NativeContextAccessor::get_native_context(NativeContext &context)
{
    return context.get_native();
}

int DX11::NativeContext::get_total_optimizations()
{
    return 	 layout_optimized_count
             + geometry_optimized_count
             + vertex_optimized_count
             + pixel_optimized_count
             + state_optimized_count
             + srv_optimized_count
             + rtv_optimized_count
             + const_buffer_optimized_count
             + sampler_optimized_count
             + topology_optimized_count
             + viewports_optimized_count
			 + total_pipeline_count;
}

void DX11::NativeContext::clear_optimized_count()
{
    layout_optimized_count = 0;
    geometry_optimized_count = 0;
    pixel_optimized_count = 0;
    vertex_optimized_count = 0;
    state_optimized_count = 0;
    srv_optimized_count = 0;
    rtv_optimized_count = 0;
    const_buffer_optimized_count = 0;
    sampler_optimized_count = 0;
    topology_optimized_count = 0;
    viewports_optimized_count = 0;
	total_pipeline_count = 0;
}

DX11_DeviceContext DX11::NativeContext::get_native()
{
    return native_context;
}

DX11::NativeContext::NativeContext(DX11_DeviceContext _native_context)
{
    native_context = _native_context;
    clear_optimized_count();
}
