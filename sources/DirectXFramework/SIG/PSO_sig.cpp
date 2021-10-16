#include "pch_dx.h"
#include "DX12/dx12_types.h"
#include "DX12/PipelineState.h"
#include "utils/utils_macros.h"
import Enums;
import PSO;
import Data;
import Profiling;
import serialization;
//#include "autogen/includes.h"
void init_pso(enum_array<PSO, PSOBase::ptr>&);
Render::ComputePipelineState::ptr SimpleComputePSO::create()
{
	Render::ComputePipelineStateDesc desc;
	desc.root_signature = get_Signature(root_signature);
	desc.shader = Render::compute_shader::get_resource(compute);

	return Render::ComputePipelineState::create(desc, name);
}

Render::PipelineState::ptr SimpleGraphicsPSO::create()
{
	Render::PipelineStateDesc desc;
	desc.root_signature = get_Signature(root_signature);
	if (!vertex.entry_point.empty())	desc.vertex = Render::vertex_shader::get_resource(vertex);
	if (!pixel.entry_point.empty())	desc.pixel = Render::pixel_shader::get_resource(pixel);

	if (!geometry.entry_point.empty())	desc.geometry = Render::geometry_shader::get_resource(geometry);
	if (!domain.entry_point.empty())	desc.domain = Render::domain_shader::get_resource(domain);
	if (!hull.entry_point.empty())	desc.hull = Render::hull_shader::get_resource(hull);

	if (!mesh.entry_point.empty())	desc.mesh = Render::mesh_shader::get_resource(mesh);
	if (!amplification.entry_point.empty())	desc.amplification = Render::amplification_shader::get_resource(amplification);


	desc.rtv.rtv_formats = rtv_formats;
	desc.rtv.ds_format = ds;
	desc.rasterizer.cull_mode = cull;
	desc.topology = topology;
	desc.rtv.func = depth_func;
	desc.rasterizer.conservative = conservative;
	desc.rtv.enable_depth_write = depth_write;
	desc.rtv.enable_depth = enable_depth;

	desc.rtv.enable_stencil = enable_stencil;
	desc.rtv.stencil_read_mask = stencil_read_mask;
	desc.rtv.stencil_write_mask = stencil_write_mask;
	desc.rtv.stencil_desc.StencilFunc = stencil_func;
	desc.rtv.stencil_desc.StencilPassOp = stencil_pass_op;

	for(int i=0;i<blend.size();i++)
	desc.blend.render_target[i] = blend[i];

	desc.blend.independ_blend = rtv_formats.size() == blend.size();
	return Render::PipelineState::create(desc, name);
}

PSOHolder::PSOHolder()
{
	ScopedCounter counter("pso init");
	init_pso(psos);
}
