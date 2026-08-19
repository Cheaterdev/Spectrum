module;
#include <Core_defs.h>
module HAL:CommandListRecorder;


import :Autogen;

import :Device;
import Core;

import HAL;

namespace HAL
{
	void DelayedCommandList::create(CommandListType type, Device& device)
	{
		list.create(type, device);
		tasks.reserve(4096);
		fn_pool.reserve(256);
	}

	void DelayedCommandList::reset()
	{
		compiled = false;
		debug_recorder.clear();
	}

	// ── compile ───────────────────────────────────────────────────────────────

	void DelayedCommandList::compile(CommandAllocator& allocator)
	{
		{
			PROFILE(L"begin");
			list.begin(allocator);
		}
		list.set_name(name);

		{
			PROFILE(L"tasks");
			for (const auto& cmd : tasks)
			{
				switch (cmd.type)
				{
				case CommandType::Transition:
					if constexpr (BuildOptions::Dev)
						for (const auto& b : cmd.barrier->get_barriers())
							HAL::Debug::BarrierBreakpoints::check_barrier(
								b.resource ? std::string_view{b.resource->name} : std::string_view{},
								b.subres, b.before, b.after);
					list.transitions(*cmd.barrier);
					break;

				case CommandType::Draw:
					list.draw(cmd.draw.vc, cmd.draw.vo, cmd.draw.ic, cmd.draw.io);
					break;

				case CommandType::DrawIndexed:
					list.draw_indexed(cmd.draw_indexed.ic, cmd.draw_indexed.ioff,
					                  cmd.draw_indexed.vo, cmd.draw_indexed.inst,
					                  cmd.draw_indexed.io);
					break;

				case CommandType::Dispatch:
					list.dispatch(cmd.dispatch_args);
					break;

				case CommandType::DispatchMesh:
					list.dispatch_mesh(cmd.dispatch_args);
					break;

				case CommandType::DispatchGraph:
					list.dispatch_graph(cmd.dispatch_graph);
					break;

				case CommandType::CopyResource:
					list.copy_resource(cmd.copy_res.dst, cmd.copy_res.src);
					break;

				case CommandType::Discard:
					list.discard(cmd.discard_res);
					break;

				case CommandType::SetDescriptorHeaps:
					list.set_descriptor_heaps(cmd.desc_heaps.cbv, cmd.desc_heaps.sampler);
					break;

				case CommandType::GraphicsSetConstant:
					list.graphics_set_constant(cmd.set_constant.i, cmd.set_constant.offset, cmd.set_constant.value);
					break;

				case CommandType::ComputeSetConstant:
					list.compute_set_constant(cmd.set_constant.i, cmd.set_constant.offset, cmd.set_constant.value);
					break;

				case CommandType::GraphicsSetConstBuffer:
					list.graphics_set_const_buffer(cmd.set_cb.i, cmd.set_cb.addr);
					break;

				case CommandType::ComputeSetConstBuffer:
					list.compute_set_const_buffer(cmd.set_cb.i, cmd.set_cb.addr);
					break;

				case CommandType::SetStencilRef:
					list.set_stencil_ref(cmd.stencil_ref);
					break;

				case CommandType::SetTopology:
					list.set_topology(cmd.set_topology.topo, cmd.set_topology.feed,
					                  cmd.set_topology.adjusted, cmd.set_topology.cpoints);
					break;

				case CommandType::StartEvent:
					list.start_event(cmd.event_str);
					break;

				case CommandType::EndEvent:
					list.end_event();
					break;

				case CommandType::ResolveTime:
					list.resolve_times(cmd.resolve.heap, cmd.resolve.count, cmd.resolve.dest);
					break;

				case CommandType::SetProgram:
					list.set_program(cmd.set_program.obj, cmd.set_program.buf,
					                 cmd.set_program.size, cmd.set_program.init);
					break;

				default:
					fn_pool[cmd.fn_idx](list);
					break;
				}
			}
		}

		{
			PROFILE(L"end");
			list.end();
		}

		tasks.clear();
		fn_pool.clear();
		compiled = true;
	}

