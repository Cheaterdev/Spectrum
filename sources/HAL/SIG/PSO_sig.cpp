import HAL;
import Core;


void init_pso(enum_array<PSO, PSOBase::ptr>&);
HAL::ComputePipelineState::ptr SimpleComputePSO::create()
{
	HAL::ComputePipelineStateDesc desc;
	desc.root_signature = get_Signature(root_signature);
	desc.shader = HAL::compute_shader::get_resource(compute);

	return HAL::ComputePipelineState::create(desc, name);
}

HAL::PipelineState::ptr SimpleGraphicsPSO::create()
{
	HAL::PipelineStateDesc desc;
	desc.root_signature = get_Signature(root_signature);
	if (!vertex.entry_point.empty())	desc.vertex = HAL::vertex_shader::get_resource(vertex);
	if (!pixel.entry_point.empty())	desc.pixel = HAL::pixel_shader::get_resource(pixel);

	if (!geometry.entry_point.empty())	desc.geometry = HAL::geometry_shader::get_resource(geometry);
	if (!domain.entry_point.empty())	desc.domain = HAL::domain_shader::get_resource(domain);
	if (!hull.entry_point.empty())	desc.hull = HAL::hull_shader::get_resource(hull);

	if (!mesh.entry_point.empty())	desc.mesh = HAL::mesh_shader::get_resource(mesh);
	if (!amplification.entry_point.empty())	desc.amplification = HAL::amplification_shader::get_resource(amplification);


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
	return HAL::PipelineState::create(desc, name);
}

PSOHolder::PSOHolder()
{
	ScopedCounter counter("pso init");
	init_pso(psos);
}
