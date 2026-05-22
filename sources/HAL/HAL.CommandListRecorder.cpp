module;
#include <Core_defs.h>
module HAL:CommandListRecorder;


import :Autogen;

import :Device;
import Core;

import HAL;

namespace HAL
{
	void DelayedCommandList::create(CommandListType type)
	{
		list.create(type);
		tasks.reserve(4096);
	}

	void DelayedCommandList::reset()
	{
		compiled = false;
		debug_records.clear();
	}

	void DelayedCommandList::func_barrier(UsagePoint* point)
	{
		debug_records.push_back({CommandType::Transition, {}, point});
		tasks.emplace_back([point](API::CommandList& list)
		{
			for (const auto& b : point->transitions.get_barriers())
				HAL::Debug::BarrierBreakpoints::check_barrier(
					b.resource ? std::string_view{b.resource->name} : std::string_view{},
					b.subres, b.before, b.after);
			list.transitions(point->transitions);
		});
	}

	void DelayedCommandList::compile(CommandAllocator& allocator)
	{
		{
				   			PROFILE(L"begin");
		list.begin(allocator);
		
		}
		list.set_name(name);

		{
			PROFILE(L"tasks");
			for (auto& f : tasks)
			{
			//		PROFILE(L"task");
				f(list);
			}
		}

		{
			PROFILE(L"end");
					list.end();
		}

		tasks.clear();
		compiled = true;
	}

	void DelayedCommandList::discard(const  HAL::Resource* resource)
	{
		debug_records.push_back({CommandType::Discard, "Discard"});
		tasks.emplace_back([=](API::CommandList& list) {
			list.discard(resource);
			});
	}

	void DelayedCommandList::func(std::function<void(API::CommandList&)> f)
	{
		debug_records.push_back({CommandType::Func, "Func"});
		tasks.emplace_back([=](API::CommandList& list) {
		//	PROFILE(L"universal_func");
			f(list);
			});
	}

	void DelayedCommandList::clear_uav(const UAVHandle& h, vec4 ClearColor)
	{
		debug_records.push_back({CommandType::ClearUAV, "ClearUAV"});
		tasks.emplace_back([=](API::CommandList& list) {
			list.clear_uav(h, ClearColor);
			});
	}

	void DelayedCommandList::clear_rtv(const RTVHandle& h, vec4 ClearColor)
	{
		debug_records.push_back({CommandType::ClearRTV, "ClearRTV"});
		tasks.emplace_back([=](API::CommandList& list) {
			list.clear_rtv(h, ClearColor);
			});
	}

	void DelayedCommandList::clear_stencil(const DSVHandle& dsv, UINT8 stencil)
	{
		debug_records.push_back({CommandType::ClearStencil,
			"ClearStencil s=" + std::to_string(stencil)});
		tasks.emplace_back([=](API::CommandList& list) {
			list.clear_stencil(dsv, stencil);
			});
	}

	void DelayedCommandList::clear_depth(const DSVHandle& dsv, float depth)
	{
		debug_records.push_back({CommandType::ClearDepth,
			"ClearDepth d=" + std::to_string(depth)});
		tasks.emplace_back([=](API::CommandList& list) {
			list.clear_depth(dsv, depth);
			});
	}

	void DelayedCommandList::clear_depth_stencil(const DSVHandle& dsv, bool depth, bool stencil, float fdepth, UINT8 fstencil)
	{
		debug_records.push_back({CommandType::ClearDepthStencil,
			std::string("ClearDepthStencil") +
			(depth   ? " d=" + std::to_string(fdepth)  : "") +
			(stencil ? " s=" + std::to_string(fstencil) : "")});
		tasks.emplace_back([=](API::CommandList& list) {
			list.clear_depth_stencil(dsv, depth, stencil, fdepth, fstencil);
			});
	}

	void DelayedCommandList::set_topology(HAL::PrimitiveTopologyType topology, HAL::PrimitiveTopologyFeed feedType, bool adjusted, uint controlpoints)
	{
		debug_records.push_back({CommandType::SetTopology, "SetTopology"});
		tasks.emplace_back([=](API::CommandList& list) {
			list.set_topology(topology, feedType, adjusted, controlpoints);
			});
	}

