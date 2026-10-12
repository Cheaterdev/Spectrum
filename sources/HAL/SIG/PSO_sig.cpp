module HAL;
import Core;
import HAL;

void init_signatures(HAL::Device& device, enum_array<Layouts, HAL::RootLayout::ptr>&);
void init_pso(HAL::Device& device, enum_array<PSO, PSOBase::ptr>&);

static void init_indirect_commands(HAL::Device& device, enum_array<IndirectCommands, HAL::IndirectCommand>& commands)
{
	using HAL::IndirectCommand;
	commands[IndirectCommands::DrawIndexedArguments]  = IndirectCommand::create_command_layout<DrawIndexedArguments>(device);
	commands[IndirectCommands::DispatchMeshArguments] = IndirectCommand::create_command_layout<DispatchMeshArguments>(device);
	commands[IndirectCommands::DispatchArguments]     = IndirectCommand::create_command_layout<DispatchArguments>(device);
	commands[IndirectCommands::DispatchRaysArguments] = IndirectCommand::create_command_layout<DispatchRaysArguments>(device);
	// draw_id lands in the IndirectDraw slot's root constant (D3D12); the
	// signature then needs the root signature that slot belongs to.
	commands[IndirectCommands::DrawMeshCommand] =
		IndirectCommand::create_command_layout<Slots::Meshes::IndirectDraw, DispatchMeshArguments>(device, Layouts::DefaultLayout);
	// Its draws' shaders fetch these tables by draw id (draw_data.hlsl) rather
	// than from their slots, which stay unbound during the draw.
	for (auto slot : { SlotID::MeshInfo, SlotID::MeshInstanceInfo, SlotID::MaterialInfo, SlotID::VSMPageBatch })
		commands[IndirectCommands::DrawMeshCommand].slots.merge(slot);
}

HAL::ComputePipelineState::ptr SimpleComputePSO::create(HAL::Device& device)
{
	// [Requires = Raytracing] on a compute/graphics PSO means inline RayQuery:
	// ray tracing pipelines are state objects, not PSOs.
	auto report = [this](PSOBuildResult r) {
		if (auto& f = PSOBuildObserver::compute()) f(*this, r);
	};

	HAL::ComputePipelineStateDesc desc;
	desc.layout = root_signature;

	if ((requires_raytracing && !device.is_ray_query_supported()) || (requires_tiled_depth && !device.is_tiled_depth_supported()))
	{
		if (PSOBuildObserver::compile_unsupported())
			HAL::compute_shader::get_resource(compute);
		report(PSOBuildResult::Unsupported);
		return nullptr;
	}

	desc.shader = HAL::compute_shader::get_resource(compute);

	auto result = HAL::ComputePipelineState::create(device, desc, name);
	report(result ? PSOBuildResult::Created : PSOBuildResult::Failed);
	return result;
}

HAL::PipelineState::ptr SimpleGraphicsPSO::create(HAL::Device&device)
{
	auto report = [this](PSOBuildResult r) {
		if (auto& f = PSOBuildObserver::graphics()) f(*this, r);
	};

	HAL::PipelineStateDesc desc;
	desc.layout = root_signature;

	auto load_shaders = [&]() {
		if (!vertex.entry_point.empty())	desc.vertex = HAL::vertex_shader::get_resource(vertex);
		if (!pixel.entry_point.empty())	desc.pixel = HAL::pixel_shader::get_resource(pixel);

		if (!geometry.entry_point.empty())	desc.geometry = HAL::geometry_shader::get_resource(geometry);
		if (!domain.entry_point.empty())	desc.domain = HAL::domain_shader::get_resource(domain);
		if (!hull.entry_point.empty())	desc.hull = HAL::hull_shader::get_resource(hull);

		if (!mesh.entry_point.empty())	desc.mesh = HAL::mesh_shader::get_resource(mesh);
		if (!amplification.entry_point.empty())	desc.amplification = HAL::amplification_shader::get_resource(amplification);
	};

	if ((requires_raytracing && !device.is_ray_query_supported()) || (requires_tiled_depth && !device.is_tiled_depth_supported())
		|| ((!mesh.entry_point.empty() || !amplification.entry_point.empty()) && !device.is_mesh_shader_supported()))
	{
		// Offline tooling wants the shaders of PSOs this GPU can't run too.
		if (PSOBuildObserver::compile_unsupported())
			load_shaders();
		report(PSOBuildResult::Unsupported);
		return nullptr;
	}

	load_shaders();

	if(ds==HAL::Format::UNKNOWN)
	enable_depth  = false;
	desc.rtv.rtv_formats = rtv_formats;
	desc.rtv.ds_format = ds;
	desc.rasterizer.cull_mode = cull;
	desc.topology = topology;
	desc.rtv.func = depth_func;
	desc.rasterizer.conservative = conservative;
	desc.rasterizer.depth_bias = depth_bias;
	desc.rasterizer.depth_bias_clamp = depth_bias_clamp;
	desc.rasterizer.slope_scaled_depth_bias = slope_scaled_depth_bias;
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
	auto result = HAL::PipelineState::create(device, desc, name);
	report(result ? PSOBuildResult::Created : PSOBuildResult::Failed);
	return result;
}

void EngineRootLayoutHolder::init(HAL::Device& device)
{
	init_signatures(device, signatures);
}

void EnginePSOHolder::init(HAL::Device& device)
{
	ScopedCounter counter("pso init");
	init_pso(device, psos);
	init_indirect_commands(device, commands);
}
