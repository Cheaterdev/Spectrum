module HAL:PipelineState;
import Core;
import :Device;
import :Shader;
import :PSO;
import :Utils;   // get_backend_name() for per-backend PSO cache

namespace HAL
{

	PipelineState::ptr PipelineState::create(Device& device, PipelineStateDesc& desc, std::string name)
	{
		return device.get_pipeline_state_cache().get_cache(desc, name);
	}

	ComputePipelineState::ptr ComputePipelineState::create(Device& device, ComputePipelineStateDesc& desc, std::string name)
	{
		return device.get_pipeline_state_cache().get_cache(desc, name);
	}


	PipelineState::PipelineState(Device& device, PipelineStateDesc _desc, std::string cache) : desc(_desc)
	{
		this->cache = cache;
		root_signature = device.get_engine_root_layout_holder().GetSignature(desc.layout);
		on_change();
		register_shader(desc.pixel);
		register_shader(desc.vertex);
		register_shader(desc.hull);
		register_shader(desc.domain);
		register_shader(desc.geometry);

		register_shader(desc.mesh);
		register_shader(desc.amplification);

	}


	PipelineStateCache::~PipelineStateCache()
	{
		std::lock_guard<std::mutex> g(m);

		FileDataStorage storage(std::filesystem::path("cache") / get_backend_name() / "pso.bin");
		storage.start_save();
		storage.put("cache", binary_cache);
		storage.save();
	}

	// What a driver blob is stored under: the name plus the hashes of the shader bytecode. The name alone is not enough:
	// the key variants of one PSO set share it, and a material pipeline's name ends in an id that is handed out per
	// run, so by name a blob was usually another pipeline's, which the driver rejects (an internal _com_error) before
	// the pipeline is compiled from scratch. A wrong match here still only costs that fallback.
	template<class... Shaders>
	static std::string blob_key(const std::string& name, const Shaders&... shaders)
	{
		if (name.empty())
			return {};

		std::string key = name;
		auto add = [&](const auto& shader)
		{
			key += '#';
			if (shader)
				key += std::to_string(shader->get_hash());
		};
		(add(shaders), ...);
		return key;
	}

	static std::string blob_key(const PipelineStateDesc& desc)
	{
		std::string key = blob_key(desc.name, desc.vertex, desc.pixel, desc.geometry, desc.hull, desc.domain, desc.mesh, desc.amplification);
		if (!key.empty())
			key += '#' + std::to_string(static_cast<int>(desc.topology));
		return key;
	}

	PipelineStateCache::PipelineStateCache(Device& device) :device(device), cache([this](const PipelineStateDesc& desc)
		{
			const std::string key = blob_key(desc);
			std::string binary;
			{
				std::lock_guard<std::mutex> g(m);
				binary = key.empty() ? "" : binary_cache[key];
			}

			//Log::get() << desc << Log::endl;
			PipelineState::ptr state;
			{
				PROFILE(L"pso_create_graphics");
				state = PipelineState::ptr(new PipelineState(this->device, desc, binary));
			}

			if (!key.empty())
			{
				std::lock_guard<std::mutex> g(m);
				binary_cache[key] = state->get_cache();
			}

			return state;

		}), compute_cache([this](const ComputePipelineStateDesc& desc)
			{
				const std::string key = blob_key(desc.name, desc.shader);
				std::string binary;
				{
					std::lock_guard<std::mutex> g(m);
					binary = key.empty() ? "" : binary_cache[key];
				}

				//Log::get() << desc << Log::endl;
				ComputePipelineState::ptr state;
				{
					PROFILE(L"pso_create_compute");
					state = ComputePipelineState::ptr(new ComputePipelineState(this->device, desc, binary));
				}

				if (!key.empty())
				{
					std::lock_guard<std::mutex> g(m);
					binary_cache[key] = state->get_cache();
				}

				return state;

			})
	{

		std::lock_guard<std::mutex> g(m);



		FileDataStorage storage(std::filesystem::path("cache") / get_backend_name() / "pso.bin");
		storage.get("cache",binary_cache);

		// Entries from before blob_key(): stored by name alone, never looked up again.
		std::erase_if(binary_cache, [](const auto& entry) { return entry.first.find('#') == std::string::npos; });
	}