	void DelayedCommandList::set_stencil_ref(UINT ref)
	{
		debug_records.push_back({CommandType::SetStencilRef, "SetStencilRef " + std::to_string(ref)});
		tasks.emplace_back([=](API::CommandList& list) {
			list.set_stencil_ref(ref);
			});
	}

	void DelayedCommandList::set_name(std::wstring_view name)
	{
		this->name = name;
	}



	void DelayedCommandList::set_program(StateObject* id, ResourceAddress buffer, uint size, bool init)
	{
		debug_records.push_back({CommandType::SetProgram, "SetProgram"});
		tasks.emplace_back([=](API::CommandList& list) {
			list.set_program(id, buffer, size, init);
			});
	}
	void DelayedCommandList::dispatch_graph(ResourceAddress addr)
	{
		debug_records.push_back({CommandType::DispatchGraph, "DispatchGraph"});
		tasks.emplace_back([=](API::CommandList& list) {
			list.dispatch_graph(addr);
			});
	}


	void DelayedCommandList::set_descriptor_heaps(DescriptorHeap* cbv, DescriptorHeap* sampler)
	{
		debug_records.push_back({CommandType::SetDescriptorHeaps, "SetDescriptorHeaps"});
		tasks.emplace_back([=](API::CommandList& list) {
			list.set_descriptor_heaps(cbv, sampler);
			});
	}

	void DelayedCommandList::insert_time(const QueryHandle& handle, uint offset)
	{
		debug_records.push_back({CommandType::InsertTime, "InsertTime off=" + std::to_string(offset)});
		tasks.emplace_back([=](API::CommandList& list) {
			list.insert_time(handle, offset);
			});
	}

	void DelayedCommandList::resolve_times(const QueryHeap* pQueryHeap, uint32_t NumQueries, ResourceAddress destination)
	{
		debug_records.push_back({CommandType::ResolveTime, "ResolveTime n=" + std::to_string(NumQueries)});
		tasks.emplace_back([=](API::CommandList& list) {
			list.resolve_times(pQueryHeap, NumQueries, destination);
			});
	}

	void DelayedCommandList::set_graphics_signature(const HAL::RootSignature::ptr& s)
	{
		debug_records.push_back({CommandType::SetGraphicsSignature, "SetGraphicsSignature"});
		tasks.emplace_back([=](API::CommandList& list) {
			list.set_graphics_signature(s);
			});
	}

	void DelayedCommandList::set_compute_signature(const HAL::RootSignature::ptr& s)
	{
		debug_records.push_back({CommandType::SetComputeSignature, "SetComputeSignature"});
		tasks.emplace_back([=](API::CommandList& list) {
			list.set_compute_signature(s);
			});
	}

	void DelayedCommandList::draw(UINT vertex_count, UINT vertex_offset, UINT instance_count, UINT instance_offset)
	{
		debug_records.push_back({CommandType::Draw,
			"Draw vc=" + std::to_string(vertex_count) +
			" vo=" + std::to_string(vertex_offset) +
			" ic=" + std::to_string(instance_count) +
			" io=" + std::to_string(instance_offset)});
		tasks.emplace_back([=](API::CommandList& list) {
			list.draw(vertex_count, vertex_offset, instance_count, instance_offset);
			});
	}

	void DelayedCommandList::draw_indexed(UINT index_count, UINT index_offset, UINT vertex_offset, UINT instance_count, UINT instance_offset)
	{
		debug_records.push_back({CommandType::DrawIndexed,
			"DrawIndexed ic=" + std::to_string(index_count) +
			" io=" + std::to_string(index_offset) +
			" vo=" + std::to_string(vertex_offset) +
			" inst=" + std::to_string(instance_count)});
		tasks.emplace_back([=](API::CommandList& list) {
			list.draw_indexed(index_count, index_offset, vertex_offset, instance_count, instance_offset);
			});
	}

