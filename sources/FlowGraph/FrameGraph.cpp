
#include "pch.h"

void TaskBuilder::begin(Pass* pass)
{
	current_pass = pass;
}

void TaskBuilder::end(Pass* pass)
{
	current_pass = nullptr;
}


void TaskBuilder::pass_texture(std::string name, Render::TextureView tex, ResourceFlags flags)
{
	resources_names[name] = name;

	ResourceHandler& handler = resources[name];
	ResourceAllocInfo& info = alloc_resources[&handler];
	info.texture = tex;
	info.placed = false;
	info.type = ResourceType::Texture;
	info.resource = tex.resource;
	info.passed = true;
	info.name = name;
	info.is_new = false;
	info.flags = flags;
	handler.info = &info;

	info.writers.clear();
	passed_resources.insert(&handler);
}
void TaskBuilder::pass_texture(std::string name, Render::Texture::ptr tex, ResourceFlags flags)
{
	pass_texture(name, tex->create_view<Render::TextureView>(*current_frame), flags);
}

void TaskBuilder::reset()
{


	current_pass = nullptr;
	//	allocator.reset();

	for (auto p : passed_resources)
	{
		p->info->resource = nullptr;
		p->info->buffer = Render::BufferView();
		p->info->texture = Render::TextureView();
		p->info->passed = false;

	}
	passed_resources.clear();
	//resources.clear();
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

void FrameContext::begin(Pass* pass, Render::FrameResources::ptr& frame) {
	list = frame->start_list(pass->name);
	//		list->set_heap(Render::DescriptorHeapType::SAMPLER, Render::DescriptorHeapManager::get().gpu_smp);

	if(!GetAsyncKeyState(VK_F7))
	for (auto handle : pass->used.resource_creations)
	{
		if (!handle->info->alloc_ptr.handle) continue;

		if (!handle->info->enabled)
			continue;
		//if (!handle->info->texture) continue;
		{
			auto& tex = get_texture(handle);

			if (tex)
				list->transition(nullptr, tex.resource.get());

		}
		{
		auto& tex = get_buffer(handle);

		if (tex)
			list->transition(nullptr, tex.resource.get());
		}

	}
	/*
	list->flush_transitions();


	for (auto handle : pass->used.resources)
	{
		if (!handle->info->placed) continue;
		//if (!handle->info->texture) continue;

		auto& tex = get_texture(handle);

		if (tex)
		{
			if (handle->info->flags & ResourceFlags::DepthStencil > 0)
			{

				list->transition(tex.resource, Render::ResourceState::PIXEL_SHADER_RESOURCE);
			}

			if (handle->info->flags & ResourceFlags::RenderTarget > 0)
			{

				list->transition(tex.resource, Render::ResourceState::PIXEL_SHADER_RESOURCE);
			}

		}
	}
	*/

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
	if (!enabled)  return;

	render_task.wait();
}
void Pass::execute()

{
	if (!enabled) {
		return;

	}
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

		builder.current_frame = builder.frames.begin_frame();
	}
}

void FrameGraph::setup()
{
	auto& timer = Profiler::get().start(L"FrameGraph::setup");

	for (auto& pass : passes)
	{
		pass->enabled = false;

		pass->setup(builder);
	}

	for (auto& pair : builder.alloc_resources)
	{
		if (pair.second.passed) continue;

		// here need to delete unused info
		if (pair.second.frame_id != builder.current_frame->get_frame())
			continue;

		auto info = &pair.second;

		info->enabled = false;

	}

	std::function<void(ResourceHandler*, int)> process_resource;

	process_resource = [&, this](ResourceHandler* handler, int pass_id) {

		handler->info->enabled = true;
		ResourceAllocInfo& info = *handler->info;
		for (auto writer : info.writers)
		{
			if (writer->id > pass_id)
				continue;
			if (writer->enabled) continue;
			writer->enabled = true;

			for (auto res : writer->used.resources)
			{
				process_resource(res, writer->id);
			}
		}

	};

	for (auto res : builder.resources)
	{
		if (check(res.second.info->flags & ResourceFlags::Required))
			process_resource(&res.second, passes.size());
	}


	for (auto pass : required_passes)
	{
		pass->enabled = true;
		for (auto res : pass->used.resources)
		{
			process_resource(res, pass->id);
		}

	}

}

void FrameGraph::compile(int frame)
{
	auto& timer = Profiler::get().start(L"FrameGraph::compile");
	//builder.allocator.begin_frame(frame);

	builder.current_alloc = &builder.frame_allocs[frame];


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
			pass->render(builder.current_frame);

	}

	{
		auto& timer = Profiler::get().start(L"execute");

		for (auto& pass : passes)
		{
			pass->execute();
		}


	}
	builder.current_frame = nullptr;
}

