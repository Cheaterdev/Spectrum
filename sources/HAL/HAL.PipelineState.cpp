module HAL:PipelineState;
import Core;
import :Device;
import :Shader;

namespace HAL
{

	PipelineState::ptr PipelineState::create(PipelineStateDesc& desc, std::string name)
	{
		return PipelineStateCache::get_cache(desc, name);
	}

	ComputePipelineState::ptr ComputePipelineState::create(ComputePipelineStateDesc& desc, std::string name)
	{
		return PipelineStateCache::get_cache(desc, name);
	}


	PipelineState::PipelineState(PipelineStateDesc _desc, std::string cache) : desc(_desc)
	{
		this->cache = cache;
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

		FileDataStorage storage(L"cache/pso.bin");
		storage.start_save();
		storage.put("cache", binary_cache);
		storage.save();
	}

	PipelineStateCache::PipelineStateCache(Device& device) :device(device), cache([this](const PipelineStateDesc& desc)
		{
			std::string binary;
			{
				std::lock_guard<std::mutex> g(m);
				binary = desc.name.empty() ? "" : binary_cache[desc.name];
			}

			//Log::get() << desc << Log::endl;
			auto state = PipelineState::ptr(new PipelineState(desc, binary));

			if (!desc.name.empty())
			{
				std::lock_guard<std::mutex> g(m);
				binary_cache[desc.name] = state->get_cache();
			}

			return state;

		}), compute_cache([this](const ComputePipelineStateDesc& desc)
			{
				std::string binary;
				{
					std::lock_guard<std::mutex> g(m);
					binary = desc.name.empty() ? "" : binary_cache[desc.name];
				}

				//Log::get() << desc << Log::endl;
				auto state = ComputePipelineState::ptr(new ComputePipelineState(desc, binary));

				if (!desc.name.empty())
				{
					std::lock_guard<std::mutex> g(m);
					binary_cache[desc.name] = state->get_cache();
				}

				return state;

			})
	{

		std::lock_guard<std::mutex> g(m);


		
		FileDataStorage storage(L"cache/pso.bin");
		storage.get("cache",binary_cache);
	}

			PipelineState::ptr PipelineStateCache::get_cache(PipelineStateDesc& desc, std::string name)
			{

				desc.name = name;

				//	 return  PipelineState::ptr(new PipelineState(desc));
				return desc.root_signature->get_device().get_pipeline_state_cache().cache[desc];
			}

			ComputePipelineState::ptr PipelineStateCache::get_cache(ComputePipelineStateDesc& desc, std::string name)
			{
				desc.name = name;
				return desc.root_signature->get_device().get_pipeline_state_cache().compute_cache[desc];
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

	ComputePipelineState::ComputePipelineState(const ComputePipelineStateDesc& _desc, std::string cache) : desc(_desc)
	{
		this->cache = cache;
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