	// ── push methods ─────────────────────────────────────────────────────────

	// Reserve this point in the command stream for a barrier group. The group
	// is still empty at record time -- it is filled in later, once the barriers
	// for the whole list are computed -- so what matters here is only the
	// POSITION. Callers reserve one point per group they intend to emit
	// (CmdListOperation brackets its work with barriers_before/barriers_after).
	// `barriers` must outlive compile(); the groups are owned by the list's
	// `operations` deque, whose entries never move.
	void DelayedCommandList::func_barrier(Barriers* barriers)
	{
		if constexpr (BuildOptions::Dev)
			debug_recorder.push_back({CommandType::Transition, {}, barriers});
		Cmd cmd{}; cmd.type = CommandType::Transition; cmd.barrier = barriers;
		tasks.push_back(cmd);
	}

	void DelayedCommandList::func(std::function<void(API::CommandList&)> f)
	{
		if constexpr (BuildOptions::Dev)
			debug_recorder.push_back({CommandType::Func, "Func"});
		push_fn(CommandType::Func, std::move(f));
	}

	void DelayedCommandList::clear_uav(const Handles::UAV& h, vec4 ClearColor)
	{
		if constexpr (BuildOptions::Dev)
			debug_recorder.push_back({CommandType::ClearUAV, "ClearUAV"});
		push_fn(CommandType::ClearUAV, [=](API::CommandList& list) {
			list.clear_uav(h, ClearColor);
		});
	}

	void DelayedCommandList::clear_rtv(const Handles::RTV& h, vec4 ClearColor)
	{
		if constexpr (BuildOptions::Dev)
			debug_recorder.push_back({CommandType::ClearRTV, "ClearRTV"});
		push_fn(CommandType::ClearRTV, [=](API::CommandList& list) {
			list.clear_rtv(h, ClearColor);
		});
	}

	void DelayedCommandList::clear_stencil(const Handles::DSV& dsv, UINT8 stencil)
	{
		if constexpr (BuildOptions::Dev)
			debug_recorder.push_back({CommandType::ClearStencil,
				"ClearStencil s=" + std::to_string(stencil)});
		push_fn(CommandType::ClearStencil, [=](API::CommandList& list) {
			list.clear_stencil(dsv, stencil);
		});
	}

	void DelayedCommandList::clear_depth(const Handles::DSV& dsv, float depth)
	{
		if constexpr (BuildOptions::Dev)
			debug_recorder.push_back({CommandType::ClearDepth,
				"ClearDepth d=" + std::to_string(depth)});
		push_fn(CommandType::ClearDepth, [=](API::CommandList& list) {
			list.clear_depth(dsv, depth);
		});
	}

	void DelayedCommandList::clear_depth_rects(const Handles::DSV& dsv, float depth, std::vector<sizer_long> rects)
	{
		if constexpr (BuildOptions::Dev)
			debug_recorder.push_back({CommandType::ClearDepth,
				"ClearDepthRects d=" + std::to_string(depth) + " n=" + std::to_string(rects.size())});
		push_fn(CommandType::ClearDepth, [dsv, depth, rc = std::move(rects)](API::CommandList& list) {
			list.clear_depth_rects(dsv, depth, rc);
		});
	}

	void DelayedCommandList::clear_depth_stencil(const Handles::DSV& dsv, bool depth, bool stencil, float fdepth, UINT8 fstencil)
	{
		if constexpr (BuildOptions::Dev)
			debug_recorder.push_back({CommandType::ClearDepthStencil,
				std::string("ClearDepthStencil") +
				(depth   ? " d=" + std::to_string(fdepth)  : "") +
				(stencil ? " s=" + std::to_string(fstencil) : "")});
		push_fn(CommandType::ClearDepthStencil, [=](API::CommandList& list) {
			list.clear_depth_stencil(dsv, depth, stencil, fdepth, fstencil);
		});
	}

