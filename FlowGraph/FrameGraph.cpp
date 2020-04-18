
#include "pch.h"

	void TaskBuilder::begin(Pass* pass)
	{
		current_pass = pass;
	}

	void TaskBuilder::end(Pass* pass)
	{
		current_pass = nullptr;
	}


	void TaskBuilder::pass_texture(std::string name, Render::TextureView tex)
	{
		ResourceHandler& handler = resources[name];
		ResourceAllocInfo& info = alloc_resources[&handler];
		info.texture = tex;
		info.placed = false;
		info.type = ResourceType::Texture;
		info.resource = tex.resource;
		info.passed = true;
		handler.info = &info;

		passed_resources.insert(&handler);
	}
	void TaskBuilder::pass_texture(std::string name, Render::Texture::ptr tex)
	{
		pass_texture(name, tex->create_view<Render::TextureView>(frames.get_creator()));
	}

	void TaskBuilder::reset()
	{
		//resources.clear();
		//alloc_resources.clear();
		current_pass = nullptr;
		allocator.reset();

		for (auto p : passed_resources)
		{
			p->info->resource = nullptr;
			p->info->buffer = Render::BufferView();
			p->info->texture = Render::TextureView();
			p->info->passed = false;

		}
		passed_resources.clear();
	}



	Render::TextureView FrameContext::get_texture(ResourceHandler* handler)
	{
		return handler->info->texture;// textures[holder];
	}

	Render::BufferView FrameContext::get_buffer(ResourceHandler* handler)
	{
		return handler->info->buffer;// textures[holder];
	}

	/*void FrameContext::request_resources(UsedResources& resources, TaskBuilder& builder)
	{
		for (auto& uses : resources.textures)
		{
			textures[uses] = builder.request_texture(uses);
		}
	}
	*/

	Render::CommandList::ptr& FrameContext::get_list()
	{
		return list;
	}

	void FrameContext::begin(Pass * pass, Render::FrameResourceManager&mgr ) {
		list = Render::Device::get().get_queue(Render::CommandListType::DIRECT)->get_free_list();
		list->begin(pass->name);
//		list->set_heap(Render::DescriptorHeapType::SAMPLER, Render::DescriptorHeapManager::get().gpu_smp);

		for (auto handle : pass->used.resources)
		{
			if (!handle->info->placed) continue;
			//if (!handle->info->texture) continue;

			auto &tex = get_texture(handle);
			
			list->transition(nullptr, tex.resource.get());
		}


		mgr.propagate_frame(list);
	}

	void FrameContext::end()
	{
		list->end();
	}


	void FrameContext::execute()
	{
		list->execute();
		list = nullptr;
	}


	void Pass::compile(TaskBuilder& builder)
	{
	//	builder.begin(this);
	//	context.request_resources(used, builder);
	//	builder.end(this);
	}

	void Pass::wait()
	{
		//if(render_task)
		render_task.wait();
	}
	void Pass::execute()
		
	{
		auto& timer = Profiler::get().start(L"wait"); {
			render_task.wait();
		}

		{	auto& timer = Profiler::get().start(L"execute");
		context.execute();
		}
	}
	void FrameGraph::start_new_frame()
	{
		{
			auto& timer = Profiler::get().start(L"begin_frame");

			builder.frames.begin_frame();
		}
	}

	void FrameGraph::setup()
	{
		auto& timer = Profiler::get().start(L"FrameGraph::setup");

		for (auto& pass : passes)
			pass->setup(builder);
	}

	void FrameGraph::compile(int frame)
	{
		auto& timer = Profiler::get().start(L"FrameGraph::compile");
		builder.allocator.begin_frame(frame);
		builder.create_resources();
	
		for (auto& pass : passes)
			pass->compile(builder);
	}

	void FrameGraph::render()
	{
		auto& timer = Profiler::get().start(L"FrameGraph::render");


		{
			auto& timer = Profiler::get().start(L"passes");

			for (auto& pass : passes)
				pass->render(builder.frames);

		}

		{
			auto& timer = Profiler::get().start(L"execute");

			for (auto& pass : passes)
			{
				pass->execute();
			}


		}

	{	auto& timer = Profiler::get().start(L"end_frame");

	builder.frames.end_frame();
	}
	}

	void FrameGraph::reset()
	{

		for (auto& pass : passes)
		{
			pass->wait();
		}
		passes.clear();
		builder.reset();
		
	}

ResourceHandler* TaskBuilder::create_texture(std::string name, ivec2 size, UINT array_count, DXGI_FORMAT format, UINT flags)
{
	ResourceHandler& handler = resources[name];
	ResourceAllocInfo& info = alloc_resources[&handler];
	handler.info = &info;



	auto desc = TextureDesc{ ivec3(size, 1), format, array_count };

	info.need_recreate = info.desc != desc;
	info.type = ResourceType::Texture;
	info.desc = desc;
	info.flags = flags;
	current_pass->used.resources.push_back(&handler);	
	info.handler = &handler;

	info.valid_from = current_pass->id;
	info.valid_to = current_pass->id;

	return &handler;
}


ResourceHandler* TaskBuilder::need_texture(std::string name, UINT flags)
{
	ResourceHandler& handler = resources[name];
	current_pass->used.resources.push_back(&handler);


	ResourceAllocInfo& info = *handler.info;

	info.flags |= flags;
	info.handler = &handler;

	info.valid_to = std::max(info.valid_to, current_pass->id);
	return &handler;
}


