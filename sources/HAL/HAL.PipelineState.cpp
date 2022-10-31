module HAL:PipelineState;

import Serializer;
import Debug;
import :Device;

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

		FileSystem::get().save_data(L"pso", Serializer::serialize(binary_cache));
	}

	PipelineStateCache::PipelineStateCache() : cache([this](const PipelineStateDesc& desc)
		{
			std::lock_guard<std::mutex> g(m);

			std::string binary = desc.name.empty() ? "" : binary_cache[desc.name];


			//Log::get() << desc << Log::endl;
			auto state = PipelineState::ptr(new PipelineState(desc, binary));

			if (!desc.name.empty())
			{
				binary_cache[desc.name] = state->get_cache();
			}

			return state;

		}), compute_cache([this](const ComputePipelineStateDesc& desc)
			{
				std::lock_guard<std::mutex> g(m);

				std::string binary = desc.name.empty() ? "" : binary_cache[desc.name];


				//Log::get() << desc << Log::endl;
				auto state = ComputePipelineState::ptr(new ComputePipelineState(desc, binary));

				if (!desc.name.empty())
				{
					binary_cache[desc.name] = state->get_cache();
				}

				return state;

			})
	{

		std::lock_guard<std::mutex> g(m);

		auto file = FileSystem::get().get_file("pso");
		if (file)
			Serializer::deserialize(file->load_all(), binary_cache);

	}

			PipelineState::ptr PipelineStateCache::get_cache(PipelineStateDesc& desc, std::string name)
			{

				desc.name = name;

				//	 return  PipelineState::ptr(new PipelineState(desc));
				return Singleton<PipelineStateCache>::get().cache[desc];
			}

			ComputePipelineState::ptr PipelineStateCache::get_cache(ComputePipelineStateDesc& desc, std::string name)
			{
				desc.name = name;
				return Singleton<PipelineStateCache>::get().compute_cache[desc];
			}




}