	void DelayedCommandList::set_index_buffer(HAL::Views::IndexBuffer index)
	{
		debug_records.push_back({CommandType::SetIndexBuffer, "SetIndexBuffer"});
		tasks.emplace_back([=](API::CommandList& list) {
			list.set_index_buffer(index);
			});
	}

	void  DelayedCommandList::graphics_set_const_buffer(UINT i, const ResourceAddress& adress)
	{
		debug_records.push_back({CommandType::GraphicsSetConstBuffer,
			"GfxSetCB slot=" + std::to_string(i)});
		tasks.emplace_back([=](API::CommandList& list) {
			list.graphics_set_const_buffer(i, adress);
			});
	}

	void  DelayedCommandList::compute_set_const_buffer(UINT i, const ResourceAddress& adress)
	{
		debug_records.push_back({CommandType::ComputeSetConstBuffer,
			"CmpSetCB slot=" + std::to_string(i)});
		tasks.emplace_back([=](API::CommandList& list) {
			list.compute_set_const_buffer(i, adress);
			});
	}

	void  DelayedCommandList::graphics_set_constant(UINT i, UINT offset, UINT value)
	{
		debug_records.push_back({CommandType::GraphicsSetConstant,
			"GfxSetConst slot=" + std::to_string(i) + " off=" + std::to_string(offset)});
		tasks.emplace_back([=](API::CommandList& list) {
			list.graphics_set_constant(i, offset, value);
			});
	}

	void  DelayedCommandList::compute_set_constant(UINT i, UINT offset, UINT value)
	{
		debug_records.push_back({CommandType::ComputeSetConstant,
			"CmpSetConst slot=" + std::to_string(i) + " off=" + std::to_string(offset)});
		tasks.emplace_back([=](API::CommandList& list) {
			list.compute_set_constant(i, offset, value);
			});
	}

	void DelayedCommandList::dispatch_mesh(ivec3 v)
	{
		debug_records.push_back({CommandType::DispatchMesh,
			"DispatchMesh " + std::to_string(v.x) + " " + std::to_string(v.y) + " " + std::to_string(v.z)});
		tasks.emplace_back([=](API::CommandList& list) {
			list.dispatch_mesh(v);
			});
	}

	void DelayedCommandList::dispatch(ivec3 v)
	{
		debug_records.push_back({CommandType::Dispatch,
			"Dispatch " + std::to_string(v.x) + " " + std::to_string(v.y) + " " + std::to_string(v.z)});
		tasks.emplace_back([=](API::CommandList& list) {
			list.dispatch(v);
			});
	}
	void DelayedCommandList::set_scissors(sizer_long rect)
	{
		debug_records.push_back({CommandType::SetScissor, "SetScissor"});
		tasks.emplace_back([=](API::CommandList& list) {
			list.set_scissors(rect);
			});
	}

	void DelayedCommandList::set_viewports(std::vector<Viewport> viewports)
	{
		debug_records.push_back({CommandType::SetViewport, "SetViewport"});
		tasks.emplace_back([=](API::CommandList& list) {
			list.set_viewports(viewports);
			});
	}

	void DelayedCommandList::copy_resource(HAL::Resource* dest, HAL::Resource* source)
	{
		debug_records.push_back({CommandType::CopyResource, "CopyResource"});
		tasks.emplace_back([=](API::CommandList& list) {
			list.copy_resource(dest, source);
			});
	}

	void  DelayedCommandList::copy_buffer(HAL::Resource* dest, uint64 dest_offset, HAL::Resource* source, uint64 source_offset, uint64 size)
	{
		debug_records.push_back({CommandType::CopyBuffer,
			"CopyBuffer sz=" + std::to_string(size)});
		tasks.emplace_back([=](API::CommandList& list) {
			list.copy_buffer(dest, dest_offset, source, source_offset, size);
			});
	}

	void DelayedCommandList::set_pipeline(HAL::PipelineStateBase* pipeline)
	{
		debug_records.push_back({CommandType::SetPipeline,
			"SetPipeline " + std::string(pipeline->name.begin(), pipeline->name.end())});
		auto info = pipeline->get_tracked();
		tasks.emplace_back([info](API::CommandList& list) {
			list.set_pipeline(info);
		});
	}