void FrameGraph::reset()
{

	for (auto& pass : passes)
	{
		pass->wait();
	}

	passes.clear();
	required_passes.clear();
	builder.reset();

	for (auto& pair : builder.alloc_resources)
	{
		auto info = &pair.second;

		if (info->heap_type == Render::HeapType::UPLOAD)
		{
			info->resource = nullptr;

			info->texture = Render::TextureView();

			info->buffer = Render::BufferView();
		}
	}

}

ResourceHandler* TaskBuilder::create_texture(std::string name, ivec2 size, UINT array_count, DXGI_FORMAT format, ResourceFlags flags)
{
	resources_names[name] = name;

	ResourceHandler& handler = resources[name];
	ResourceAllocInfo& info = alloc_resources[&handler];
	handler.info = &info;



	auto desc = TextureDesc{ ivec3(size, 1), format, array_count };
	info.is_new = false;
	info.need_recreate = info.desc != desc;
	info.type = ResourceType::Texture;
	info.desc = desc;
	info.flags = flags;/// | ResourceFlags::Static;
	current_pass->used.resources.insert(&handler);
	current_pass->used.resource_creations.insert(&handler);
	current_pass->used.resource_flags[&handler] = flags;

	info.handler = &handler;
	info.name = name;
	info.valid_from = current_pass->id;
	info.valid_to = current_pass->id;
	info.frame_id = current_frame->get_frame();

	info.writers.clear();
	info.writers.push_back(current_pass);
	return &handler;
}


ResourceHandler* TaskBuilder::recreate_texture(std::string name, ResourceFlags flags)
{

	ResourceHandler& old_handler = resources[name];

	std::string new_name = resources_names[name] + "recreated";
	resources_names[name] = new_name;

	name = new_name;

	ResourceHandler& handler = resources[name];
	ResourceAllocInfo& info = alloc_resources[&handler];
	handler.info = &info;

	auto desc = old_handler.info->desc;
	info.is_new = false;
	info.need_recreate = info.desc != desc;
	info.type = ResourceType::Texture;
	info.desc = desc;
	info.flags = flags;// | ResourceFlags::Static;
	current_pass->used.resources.insert(&handler);
	current_pass->used.resource_creations.insert(&handler);
	current_pass->used.resource_flags[&handler] = flags;
	info.handler = &handler;
	info.name = name;
	info.valid_from = current_pass->id;
	info.valid_to = current_pass->id;
	info.frame_id = current_frame->get_frame();

	info.writers = old_handler.info->writers;

	info.writers.push_back(current_pass);

	info.orig = old_handler.info;

	return &handler;
}
ResourceHandler* TaskBuilder::need_texture(std::string name, ResourceFlags flags)
{
	name = resources_names[name];

	ResourceHandler& handler = resources[name];
	current_pass->used.resources.insert(&handler);
	current_pass->used.resource_flags[&handler] = flags;

	ResourceAllocInfo& info = *handler.info;
	info.is_new = false;
	info.flags = info.flags | flags;
	info.handler = &handler;

	info.valid_to = std::max(info.valid_to, current_pass->id);


	if (check(flags & WRITEABLE_FLAGS))
	{
		info.writers.push_back(current_pass);
	}
	return &handler;
}


ResourceHandler* TaskBuilder::create_buffer(std::string name, UINT64 size, ResourceFlags flags)
{
	resources_names[name] = name;

	size = Math::AlignUp(size, 256); // TODO: make in GAPI
	ResourceHandler& handler = resources[name];
	ResourceAllocInfo& info = alloc_resources[&handler];
	handler.info = &info;
	auto desc = BufferDesc{ size };

	info.need_recreate = info.desc != desc;
	info.type = ResourceType::Buffer;
	info.desc = desc;
	info.flags = flags;
	current_pass->used.resources.insert(&handler);
	current_pass->used.resource_creations.insert(&handler);
	current_pass->used.resource_flags[&handler] = flags;
	info.name = name;
	info.handler = &handler;
	info.valid_from = current_pass->id;
	info.valid_to = current_pass->id;
	info.frame_id = current_frame->get_frame();
	info.is_new = false;
	info.writers.clear();
	info.writers.push_back(current_pass);

	return &handler;
}


ResourceHandler* TaskBuilder::need_buffer(std::string name, ResourceFlags flags)
{
	name = resources_names[name];

	ResourceHandler& handler = resources[name];
	current_pass->used.resources.insert(&handler);
	current_pass->used.resource_flags[&handler] = flags;

	ResourceAllocInfo& info = *handler.info;

	info.flags = info.flags | flags;
	info.handler = &handler;
	info.is_new = false;
	info.valid_to = std::max(info.valid_to, current_pass->id);

	if (check(flags & WRITEABLE_FLAGS))
	{
		info.writers.push_back(current_pass);
	}

	return &handler;
}