	void DelayedCommandList::set_topology(HAL::PrimitiveTopologyType topology, HAL::PrimitiveTopologyFeed feedType, bool adjusted, uint controlpoints)
	{
		if constexpr (BuildOptions::Dev)
			debug_recorder.push_back({CommandType::SetTopology, "SetTopology"});
		Cmd cmd{}; cmd.type = CommandType::SetTopology;
		cmd.set_topology = {topology, feedType, adjusted, controlpoints};
		tasks.push_back(cmd);
	}

	void DelayedCommandList::set_stencil_ref(UINT ref)
	{
		if constexpr (BuildOptions::Dev)
			debug_recorder.push_back({CommandType::SetStencilRef, "SetStencilRef " + std::to_string(ref)});
		Cmd cmd{}; cmd.type = CommandType::SetStencilRef; cmd.stencil_ref = ref;
		tasks.push_back(cmd);
	}

	void DelayedCommandList::set_name(std::wstring_view name)
	{
		this->name = name;
	}

	void DelayedCommandList::set_program(StateObject* id, ResourceAddress buffer, uint size, bool init)
	{
		if constexpr (BuildOptions::Dev)
			debug_recorder.push_back({CommandType::SetProgram, "SetProgram"});
		Cmd cmd{}; cmd.type = CommandType::SetProgram;
		cmd.set_program = {id, buffer, size, init};
		tasks.push_back(cmd);
	}

	void DelayedCommandList::dispatch_graph(ResourceAddress addr)
	{
		if constexpr (BuildOptions::Dev)
			debug_recorder.push_back({CommandType::DispatchGraph, "DispatchGraph"});
		Cmd cmd{}; cmd.type = CommandType::DispatchGraph; cmd.dispatch_graph = addr;
		tasks.push_back(cmd);
	}

	void DelayedCommandList::set_descriptor_heaps(DescriptorHeap* cbv, DescriptorHeap* sampler)
	{
		if constexpr (BuildOptions::Dev)
			debug_recorder.push_back({CommandType::SetDescriptorHeaps, "SetDescriptorHeaps"});
		Cmd cmd{}; cmd.type = CommandType::SetDescriptorHeaps;
		cmd.desc_heaps = {cbv, sampler};
		tasks.push_back(cmd);
	}

	void DelayedCommandList::insert_time(const QueryHandle& handle, uint offset)
	{
		if constexpr (BuildOptions::Dev)
			debug_recorder.push_back({CommandType::InsertTime, "InsertTime off=" + std::to_string(offset)});
		push_fn(CommandType::InsertTime, [=](API::CommandList& list) {
			list.insert_time(handle, offset);
		});
	}

	void DelayedCommandList::resolve_times(const QueryHeap* pQueryHeap, uint32_t NumQueries, ResourceAddress destination)
	{
		if constexpr (BuildOptions::Dev)
			debug_recorder.push_back({CommandType::ResolveTime, "ResolveTime n=" + std::to_string(NumQueries)});
		Cmd cmd{}; cmd.type = CommandType::ResolveTime;
		cmd.resolve = {pQueryHeap, NumQueries, destination};
		tasks.push_back(cmd);
	}

	void DelayedCommandList::set_graphics_signature(const HAL::RootSignature::ptr& s)
	{
		if constexpr (BuildOptions::Dev)
			debug_recorder.push_back({CommandType::SetGraphicsSignature, "SetGraphicsSignature"});
		push_fn(CommandType::SetGraphicsSignature, [=](API::CommandList& list) {
			list.set_graphics_signature(s);
		});
	}

	void DelayedCommandList::set_compute_signature(const HAL::RootSignature::ptr& s)
	{
		if constexpr (BuildOptions::Dev)
			debug_recorder.push_back({CommandType::SetComputeSignature, "SetComputeSignature"});
		push_fn(CommandType::SetComputeSignature, [=](API::CommandList& list) {
			list.set_compute_signature(s);
		});
	}

