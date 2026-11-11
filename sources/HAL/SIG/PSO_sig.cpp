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

// PSOs a specific GPU's driver cannot build, by GPU name fragment and PSO name. These are skipped
// like any other unsupported PSO (create() returns null), so only that one pipeline is lost, not the
// whole feature it belongs to. Temporary: remove an entry once the shader or the driver is fixed.
static bool driver_cannot_build(HAL::Device& device, const std::string& pso_name)
{
	struct Entry { const char* gpu; const char* pso; const char* why; };
	static const Entry entries[] = {
		// Adreno's shader compiler never finishes it (100% CPU, GBs of memory, the app is killed).
		// A debug visualization, so nothing is lost; cause not isolated yet (2 ray queries only).
		{ "Adreno", "DDGIIndirectDebug", "driver shader compiler never finishes" },
	};

	const auto& gpu = device.get_properties().name;
	for (const auto& e : entries)
		if (pso_name == e.pso && gpu.find(e.gpu) != std::string::npos)
		{
			Log::get() << "PSO " << pso_name << " skipped on " << gpu << ": " << e.why << Log::endl;
			return true;
		}
	return false;
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

	if (driver_cannot_build(device, name))
	{
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
	auto desc = make_desc(device);
	if (!desc)
		return nullptr;

	auto result = HAL::PipelineState::create(device, *desc, name);
	if (auto& f = PSOBuildObserver::graphics()) f(*this, result ? PSOBuildResult::Created : PSOBuildResult::Failed);
	return result;
}

std::optional<HAL::PipelineStateDesc> SimpleGraphicsPSO::make_desc(HAL::Device& device)
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
		return std::nullopt;
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
	desc.name = name;
	return desc;
}

// A variant saved by a device with mesh shaders keeps its description on one without (it is saved again as it was
// loaded), but is never created there.
static bool can_create(HAL::Device& device, const HAL::PipelineStateDesc& desc)
{
	return !((desc.mesh || desc.amplification) && !device.is_mesh_shader_supported());
}

PSOVariant::PSOVariant(HAL::Device& device, std::optional<HAL::PipelineStateDesc> desc) : device(device), desc(std::move(desc))
{
	if (!this->desc || !can_create(device, *this->desc))
		status.store(Status::Unavailable, std::memory_order_release);
}

void PSOVariant::create()
{
	HAL::PipelineState::ptr result;
	{
		PROFILE(L"pso_variant_create");
		try
		{
			auto d = *desc;
			result = HAL::PipelineState::create(device, d, d.name);
		}
		catch (const std::exception& e)
		{
			Log::get() << Log::LEVEL_ERROR << "PSO variant " << desc->name << " creation threw: " << e.what() << Log::endl;
		}
	}
	pso = result;
	status.store(result ? Status::Created : Status::Unavailable, std::memory_order_release);
	// After the status: a consumer that saw the old count and then found the variant not created is told to redo.
	created_count().fetch_add(1, std::memory_order_acq_rel);
}

bool PSOVariant::start()
{
	auto expected = Status::Idle;
	if (!status.compare_exchange_strong(expected, Status::Queued, std::memory_order_acq_rel))
		return false;

	thread_pool::get().enqueue_detached(TaskOptions{ TaskPriority::LOW, TaskLane::Asset }, [self = shared_from_this()]()
	{
		auto queued = Status::Queued;
		if (self->status.compare_exchange_strong(queued, Status::Creating, std::memory_order_acq_rel))
			self->create();
	});
	return true;
}

void PSOVariant::create_now()
{
	while (true)
	{
		auto s = status.load(std::memory_order_acquire);
		if (s == Status::Idle || s == Status::Queued)
		{
			// A queued task is not waited for: it may sit behind this very thread's lane. Whoever wins the exchange
			// creates it; the queued task then finds nothing to do.
			if (status.compare_exchange_weak(s, Status::Creating, std::memory_order_acq_rel))
			{
				create();
				return;
			}
		}
		else if (s == Status::Creating)
			std::this_thread::yield();
		else
			return;
	}
}

HAL::PipelineState::ptr PSOVariant::get_built() const
{
	if (status.load(std::memory_order_acquire) != Status::Created)
		return nullptr;
	return pso->is_built() ? pso : nullptr;
}

bool PSOVariant::settled() const
{
	switch (status.load(std::memory_order_acquire))
	{
	case Status::Created:     return pso->is_built();
	case Status::Unavailable: return true;
	default:                  return false;
	}
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