ResourceHandler* TaskBuilder::create_buffer(std::string name, UINT64 size, UINT flags)
{
	ResourceHandler& handler = resources[name];
	ResourceAllocInfo& info = alloc_resources[&handler];
	handler.info = &info;
	auto desc = BufferDesc{ size };

	info.need_recreate = info.desc != desc;
	info.type = ResourceType::Buffer;
	info.desc = desc;
	info.flags = flags;
	current_pass->used.resources.push_back(&handler);

	info.handler = &handler;
	info.valid_from = current_pass->id;
	info.valid_to = current_pass->id;

	return &handler;
}


ResourceHandler* TaskBuilder::need_buffer(std::string name, UINT flags)
{
	ResourceHandler& handler = resources[name];
	current_pass->used.resources.push_back(&handler);


	ResourceAllocInfo& info = *handler.info;

	info.flags |= flags;
	info.handler = &handler;

	info.valid_to = std::max(info.valid_to, current_pass->id);
	return &handler;
}



void TaskBuilder::create_resources()
{
	
	struct Events
	{
		std::set<ResourceAllocInfo*> create;
		std::set<ResourceAllocInfo*> free;
	};

	std::map<size_t, Events> events;

	for (auto& pair : alloc_resources)
	{
		if (pair.second.passed) continue;
		events[pair.second.valid_from].create.insert(&pair.second);
		events[pair.second.valid_to+1].free.insert(&pair.second);
	}

	{
		auto& timer = Profiler::get().start(L"allocate memory");

	for (auto [id, e] : events)
	{
		for (auto info : e.free)
		{
			//auto& timer = Profiler::get().start(L"free");
			//if (info->type == ResourceType::Texture)
			info->alloc_ptr.Free();
		}


		for (auto info : e.create)
		{

			CommonAllocator::Handle alloc_ptr;
			Render::HeapType heap_type = Render::HeapType::DEFAULT;

			if (info->flags & ResourceFlags::GenCPU)
			{
				heap_type = Render::HeapType::UPLOAD;
			}

			if (info->flags & ResourceFlags::ReadCPU)
			{
				heap_type = Render::HeapType::READBACK;
			}

			if (info->type == ResourceType::Texture)
			{
				//auto& timer = Profiler::get().start(L"create");

				TextureDesc desc = info->desc.get<TextureDesc>();

				D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE;

				if (info->flags & ResourceFlags::RenderTarget)
				{
					flags |= D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
				}

				if (info->flags & ResourceFlags::DepthStencil)
				{
					flags |= D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
				}

				if (info->flags & ResourceFlags::UnorderedAccess)
				{
					flags |= D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
				}

				alloc_ptr = allocator.allocate_resource(CD3DX12_RESOURCE_DESC::Tex2D(desc.format, desc.size.x, desc.size.y, desc.array_count, 1, 1, 0, flags), heap_type);

			}

			if (info->type == ResourceType::Buffer)
			{
				BufferDesc desc = info->desc.get<BufferDesc>();
				alloc_ptr = allocator.allocate_resource(CD3DX12_RESOURCE_DESC::Buffer(desc.size), heap_type);
			}
			if (info->alloc_ptr != alloc_ptr) {
			info->alloc_ptr = alloc_ptr;
			info->need_recreate = true;
		}

		}
	}
	}

	{
		auto& timer = Profiler::get().start(L"allocate heaps");

		allocator.allocate_heaps();

	}

	{
		auto& timer = Profiler::get().start(L"create resources");
		int id = 0;

		for (auto& pair : alloc_resources)
		{
			auto info = &pair.second;

			if (info->passed) continue;
	//		if (!info->need_recreate) 
	//			continue;
	if (info->type == ResourceType::Texture)
			{
		if (info->need_recreate) {
				TextureDesc desc = info->desc.get<TextureDesc>();

				D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE;

				if (info->flags & ResourceFlags::RenderTarget)
				{
					flags |= D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
				}

				if (info->flags & ResourceFlags::DepthStencil)
				{
					flags |= D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
				}

				if (info->flags & ResourceFlags::UnorderedAccess)
				{
					flags |= D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
				}

			
					info->resource = allocator.create_resource(CD3DX12_RESOURCE_DESC::Tex2D(desc.format, desc.size.x, desc.size.y, desc.array_count, 1, 1, 0, flags), info->alloc_ptr, Render::ResourceState::UNKNOWN, vec4());
					info->resource->set_name(std::string("Graph Texture:") + std::to_string(id));
				}

				info->texture = info->resource->create_view<Render::TextureView>(frames.get_creator());

				
			}

			if (info->type == ResourceType::Buffer)
			{
		
				if (info->need_recreate) {
					BufferDesc desc = info->desc.get<BufferDesc>();
					info->resource = allocator.create_resource(CD3DX12_RESOURCE_DESC::Buffer(desc.size, D3D12_RESOURCE_FLAG_NONE), info->alloc_ptr, Render::ResourceState::GEN_READ, vec4());
					info->resource->set_name(std::string("Graph Buffer:") + std::to_string(id));
				}

				info->buffer = info->resource->create_view<Render::BufferView>(frames.get_creator());
			}
			id++;
		}
	}


}


Render::TextureView TaskBuilder::request_texture(ResourceHandler* handler)
{
	return handler->info->texture;
}