	void DelayedCommandList::draw(UINT vertex_count, UINT vertex_offset, UINT instance_count, UINT instance_offset)
	{
		if constexpr (BuildOptions::Dev)
			debug_recorder.push_back({CommandType::Draw,
				"Draw vc=" + std::to_string(vertex_count) +
				" vo=" + std::to_string(vertex_offset) +
				" ic=" + std::to_string(instance_count) +
				" io=" + std::to_string(instance_offset)});
		Cmd cmd{}; cmd.type = CommandType::Draw;
		cmd.draw = {vertex_count, vertex_offset, instance_count, instance_offset};
		tasks.push_back(cmd);
	}

	void DelayedCommandList::draw_indexed(UINT index_count, UINT index_offset, UINT vertex_offset, UINT instance_count, UINT instance_offset)
	{
		if constexpr (BuildOptions::Dev)
			debug_recorder.push_back({CommandType::DrawIndexed,
				"DrawIndexed ic=" + std::to_string(index_count) +
				" io=" + std::to_string(index_offset) +
				" vo=" + std::to_string(vertex_offset) +
				" inst=" + std::to_string(instance_count)});
		Cmd cmd{}; cmd.type = CommandType::DrawIndexed;
		cmd.draw_indexed = {index_count, index_offset, vertex_offset, instance_count, instance_offset};
		tasks.push_back(cmd);
	}

	void DelayedCommandList::set_index_buffer(HAL::Views::IndexBuffer index)
	{
		if constexpr (BuildOptions::Dev)
			debug_recorder.push_back({CommandType::SetIndexBuffer, "SetIndexBuffer"});
		push_fn(CommandType::SetIndexBuffer, [=](API::CommandList& list) {
			list.set_index_buffer(index);
		});
	}

	void DelayedCommandList::graphics_set_const_buffer(UINT i, const ResourceAddress& addr)
	{
		if constexpr (BuildOptions::Dev)
			debug_recorder.push_back({CommandType::GraphicsSetConstBuffer,
				"GfxSetCB slot=" + std::to_string(i)});
		Cmd cmd{}; cmd.type = CommandType::GraphicsSetConstBuffer;
		cmd.set_cb = {i, addr};
		tasks.push_back(cmd);
	}

	void DelayedCommandList::compute_set_const_buffer(UINT i, const ResourceAddress& addr)
	{
		if constexpr (BuildOptions::Dev)
			debug_recorder.push_back({CommandType::ComputeSetConstBuffer,
				"CmpSetCB slot=" + std::to_string(i)});
		Cmd cmd{}; cmd.type = CommandType::ComputeSetConstBuffer;
		cmd.set_cb = {i, addr};
		tasks.push_back(cmd);
	}

	void DelayedCommandList::graphics_set_constant(UINT i, UINT offset, UINT value)
	{
		if constexpr (BuildOptions::Dev)
			debug_recorder.push_back({CommandType::GraphicsSetConstant,
				"GfxSetConst slot=" + std::to_string(i) + " off=" + std::to_string(offset)});
		Cmd cmd{}; cmd.type = CommandType::GraphicsSetConstant;
		cmd.set_constant = {i, offset, value};
		tasks.push_back(cmd);
	}

	void DelayedCommandList::compute_set_constant(UINT i, UINT offset, UINT value)
	{
		if constexpr (BuildOptions::Dev)
			debug_recorder.push_back({CommandType::ComputeSetConstant,
				"CmpSetConst slot=" + std::to_string(i) + " off=" + std::to_string(offset)});
		Cmd cmd{}; cmd.type = CommandType::ComputeSetConstant;
		cmd.set_constant = {i, offset, value};
		tasks.push_back(cmd);
	}

	void DelayedCommandList::dispatch_mesh(ivec3 v)
	{
		if constexpr (BuildOptions::Dev)
			debug_recorder.push_back({CommandType::DispatchMesh,
				"DispatchMesh " + std::to_string(v.x) + " " + std::to_string(v.y) + " " + std::to_string(v.z)});
		Cmd cmd{}; cmd.type = CommandType::DispatchMesh; cmd.dispatch_args = v;
		tasks.push_back(cmd);
	}