	// A loaded description's shaders resolve on first access, and comparing them does that: resolve before the cache
	// lookup, or the shader loads run inside the cache's lock.
	static void resolve(const auto& shader)
	{
		if (shader)
			(void)shader.operator->();
	}

	PipelineState::ptr PipelineStateCache::get_cache(PipelineStateDesc& desc, std::string name)
	{
		desc.name = name;
		resolve(desc.vertex);
		resolve(desc.pixel);
		resolve(desc.geometry);
		resolve(desc.domain);
		resolve(desc.hull);
		resolve(desc.mesh);
		resolve(desc.amplification);
		return cache[desc];
	}

	ComputePipelineState::ptr PipelineStateCache::get_cache(ComputePipelineStateDesc& desc, std::string name)
	{
		desc.name = name;
		resolve(desc.shader);
		return compute_cache[desc];
	}

	PipelineStateDesc::PipelineStateDesc()
	{
		rtv.rtv_formats.emplace_back(Format::R8G8B8A8_UNORM);
		rasterizer.cull_mode = CullMode::Back;
		rasterizer.fill_mode = FillMode::Solid;
		topology = HAL::PrimitiveTopologyType::TRIANGLE;
	}

	bool PipelineStateDesc::is_memory()
	{
		if (vertex && vertex->get_header().file_name.empty()) return true;
		if (pixel && pixel->get_header().file_name.empty()) return true;
		if (geometry && geometry->get_header().file_name.empty()) return true;
		if (hull && hull->get_header().file_name.empty()) return true;
		if (domain && domain->get_header().file_name.empty()) return true;
		if (mesh && mesh->get_header().file_name.empty()) return true;
		if (amplification && amplification->get_header().file_name.empty()) return true;
		return false;
	}

	ComputePipelineState::ComputePipelineState(Device& device, const ComputePipelineStateDesc& _desc, std::string cache) : desc(_desc)
	{
		this->cache = cache;
		root_signature = device.get_engine_root_layout_holder().GetSignature(desc.layout);
		on_change();
		register_shader(desc.shader);
	}

	void LibraryObject::export_shader(std::wstring name, std::wstring as)
	{
		exports[name] = as;
	}

	HAL::shader_identifier StateObject::identify(void* data)
	{
		HAL::shader_identifier result;
		std::memcpy(result.data(), data, result.size());
		return result;
	}

	StateObjectDesc StateObject::extended(const StateObjectDesc& base, const std::list<std::shared_ptr<StateObject>>& added)
	{
		StateObjectDesc result = base;
		result.collections.insert(result.collections.end(), added.begin(), added.end());
		return result;
	}

	StateObject::StateObject(std::shared_ptr<StateObject> parent, std::list<std::shared_ptr<StateObject>> added)
		// Members are initialized in declaration order: base and added_collections, then desc from them.
		: base(std::move(parent)), added_collections(std::move(added)), desc(extended(base->desc, added_collections))
	{
		for (auto& l : desc.libraries)
		{
			register_shader(l.library);
		}
		for (auto& c : desc.collections)
		{
			c->event_change.register_handler(this, [this]()
				{
					on_change();
				});
		}
		on_change();

		// Later rebuilds (a collection or shader changed) build everything from `desc`.
		base.reset();
		added_collections.clear();
	}

	StateObject::ptr StateObject::add(const ptr& base, const std::list<ptr>& collections)
	{
		return ptr(new StateObject(base, collections));
	}

	StateObject::StateObject(StateObjectDesc& _desc) : desc(_desc)
	{
		for (auto& l : desc.libraries)
		{
			register_shader(l.library);
		}
		for (auto& c : desc.collections)
		{
			c->event_change.register_handler(this, [this]()
				{
					on_change();
				});
		}
		on_change();
	}

}