void TaskBuilder::create_resources()
{

	struct Events
	{
		std::set<ResourceAllocInfo*> create;
		std::set<ResourceAllocInfo*> free;
	};

	std::map<int, Events> events;

	for (auto& pair : alloc_resources)
	{
		if (pair.second.passed) continue;

		// here need to delete unused info
		if (pair.second.frame_id != current_frame->get_frame())
			continue;
		auto info = &pair.second;
		if (!info->enabled)
			continue;

		info->heap_type = Render::HeapType::DEFAULT;

		if (check(info->flags & ResourceFlags::GenCPU))
		{
			info->heap_type = Render::HeapType::UPLOAD;
		}

		if (check(info->flags & ResourceFlags::ReadCPU))
		{
			info->heap_type = Render::HeapType::READBACK;
		}

		if (info->type == ResourceType::Texture)
		{
			//auto& timer = Profiler::get().start(L"create");

			TextureDesc desc = info->desc.get<TextureDesc>();

			D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE;

			if (check(info->flags & ResourceFlags::RenderTarget))
			{
				flags |= D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
			}

			if (check(info->flags & ResourceFlags::DepthStencil))
			{
				flags |= D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
			}

			if (check(info->flags & ResourceFlags::UnorderedAccess))
			{
				flags |= D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			}

			if (!is_shader_visible(desc.format))
				flags |= D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;

			int mip_count = 1;
			auto tsize = desc.size;

			while (tsize.x != 1 && tsize.y != 1)
			{
				tsize /= 2;
				mip_count++;
			}
			info->d3ddesc = CD3DX12_RESOURCE_DESC::Tex2D(desc.format, desc.size.x, desc.size.y, desc.array_count, mip_count, 1, 0, flags);
			info->placed = true;

			Render::Device::get().get_alloc_info(info->d3ddesc);
		}

		if (info->type == ResourceType::Buffer)
		{
			BufferDesc desc = info->desc.get<BufferDesc>();

			D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE;

			if (check(info->flags & ResourceFlags::UnorderedAccess))
			{
				flags |= D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			}

			info->d3ddesc = CD3DX12_RESOURCE_DESC::Buffer(desc.size, flags);
		}

		if (info->heap_type != Render::HeapType::DEFAULT || check(info->flags & ResourceFlags::Static)) continue;


		events[pair.second.valid_from].create.insert(info);
		events[pair.second.valid_to + 1].free.insert(info);
	}

	{
		auto& timer = Profiler::get().start(L"allocate memory");

		for (auto [id, e] : events)
		{
			for (auto info : e.free)
			{
				info->alloc_ptr.handle.Free();
			}


			for (auto info : e.create)
			{
	
				auto creation_info = Render::Device::get().get_alloc_info(info->d3ddesc);
				auto alloc_ptr = allocator.alloc(creation_info.size, creation_info.alignment, creation_info.flags, info->heap_type);

				info->need_recreate = info->alloc_ptr != alloc_ptr;
				info->alloc_ptr = alloc_ptr;

			}
		}
	}

	{
		auto& timer = Profiler::get().start(L"create resources");
		int id = 0;

		for (auto& pair : alloc_resources)
		{
			auto info = &pair.second;

			// here need to delete unused info
			if (pair.second.frame_id != current_frame->get_frame())
				continue;

			if (info->passed) continue;
			if (!info->enabled)
				continue;


			if (info->alloc_ptr.handle)
			{
				auto& res = info->resource_places[info->alloc_ptr];


				if (!res || res->get_desc() != info->d3ddesc)
				{
					res = std::make_shared<Render::Resource>(info->d3ddesc, info->alloc_ptr);
					res->set_name(info->name);
				}

				if (info->resource != res)
				{
					info->is_new = true;
				}
				info->resource = res;// std::make_shared<Render::Resource>(info->d3ddesc, info->alloc_ptr);

				assert(res->tmp_handle == info->alloc_ptr);
			}
			else
			{
				if (info->heap_type == Render::HeapType::UPLOAD)
				{
					info->resource = std::make_shared<Render::Resource>(info->d3ddesc, Render::UploadAllocator::get());
				}
				else if (info->heap_type == Render::HeapType::READBACK)
				{
					info->resource = std::make_shared<Render::Resource>(info->d3ddesc, Render::ReadbackAllocator::get());
				}
				else 	if (!info->resource || info->resource->get_desc() != info->d3ddesc)
				{
					info->resource = std::make_shared<Render::Resource>(info->d3ddesc, Render::DefaultAllocator::get());
					info->is_new = true;
				}


				info->resource->set_name(info->name);

			}

			//		if (!info->need_recreate) 
			//			continue;
			if (info->type == ResourceType::Texture)
			{
				info->texture = info->resource->create_view<Render::TextureView>(*current_frame, check(info->flags & ResourceFlags::Cube));
			}

			if (info->type == ResourceType::Buffer)
			{
				info->buffer = info->resource->create_view<Render::BufferView>(*current_frame);
			}

			id++;
		}
	}


}


Render::TextureView TaskBuilder::request_texture(ResourceHandler* handler)
{
	return handler->info->texture;
}

bool ResourceHandler::is_new()
{
	return info->is_new;
}