	void DelayedCommandList::dispatch(ivec3 v)
	{
		if constexpr (BuildOptions::Dev)
			debug_recorder.push_back({CommandType::Dispatch,
				"Dispatch " + std::to_string(v.x) + " " + std::to_string(v.y) + " " + std::to_string(v.z)});
		Cmd cmd{}; cmd.type = CommandType::Dispatch; cmd.dispatch_args = v;
		tasks.push_back(cmd);
	}

	void DelayedCommandList::set_scissors(sizer_long rect)
	{
		if constexpr (BuildOptions::Dev)
			debug_recorder.push_back({CommandType::SetScissor, "SetScissor"});
		push_fn(CommandType::SetScissor, [=](API::CommandList& list) {
			list.set_scissors(rect);
		});
	}

	void DelayedCommandList::set_scissors(std::vector<sizer_long> rects)
	{
		if constexpr (BuildOptions::Dev)
			debug_recorder.push_back({CommandType::SetScissor, "SetScissor"});
		push_fn(CommandType::SetScissor, [rc = std::move(rects)](API::CommandList& list) {
			list.set_scissors(rc);
		});
	}

	void DelayedCommandList::set_viewports(std::vector<Viewport> viewports)
	{
		if constexpr (BuildOptions::Dev)
			debug_recorder.push_back({CommandType::SetViewport, "SetViewport"});
		push_fn(CommandType::SetViewport, [vp = std::move(viewports)](API::CommandList& list) {
			list.set_viewports(vp);
		});
	}

	void DelayedCommandList::copy_resource(HAL::Resource* dest, HAL::Resource* source)
	{
		if constexpr (BuildOptions::Dev)
			debug_recorder.push_back({CommandType::CopyResource, "CopyResource"});
		Cmd cmd{}; cmd.type = CommandType::CopyResource;
		cmd.copy_res = {dest, source};
		tasks.push_back(cmd);
	}

	void DelayedCommandList::copy_buffer(HAL::Resource* dest, uint64 dest_offset, HAL::Resource* source, uint64 source_offset, uint64 size)
	{
		if constexpr (BuildOptions::Dev)
			debug_recorder.push_back({CommandType::CopyBuffer,
				"CopyBuffer sz=" + std::to_string(size)});
		push_fn(CommandType::CopyBuffer, [=](API::CommandList& list) {
			list.copy_buffer(dest, dest_offset, source, source_offset, size);
		});
	}

	void DelayedCommandList::set_pipeline(HAL::PipelineStateBase* pipeline)
	{
		if constexpr (BuildOptions::Dev)
			debug_recorder.push_back({CommandType::SetPipeline,
				"SetPipeline " + std::string(pipeline->name.begin(), pipeline->name.end())});
		auto info = pipeline->get_tracked();
		push_fn(CommandType::SetPipeline, [info](API::CommandList& list) {
			list.set_pipeline(info);
		});
	}

	void DelayedCommandList::execute_indirect(IndirectCommand& command_types, UINT max_commands, Resource* command_buffer, UINT64 command_offset, Resource* counter_buffer, UINT64 counter_offset)
	{
		if constexpr (BuildOptions::Dev)
			debug_recorder.push_back({CommandType::ExecuteIndirect,
				"ExecuteIndirect max=" + std::to_string(max_commands)});
		push_fn(CommandType::ExecuteIndirect, [=](API::CommandList& list) {
			list.execute_indirect(command_types, max_commands, command_buffer, command_offset, counter_buffer, counter_offset);
		});
	}

	void DelayedCommandList::set_rtv(int c, Handles::RTV rt, Handles::DSV h)
	{
		if constexpr (BuildOptions::Dev)
			debug_recorder.push_back({CommandType::SetRTV, "SetRTV count=" + std::to_string(c)});
		push_fn(CommandType::SetRTV, [=](API::CommandList& list) {
			list.set_rtv(c, rt, h);
		});
	}

	void DelayedCommandList::start_event(std::wstring_view str)
	{
		if constexpr (BuildOptions::Dev)
			debug_recorder.push_back({CommandType::StartEvent,
				"StartEvent " + std::string(str.begin(), str.end())});
		Cmd cmd{}; cmd.type = CommandType::StartEvent; cmd.event_str = str;
		tasks.push_back(cmd);
	}