	void DelayedCommandList::execute_indirect(IndirectCommand& command_types, UINT max_commands, Resource* command_buffer, UINT64 command_offset, Resource* counter_buffer, UINT64 counter_offset)
	{
		debug_records.push_back({CommandType::ExecuteIndirect,
			"ExecuteIndirect max=" + std::to_string(max_commands)});
		tasks.emplace_back([=](API::CommandList& list) {
			list.execute_indirect(command_types, max_commands, command_buffer, command_offset, counter_buffer, counter_offset);
			});
	}

	void DelayedCommandList::set_rtv(int c, RTVHandle rt, DSVHandle h)
	{
		debug_records.push_back({CommandType::SetRTV, "SetRTV count=" + std::to_string(c)});
		tasks.emplace_back([=](API::CommandList& list) {
			list.set_rtv(c, rt, h);
			});
	}

	void DelayedCommandList::start_event(std::wstring_view str)
	{
		debug_records.push_back({CommandType::StartEvent,
			"StartEvent " + std::string(str.begin(), str.end())});
		tasks.emplace_back([=](API::CommandList& list) {
			list.start_event(str);
			});
	}

	void DelayedCommandList::end_event()
	{
		debug_records.push_back({CommandType::EndEvent, "EndEvent"});
		tasks.emplace_back([=](API::CommandList& list) {
			list.end_event();
			});
	}

	void DelayedCommandList::build_ras(const HAL::RaytracingBuildDescStructure& build_desc, const HAL::RaytracingBuildDescBottomInputs& bottom)
	{
		debug_records.push_back({CommandType::BuildRAS, "BuildRAS BLAS"});
		tasks.emplace_back([=](API::CommandList& list) {
			list.build_ras(build_desc, bottom);
			});
	}

	void DelayedCommandList::build_ras(const HAL::RaytracingBuildDescStructure& build_desc, const HAL::RaytracingBuildDescTopInputs& top)
	{
		debug_records.push_back({CommandType::BuildRAS, "BuildRAS TLAS"});
		tasks.emplace_back([=](API::CommandList& list) {
			list.build_ras(build_desc, top);
			});
	}

	void DelayedCommandList::copy_texture(const Resource::ptr& dest, int dest_subres, const Resource::ptr& source, int source_subres)
	{
		debug_records.push_back({CommandType::CopyTexture,
			"CopyTexture subres " + std::to_string(source_subres) + "->" + std::to_string(dest_subres)});
		tasks.emplace_back([=](API::CommandList& list) {
			list.copy_texture(dest, dest_subres, source, source_subres);
			});
	}

	void DelayedCommandList::copy_texture(const Resource::ptr& to, ivec3 to_pos, const Resource::ptr& from, ivec3 from_pos, ivec3 size)
	{
		debug_records.push_back({CommandType::CopyTexture, "CopyTexture region"});
		tasks.emplace_back([=](API::CommandList& list) {
			list.copy_texture(to, to_pos, from, from_pos, size);
			});
	}

	void DelayedCommandList::update_texture(HAL::Resource* resource, ivec3 offset, ivec3 box, UINT sub_resource, ResourceAddress address, texture_layout layout)
	{
		debug_records.push_back({CommandType::UpdateTexture,
			"UpdateTexture subres=" + std::to_string(sub_resource)});
		tasks.emplace_back([=](API::CommandList& list) {
			list.update_texture(resource, offset, box, sub_resource, address, layout);
			});
	}

	void DelayedCommandList::read_texture(const  HAL::Resource* resource, ivec3 offset, ivec3 box, UINT sub_resource, ResourceAddress target, texture_layout layout)
	{
		debug_records.push_back({CommandType::ReadTexture,
			"ReadTexture subres=" + std::to_string(sub_resource)});
		tasks.emplace_back([=](API::CommandList& list) {
			list.read_texture(resource, offset, box, sub_resource, target, layout);
			});
	}
}