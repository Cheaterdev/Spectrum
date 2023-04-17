module HAL:CommandListRecorder;


import :Autogen;

import :Buffer;
import :Device;
import Core;

import HAL;

namespace HAL
{
	void DelayedCommandList::create(CommandListType type)
	{
		list.create(type);
	}

	void DelayedCommandList::reset()
	{
		compiled = false;
	}

	void DelayedCommandList::compile(CommandAllocator& allocator)
	{
		list.begin(allocator);
		list.set_name(name);
		for (auto& f : tasks)
		{
			f(list);
		}

		list.end();
		tasks.clear();
		compiled = true;
	}

	void DelayedCommandList::func(std::function<void(API::CommandList&)> f)
	{
		tasks.emplace_back(f);
	}

	void DelayedCommandList::clear_uav(const UAVHandle& h, vec4 ClearColor)
	{
		tasks.emplace_back([=](API::CommandList& list) {
			list.clear_uav(h, ClearColor);
			});
	}

	void DelayedCommandList::clear_rtv(const RTVHandle& h, vec4 ClearColor)
	{
		tasks.emplace_back([=](API::CommandList& list) {
			list.clear_rtv(h, ClearColor);
			});
	}

	void DelayedCommandList::clear_stencil(const DSVHandle& dsv, UINT8 stencil)
	{
		tasks.emplace_back([=](API::CommandList& list) {
			list.clear_stencil(dsv, stencil);
			});
	}

	void DelayedCommandList::clear_depth(const DSVHandle& dsv, float depth)
	{
		tasks.emplace_back([=](API::CommandList& list) {
			list.clear_depth(dsv, depth);
			});
	}

	void DelayedCommandList::clear_depth_stencil(const DSVHandle& dsv, bool depth, bool stencil, float fdepth, UINT8 fstencil)
	{
		tasks.emplace_back([=](API::CommandList& list) {
			list.clear_depth_stencil(dsv, depth, stencil, fdepth, fstencil);
			});
	}

	void DelayedCommandList::set_topology(HAL::PrimitiveTopologyType topology, HAL::PrimitiveTopologyFeed feedType, bool adjusted, uint controlpoints)
	{
		tasks.emplace_back([=](API::CommandList& list) {
			list.set_topology(topology, feedType, adjusted, controlpoints);
			});
	}

	void DelayedCommandList::set_stencil_ref(UINT ref)
	{
		tasks.emplace_back([=](API::CommandList& list) {
			list.set_stencil_ref(ref);
			});
	}

	void DelayedCommandList::set_name(std::wstring_view name)
	{
		this->name=name;
	}

	void DelayedCommandList::set_descriptor_heaps(DescriptorHeap* cbv, DescriptorHeap* sampler)
	{
		tasks.emplace_back([=](API::CommandList& list) {
			list.set_descriptor_heaps(cbv, sampler);
			});
	}

	void DelayedCommandList::insert_time(const QueryHandle& handle, uint offset)
	{
		tasks.emplace_back([=](API::CommandList& list) {
			list.insert_time(handle, offset);
			});
	}

	void DelayedCommandList::resolve_times(const QueryHeap* pQueryHeap, uint32_t NumQueries, ResourceAddress destination)
	{
		tasks.emplace_back([=](API::CommandList& list) {
			list.resolve_times(pQueryHeap, NumQueries, destination);
			});
	}

	void DelayedCommandList::set_graphics_signature(const HAL::RootSignature::ptr& s)
	{
		tasks.emplace_back([=](API::CommandList& list) {
			list.set_graphics_signature(s);
			});
	}

	void DelayedCommandList::set_compute_signature(const HAL::RootSignature::ptr& s)
	{
		tasks.emplace_back([=](API::CommandList& list) {
			list.set_compute_signature(s);
			});
	}

	void DelayedCommandList::draw(UINT vertex_count, UINT vertex_offset, UINT instance_count, UINT instance_offset)
	{
		tasks.emplace_back([=](API::CommandList& list) {
			list.draw(vertex_count, vertex_offset, instance_count, instance_offset);
			});
	}

	void DelayedCommandList::draw_indexed(UINT index_count, UINT index_offset, UINT vertex_offset, UINT instance_count, UINT instance_offset)
	{
		tasks.emplace_back([=](API::CommandList& list) {
			list.draw_indexed(index_count, index_offset, vertex_offset, instance_count, instance_offset);
			});
	}

	void DelayedCommandList::set_index_buffer(HAL::Views::IndexBuffer index)
	{
		tasks.emplace_back([=](API::CommandList& list) {
			list.set_index_buffer(index);
			});
	}

	void  DelayedCommandList::graphics_set_const_buffer(UINT i, const ResourceAddress& adress)
	{
		tasks.emplace_back([=](API::CommandList& list) {
			list.graphics_set_const_buffer(i, adress);
			});
	}