	void DelayedCommandList::end_event()
	{
		if constexpr (BuildOptions::Dev)
			debug_recorder.push_back({CommandType::EndEvent, "EndEvent"});
		Cmd cmd{}; cmd.type = CommandType::EndEvent;
		tasks.push_back(cmd);
	}

	void DelayedCommandList::build_ras(const HAL::RaytracingBuildDescStructure& build_desc, const HAL::RaytracingBuildDescBottomInputs& bottom)
	{
		if constexpr (BuildOptions::Dev)
			debug_recorder.push_back({CommandType::BuildRAS, "BuildRAS BLAS"});
		push_fn(CommandType::BuildRAS, [=](API::CommandList& list) {
			list.build_ras(build_desc, bottom);
		});
	}

	void DelayedCommandList::build_ras(const HAL::RaytracingBuildDescStructure& build_desc, const HAL::RaytracingBuildDescTopInputs& top)
	{
		if constexpr (BuildOptions::Dev)
			debug_recorder.push_back({CommandType::BuildRAS, "BuildRAS TLAS"});
		push_fn(CommandType::BuildRAS, [=](API::CommandList& list) {
			list.build_ras(build_desc, top);
		});
	}

	void DelayedCommandList::copy_texture(const Resource::ptr& dest, int dest_subres, const Resource::ptr& source, int source_subres)
	{
		if constexpr (BuildOptions::Dev)
			debug_recorder.push_back({CommandType::CopyTexture,
				"CopyTexture subres " + std::to_string(source_subres) + "->" + std::to_string(dest_subres)});
		push_fn(CommandType::CopyTexture, [=](API::CommandList& list) {
			list.copy_texture(dest, dest_subres, source, source_subres);
		});
	}

	void DelayedCommandList::copy_texture(const Resource::ptr& to, ivec3 to_pos, const Resource::ptr& from, ivec3 from_pos, ivec3 size)
	{
		if constexpr (BuildOptions::Dev)
			debug_recorder.push_back({CommandType::CopyTexture, "CopyTexture region"});
		push_fn(CommandType::CopyTexture, [=](API::CommandList& list) {
			list.copy_texture(to, to_pos, from, from_pos, size);
		});
	}

	void DelayedCommandList::update_texture(HAL::Resource* resource, ivec3 offset, ivec3 box, UINT sub_resource, ResourceAddress address, texture_layout layout)
	{
		if constexpr (BuildOptions::Dev)
			debug_recorder.push_back({CommandType::UpdateTexture,
				"UpdateTexture subres=" + std::to_string(sub_resource)});
		push_fn(CommandType::UpdateTexture, [=](API::CommandList& list) {
			list.update_texture(resource, offset, box, sub_resource, address, layout);
		});
	}

	void DelayedCommandList::read_texture(const  HAL::Resource* resource, ivec3 offset, ivec3 box, UINT sub_resource, ResourceAddress target, texture_layout layout)
	{
		if constexpr (BuildOptions::Dev)
			debug_recorder.push_back({CommandType::ReadTexture,
				"ReadTexture subres=" + std::to_string(sub_resource)});
		push_fn(CommandType::ReadTexture, [=](API::CommandList& list) {
			list.read_texture(resource, offset, box, sub_resource, target, layout);
		});
	}

	void DelayedCommandList::discard(const HAL::Resource* resource)
	{
		if constexpr (BuildOptions::Dev)
			debug_recorder.push_back({CommandType::Discard, "Discard"});
		Cmd cmd{}; cmd.type = CommandType::Discard; cmd.discard_res = resource;
		tasks.push_back(cmd);
	}

	const API::CommandList& DelayedCommandList::get_list() const { return list; }
	bool DelayedCommandList::is_compiled() const { return compiled; }
	const std::vector<CommandRecord>& DelayedCommandList::get_debug_records() const { return debug_recorder.get(); }
	std::vector<CommandRecord> DelayedCommandList::take_debug_records() { return debug_recorder.take(); }
}