	void  DelayedCommandList::compute_set_const_buffer(UINT i, const ResourceAddress& adress)
	{
		tasks.emplace_back([=](API::CommandList& list) {
			list.compute_set_const_buffer(i, adress);
			});
	}

	void  DelayedCommandList::graphics_set_constant(UINT i, UINT offset, UINT value)
	{
		tasks.emplace_back([=](API::CommandList& list) {
			list.graphics_set_constant(i, offset, value);
			});
	}

	void  DelayedCommandList::compute_set_constant(UINT i, UINT offset, UINT value)
	{
		tasks.emplace_back([=](API::CommandList& list) {
			list.compute_set_constant(i, offset, value);
			});
	}

	void DelayedCommandList::dispatch_mesh(ivec3 v)
	{
		tasks.emplace_back([=](API::CommandList& list) {
			list.dispatch_mesh(v);
			});
	}

	void DelayedCommandList::dispatch(ivec3 v)
	{
		tasks.emplace_back([=](API::CommandList& list) {
			list.dispatch(v);
			});
	}
	void DelayedCommandList::set_scissors(sizer_long rect)
	{
		tasks.emplace_back([=](API::CommandList& list) {
			list.set_scissors(rect);
			});
	}

	void DelayedCommandList::set_viewports(std::vector<Viewport> viewports)
	{
		tasks.emplace_back([=](API::CommandList& list) {
			list.set_viewports(viewports);
			});
	}

	void DelayedCommandList::copy_resource(HAL::Resource* dest, HAL::Resource* source)
	{
		tasks.emplace_back([=](API::CommandList& list) {
			list.copy_resource(dest, source);
			});
	}

	void  DelayedCommandList::copy_buffer(HAL::Resource* dest, uint64 dest_offset, HAL::Resource* source, uint64 source_offset, uint64 size)
	{
		tasks.emplace_back([=](API::CommandList& list) {
			list.copy_buffer(dest, dest_offset, source, source_offset, size);
			});
	}

	void DelayedCommandList::set_pipeline(PipelineStateBase* pipeline)
	{
		tasks.emplace_back([=](API::CommandList& list) {
			list.set_pipeline(pipeline);
			});
	}

	void DelayedCommandList::execute_indirect(IndirectCommand& command_types, UINT max_commands, Resource* command_buffer, UINT64 command_offset, Resource* counter_buffer, UINT64 counter_offset)
	{
		tasks.emplace_back([=](API::CommandList& list) {
			list.execute_indirect(command_types, max_commands, command_buffer, command_offset, counter_buffer, counter_offset);
			});
	}

	void DelayedCommandList::set_rtv(int c, RTVHandle rt, DSVHandle h)
	{
		tasks.emplace_back([=](API::CommandList& list) {
			list.set_rtv(c, rt, h);
			});
	}

	void DelayedCommandList::start_event(std::wstring str)
	{
		tasks.emplace_back([=](API::CommandList& list) {
			list.start_event(str);
			});
	}

	void DelayedCommandList::end_event()
	{
		tasks.emplace_back([=](API::CommandList& list) {
			list.end_event();
			});
	}

	void DelayedCommandList::build_ras(const HAL::RaytracingBuildDescStructure& build_desc, const HAL::RaytracingBuildDescBottomInputs& bottom)
	{
		tasks.emplace_back([=](API::CommandList& list) {
			list.build_ras(build_desc, bottom);
			});
	}

	void DelayedCommandList::build_ras(const HAL::RaytracingBuildDescStructure& build_desc, const HAL::RaytracingBuildDescTopInputs& top)
	{
		tasks.emplace_back([=](API::CommandList& list) {
			list.build_ras(build_desc, top);
			});
	}

	void DelayedCommandList::copy_texture(const Resource::ptr& dest, int dest_subres, const Resource::ptr& source, int source_subres)
	{
		tasks.emplace_back([=](API::CommandList& list) {
			list.copy_texture(dest, dest_subres, source, source_subres);
			});
	}

	void DelayedCommandList::copy_texture(const Resource::ptr& to, ivec3 to_pos, const Resource::ptr& from, ivec3 from_pos, ivec3 size)
	{
		tasks.emplace_back([=](API::CommandList& list) {
			list.copy_texture(to, to_pos, from, from_pos, size);
			});
	}

	void DelayedCommandList::update_texture(HAL::Resource* resource, ivec3 offset, ivec3 box, UINT sub_resource, ResourceAddress address, texture_layout layout)
	{
		tasks.emplace_back([=](API::CommandList& list) {
			list.update_texture(resource, offset, box, sub_resource, address, layout);
			});
	}

	void DelayedCommandList::read_texture(const  HAL::Resource* resource, ivec3 offset, ivec3 box, UINT sub_resource, ResourceAddress target, texture_layout layout)
	{
		tasks.emplace_back([=](API::CommandList& list) {
			list.read_texture(resource, offset, box, sub_resource, target, layout);
			});
	}
}