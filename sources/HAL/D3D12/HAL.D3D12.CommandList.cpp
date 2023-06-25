module;
import <HAL.h>;
import <d3d12/d3d12_includes.h>;
#include <pix3.h>
module HAL:API.CommandList;
import Core;
import :Device;
import :CommandAllocator;
namespace HAL
{
	namespace API
	{

		void CommandList::create(CommandListType type)
		{
			this->type = type;
		}

		void CommandList::begin(HAL::CommandAllocator& allocator)
		{
			if (!m_commandList)
			{
				TEST(HAL::Device::get(), HAL::Device::get().get_native_device()->CreateCommandList(0, to_native(type), allocator.m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList)));
			}
			else
			{
				TEST(HAL::Device::get(), m_commandList->Reset(allocator.m_commandAllocator.Get(), nullptr));
			}

			native_topology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
		}

		void CommandList::end()
		{
			m_commandList->Close();
		}


		void CommandList::clear_uav(const UAVHandle& h, vec4 ClearColor)
		{
			auto v = h.get_resource_info().view;
			auto uav = std::get<HAL::Views::UnorderedAccess>(v);

			if (Debug::CheckErrors)
			{
				if (uav.Resource->get_desc().is_buffer())
				{
					auto buffer = std::get<HAL::Views::UnorderedAccess::Buffer>(uav.View);
					assert(buffer.StructureByteStride == 0);
				}
			}

			auto dx_resource = uav.Resource->native_resource.Get();
			m_commandList->ClearUnorderedAccessViewFloat(h.get_gpu(), h.get_cpu(), dx_resource, reinterpret_cast<FLOAT*>(ClearColor.data()), 0, nullptr);
		}

		void CommandList::clear_rtv(const RTVHandle& h, vec4 ClearColor)
		{
			m_commandList->ClearRenderTargetView(h.get_cpu(), ClearColor.data(), 0, nullptr);
		}

		void CommandList::clear_stencil(const DSVHandle& dsv, UINT8 stencil)
		{
			m_commandList->ClearDepthStencilView(dsv.get_cpu(), D3D12_CLEAR_FLAG_STENCIL, 0, stencil, 0, nullptr);
		}

		void CommandList::clear_depth(const DSVHandle& dsv, float depth)
		{
			m_commandList->ClearDepthStencilView(dsv.get_cpu(), D3D12_CLEAR_FLAG_DEPTH, depth, 0, 0, nullptr);
		}

		void CommandList::clear_depth_stencil(const DSVHandle& dsv, bool depth, bool stencil, float fdepth, UINT8 fstencil)
		{
			m_commandList->ClearDepthStencilView(dsv.get_cpu(), D3D12_CLEAR_FLAGS((depth * D3D12_CLEAR_FLAG_DEPTH) | (stencil * D3D12_CLEAR_FLAG_STENCIL)), fdepth, fstencil, 0, nullptr);
		}

		void CommandList::set_topology(HAL::PrimitiveTopologyType topology, HAL::PrimitiveTopologyFeed feedType, bool adjusted, uint controlpoints)
		{
			auto native = to_native(topology, feedType, adjusted, controlpoints);
			if (this->native_topology != native)
				m_commandList->IASetPrimitiveTopology(native);

			this->native_topology = native;
		}

		void CommandList::set_stencil_ref(UINT ref)
		{
			m_commandList->OMSetStencilRef(ref);
		}

		void CommandList::set_name(std::wstring_view name)
		{
			m_commandList->SetName(name.data());
		};

		void CommandList::set_descriptor_heaps(DescriptorHeap* cbv, DescriptorHeap* sampler)
		{
			std::array<ID3D12DescriptorHeap*, 2> heaps;

			heaps[0] = sampler ? sampler->get_dx() : nullptr;
			heaps[1] = cbv ? cbv->get_dx() : nullptr;
			m_commandList->SetDescriptorHeaps(2, heaps.data());
		}


		void CommandList::insert_time(const QueryHandle& handle, uint offset)
		{
			m_commandList->EndQuery(handle.get_heap()->get_native().Get(), D3D12_QUERY_TYPE_TIMESTAMP, static_cast<uint>(handle.get_offset()) + offset);
		}

		void CommandList::resolve_times(const QueryHeap* pQueryHeap, uint32_t NumQueries, ResourceAddress destination)
		{
			m_commandList->ResolveQueryData(pQueryHeap->get_native().Get(), D3D12_QUERY_TYPE_TIMESTAMP, 0, NumQueries, destination.resource->get_dx(), destination.resource_offset);
		}


		void CommandList::set_graphics_signature(const HAL::RootSignature::ptr& s)
		{
			m_commandList->SetGraphicsRootSignature(s->get_native().Get());
		}

		void CommandList::set_compute_signature(const HAL::RootSignature::ptr& s)
		{
			m_commandList->SetComputeRootSignature(s->get_native().Get());
		}

		void CommandList::draw(UINT vertex_count, UINT vertex_offset, UINT instance_count, UINT instance_offset)
		{
			m_commandList->DrawInstanced(vertex_count, instance_count, vertex_offset, instance_offset);
		}

		void CommandList::draw_indexed(UINT index_count, UINT index_offset, UINT vertex_offset, UINT instance_count, UINT instance_offset)
		{

			m_commandList->DrawIndexedInstanced(index_count, instance_count, index_offset, vertex_offset, instance_offset);
		}

		void CommandList::set_index_buffer(HAL::Views::IndexBuffer index)
		{
			D3D12_INDEX_BUFFER_VIEW native;
			native.SizeInBytes = index.Resource ? index.SizeInBytes : 0;
			native.Format = ::to_native(index.Format);
			native.BufferLocation = index.Resource ? to_native(index.Resource->get_resource_address().offset(index.OffsetInBytes)) : 0;// index.Resource ? static_cast<HAL::Resource*>(index.Resource)->get_resource_address() + index.OffsetInBytes : 0;
			m_commandList->IASetIndexBuffer(&native);
		}

		void CommandList::graphics_set_const_buffer(UINT i, const ResourceAddress& adress)
		{
			m_commandList->SetGraphicsRootConstantBufferView(i, to_native(adress));
		}

		void CommandList::compute_set_const_buffer(UINT i, const ResourceAddress& adress)
		{
			m_commandList->SetComputeRootConstantBufferView(i, to_native(adress));
		}

		void CommandList::graphics_set_constant(UINT i, UINT offset, UINT value)
		{
			m_commandList->SetGraphicsRoot32BitConstant(i, value, offset);
		}

		void CommandList::compute_set_constant(UINT i, UINT offset, UINT value)
		{
			m_commandList->SetComputeRoot32BitConstant(i, value, offset);
		}

		void CommandList::dispatch_mesh(ivec3 v)
		{
			m_commandList->DispatchMesh(v.x, v.y, v.z);
		}

		void CommandList::dispatch(ivec3 v)
		{
			m_commandList->Dispatch(v.x, v.y, v.z);
		}

		void CommandList::set_scissors(sizer_long rect)
		{
			m_commandList->RSSetScissorRects(1, reinterpret_cast<D3D12_RECT*>(&rect));
		}

		void CommandList::set_viewports(std::vector<Viewport> viewports)
		{

			std::vector<D3D12_VIEWPORT> vps(viewports.size());

			for (uint i = 0; i < vps.size(); i++)
			{
				vps[i] = to_native(viewports[i]);
			}
			m_commandList->RSSetViewports(static_cast<UINT>(vps.size()), vps.data());
		}

		void CommandList::copy_resource(HAL::Resource* dest, HAL::Resource* source)
		{
			m_commandList->CopyResource(dest->get_dx(), source->get_dx());
		}

		void CommandList::copy_buffer(HAL::Resource* dest, uint64 dest_offset, HAL::Resource* source, uint64 source_offset, uint64 size)
		{

			if constexpr (Debug::CheckErrors)
			{
				auto source_size = source->get_size();
				auto dest_size = dest->get_size();

				assert(dest_offset + size <= dest_size);
				assert(source_offset + size <= source_size);

			}
			m_commandList->CopyBufferRegion(dest->get_dx(), dest_offset, source->get_dx(), source_offset, size);
		}

		void CommandList::set_pipeline(PipelineStateBase* pipeline)
		{

			auto pso = pipeline->get_native();
			if (pso)	m_commandList->SetPipelineState(pso.Get());
			else
			{
				auto state = pipeline->get_native_state();
				m_commandList->SetPipelineState1(state.Get());
			}


		}

		void CommandList::execute_indirect(const IndirectCommand& command_types, UINT max_commands, Resource* command_buffer, UINT64 command_offset, Resource* counter_buffer, UINT64 counter_offset)
		{
			m_commandList->ExecuteIndirect(
				command_types.command_signature.Get(),
				max_commands,
				command_buffer ? command_buffer->get_dx() : nullptr,
				command_offset,
				counter_buffer ? counter_buffer->get_dx() : nullptr,
				counter_offset);
		}

		void CommandList::set_rtv(int c, RTVHandle rt, DSVHandle h)
		{

			CD3DX12_CPU_DESCRIPTOR_HANDLE rtv;
			CD3DX12_CPU_DESCRIPTOR_HANDLE dsv;
			CD3DX12_CPU_DESCRIPTOR_HANDLE* dsv_ptr = nullptr;
			CD3DX12_CPU_DESCRIPTOR_HANDLE* rtv_ptr = nullptr;

			if (rt.is_valid())
			{
				rtv = rt.get_cpu();
				rtv_ptr = &rtv;
			}


			if (h.is_valid())
			{
				dsv = h.get_cpu();
				dsv_ptr = &dsv;
			}

			m_commandList->OMSetRenderTargets(c, rtv_ptr, true, dsv_ptr);
		}

		void CommandList::start_event(std::wstring_view str)
		{
			if constexpr (HAL::Debug::RunForPix) PIXBeginEvent(m_commandList.Get(), 0, str.data());
		}

		void CommandList::end_event()
		{
			if constexpr (HAL::Debug::RunForPix) PIXEndEvent(m_commandList.Get());
		}

		void CommandList::build_ras(const HAL::RaytracingBuildDescStructure& build_desc, const HAL::RaytracingBuildDescBottomInputs& bottom)
		{
			auto _bottom = to_native(bottom);
			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC desc = {};

			desc.DestAccelerationStructureData = to_native(build_desc.DestAccelerationStructureData);
			desc.SourceAccelerationStructureData = to_native(build_desc.SourceAccelerationStructureData);
			desc.ScratchAccelerationStructureData = to_native(build_desc.ScratchAccelerationStructureData);

			desc.Inputs = _bottom;

			m_commandList->BuildRaytracingAccelerationStructure(&desc, 0, nullptr);
		}

		void CommandList::build_ras(const HAL::RaytracingBuildDescStructure& build_desc, const HAL::RaytracingBuildDescTopInputs& top)
		{
			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC desc = {};

			desc.DestAccelerationStructureData = to_native(build_desc.DestAccelerationStructureData);
			desc.SourceAccelerationStructureData = to_native(build_desc.SourceAccelerationStructureData);
			desc.ScratchAccelerationStructureData = to_native(build_desc.ScratchAccelerationStructureData);

			desc.Inputs = to_native(top);

			m_commandList->BuildRaytracingAccelerationStructure(&desc, 0, nullptr);

		}

		void CommandList::copy_texture(const Resource::ptr& dest, int dest_subres, const Resource::ptr& source, int source_subres)
		{
			CD3DX12_TEXTURE_COPY_LOCATION Dst(dest->get_dx(), dest_subres);
			CD3DX12_TEXTURE_COPY_LOCATION Src(source->get_dx(), source_subres);
			m_commandList->CopyTextureRegion(&Dst, 0, 0, 0, &Src, nullptr);
		}

		void CommandList::copy_texture(const Resource::ptr& to, ivec3 to_pos, const Resource::ptr& from, ivec3 from_pos, ivec3 size)
		{
			CD3DX12_TEXTURE_COPY_LOCATION Dst(to->get_dx(), 0);
			CD3DX12_TEXTURE_COPY_LOCATION Src(from->get_dx(), 0);

			D3D12_BOX box;
			box.left = from_pos.x;
			box.top = from_pos.y;
			box.front = from_pos.z;


			box.right = from_pos.x + size.x;
			box.bottom = from_pos.y + size.y;
			box.back = from_pos.z + size.z;
			m_commandList->CopyTextureRegion(&Dst, to_pos.x, to_pos.y, to_pos.z, &Src, &box);
		}

		void CommandList::update_texture(HAL::Resource* resource, ivec3 offset, ivec3 box, UINT sub_resource, ResourceAddress address, texture_layout layout)
		{
			auto buffer_desc = address.resource->get_desc().as_buffer();
			auto size = box.z * box.y * layout.row_stride;

			assert(address.resource_offset + size <= buffer_desc.SizeInBytes);
			assert(box.z > 0);
			CD3DX12_TEXTURE_COPY_LOCATION Dst(resource->get_dx(), sub_resource);
			CD3DX12_TEXTURE_COPY_LOCATION Src;
			Src.pResource = address.resource->get_dx();
			Src.Type = D3D12_TEXTURE_COPY_TYPE::D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
			Src.PlacedFootprint.Offset = address.resource_offset;
			Src.PlacedFootprint.Footprint.Width = box.x;
			Src.PlacedFootprint.Footprint.Height = box.y;
			Src.PlacedFootprint.Footprint.Depth = box.z;
			Src.PlacedFootprint.Footprint.RowPitch = layout.row_stride;
			Src.PlacedFootprint.Footprint.Format = ::to_native(layout.format.to_srv());
			m_commandList->CopyTextureRegion(&Dst, offset.x, offset.y, offset.z, &Src, nullptr);
		}

		void CommandList::read_texture(const  HAL::Resource* resource, ivec3 offset, ivec3 box, UINT sub_resource, ResourceAddress target, texture_layout layout)
		{
			CD3DX12_TEXTURE_COPY_LOCATION source(resource->get_dx(), sub_resource);
			CD3DX12_TEXTURE_COPY_LOCATION dest;
			if (box.z == 0) box.z = 1;
			if (box.y == 0) box.y = 1;

			dest.pResource = target.resource->get_dx();
			dest.Type = D3D12_TEXTURE_COPY_TYPE::D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
			dest.PlacedFootprint.Offset = target.resource_offset;
			dest.PlacedFootprint.Footprint.Width = box.x;
			dest.PlacedFootprint.Footprint.Height = box.y;
			dest.PlacedFootprint.Footprint.Depth = box.z;
			dest.PlacedFootprint.Footprint.RowPitch = static_cast<UINT>(layout.row_stride);
			dest.PlacedFootprint.Footprint.Format = ::to_native(layout.format.to_srv());
			m_commandList->CopyTextureRegion(&dest, offset.x, offset.y, offset.z, &source, nullptr);
		}

		void CommandList::dispatch_rays(uint hit_size, uint miss_size, uint raygen_sige, ivec2 size, HAL::ResourceAddress hit_buffer, UINT hit_count, HAL::ResourceAddress miss_buffer, UINT miss_count, HAL::ResourceAddress raygen_buffer)
		{
			D3D12_DISPATCH_RAYS_DESC dispatchDesc = {};
			// Since each shader table has only one shader record, the stride is same as the size.
			dispatchDesc.HitGroupTable.StartAddress = to_native(hit_buffer);
			dispatchDesc.HitGroupTable.SizeInBytes = hit_size * hit_count;
			dispatchDesc.HitGroupTable.StrideInBytes = hit_size;

			dispatchDesc.MissShaderTable.StartAddress = to_native(miss_buffer);
			dispatchDesc.MissShaderTable.SizeInBytes = miss_size * miss_count;
			dispatchDesc.MissShaderTable.StrideInBytes = miss_size;

			dispatchDesc.RayGenerationShaderRecord.StartAddress = to_native(raygen_buffer);
			dispatchDesc.RayGenerationShaderRecord.SizeInBytes = raygen_sige;
			dispatchDesc.Width = size.x;
			dispatchDesc.Height = size.y;
			dispatchDesc.Depth = 1;

			m_commandList->DispatchRays(&dispatchDesc);
		}

		void CommandList::transitions(const HAL::Barriers& _barriers)
		{
			auto& barriers = _barriers.get_barriers();
		//	if (barriers.empty())return;


			std::vector<D3D12_TEXTURE_BARRIER> textures;
			std::vector<D3D12_BUFFER_BARRIER> buffers;

			std::vector<D3D12_GLOBAL_BARRIER> global;



			for (auto& e : barriers)
			{


				if (e.resource->get_desc().is_buffer())
				{
					D3D12_BUFFER_BARRIER barrier;



					barrier.SyncBefore = to_native(e.before.get_operation());
					barrier.SyncAfter = to_native(e.after.get_operation());
					barrier.AccessBefore = to_native(e.before.get_access());
					barrier.AccessAfter = to_native(e.after.get_access());
					barrier.pResource = e.resource->get_dx();
					barrier.Offset = 0;
					barrier.Size = e.resource->get_desc().as_buffer().SizeInBytes; // Must be UINT64_MAX or buffer size in bytes

					buffers.emplace_back(barrier);
				}



				if (e.resource->get_desc().is_texture())
				{
					D3D12_TEXTURE_BARRIER barrier;



					barrier.SyncBefore = to_native(e.before.get_operation());
					barrier.SyncAfter = to_native(e.after.get_operation());
					barrier.AccessBefore = to_native(e.before.get_access());
					barrier.AccessAfter = to_native(e.after.get_access());
					barrier.LayoutBefore = to_native(e.before.get_layout());
					barrier.LayoutAfter = to_native(e.after.get_layout());

					barrier.pResource = e.resource->get_dx();
					//barrier.Offset = 0;
				   ///   barrier.Size=e.resource->get_desc().as_buffer().SizeInBytes; // Must be UINT64_MAX or buffer size in bytes



					barrier.Subresources.IndexOrFirstMipLevel = e.resource->get_desc().as_texture().get_mip(e.subres);
					barrier.Subresources.NumMipLevels = 1;
					barrier.Subresources.FirstArraySlice = e.resource->get_desc().as_texture().get_array(e.subres);
					barrier.Subresources.NumArraySlices = 1;
					barrier.Subresources.FirstPlane = e.resource->get_desc().as_texture().get_plane(e.subres);
					barrier.Subresources.NumPlanes = 1;


					barrier.Flags = D3D12_TEXTURE_BARRIER_FLAG_NONE;
					if (check(e.flags &BarrierFlags::DISCARD)) 
						barrier.Flags|= D3D12_TEXTURE_BARRIER_FLAG_DISCARD ;
									

					textures.emplace_back(barrier);
				}



			}


			std::vector<D3D12_BARRIER_GROUP> native;



			if (!buffers.empty())
			{
				D3D12_BARRIER_GROUP group;

				group.Type = D3D12_BARRIER_TYPE::D3D12_BARRIER_TYPE_BUFFER;
				group.NumBarriers = buffers.size();
				group.pBufferBarriers = buffers.data();

				native.emplace_back(group);
			}


			if (!textures.empty())
			{
				D3D12_BARRIER_GROUP group;

				group.Type = D3D12_BARRIER_TYPE::D3D12_BARRIER_TYPE_TEXTURE;
				group.NumBarriers = textures.size();
				group.pTextureBarriers = textures.data();

				native.emplace_back(group);
			}






			if (GetAsyncKeyState('8'))
			{


				
					D3D12_GLOBAL_BARRIER barrier;

						barrier.SyncBefore =
						barrier.SyncAfter =
						D3D12_BARRIER_SYNC_ALL_SHADING |
						D3D12_BARRIER_SYNC_BUILD_RAYTRACING_ACCELERATION_STRUCTURE |
						D3D12_BARRIER_SYNC_COPY_RAYTRACING_ACCELERATION_STRUCTURE |
						D3D12_BARRIER_SYNC_EMIT_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO;

					barrier.AccessBefore =
						barrier.AccessAfter =
						D3D12_BARRIER_ACCESS_UNORDERED_ACCESS |
						D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_WRITE |
						D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_READ;

					/*	barrier.SyncBefore =D3D12_BARRIER_SYNC_ALL;
						barrier.SyncAfter = D3D12_BARRIER_SYNC_ALL;
						barrier.AccessBefore = to_native(e.before.get_access());
						barrier.AccessAfter = to_native(e.after.get_access());*/

					global.emplace_back(barrier);
				
				D3D12_BARRIER_GROUP group;

				group.Type = D3D12_BARRIER_TYPE::D3D12_BARRIER_TYPE_GLOBAL;
				group.NumBarriers = global.size();
				group.pGlobalBarriers = global.data();

				native.emplace_back(group);
			}




			/*auto& barriers = _barriers.get_barriers();
			if (!barriers.empty())
			{

				std::vector<D3D12_RESOURCE_BARRIER> native(barriers.size());

				for (uint i = 0; i < native.size(); i++)
				{

					auto& e = barriers[i];
					std::visit(overloaded{
				[&](const BarrierUAV& barrier) {
				native[i] = (CD3DX12_RESOURCE_BARRIER::UAV((barrier.resource)->get_dx()));
				},
				[&](const BarrierAlias& barrier) {
					auto native_from = barrier.before ? (barrier.before)->get_dx() : nullptr;
					auto native_to = barrier.after ? (barrier.after)->get_dx() : nullptr;
					native[i] = (CD3DX12_RESOURCE_BARRIER::Aliasing(native_from, native_to));
				},[&](const BarrierTransition& barrier) {
					D3D12_RESOURCE_BARRIER_FLAGS native_flags = D3D12_RESOURCE_BARRIER_FLAGS::D3D12_RESOURCE_BARRIER_FLAG_NONE;

										if (barrier.flags == BarrierFlags::BEGIN) native_flags = D3D12_RESOURCE_BARRIER_FLAGS::D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY;
										if (barrier.flags == BarrierFlags::END) native_flags = D3D12_RESOURCE_BARRIER_FLAGS::D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;

										native[i] = (CD3DX12_RESOURCE_BARRIER::Transition((barrier.resource)->get_dx(),
											static_cast<D3D12_RESOURCE_STATES>(barrier.before),
											static_cast<D3D12_RESOURCE_STATES>(barrier.after),
											barrier.subres == ALL_SUBRESOURCES ? D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES : barrier.subres,
											native_flags));
				},
				[&](auto other) {
					assert(false);
				}
						}, e);



				}*/

			if (!native.empty())
			{
				m_commandList->Barrier((UINT)native.size(), native.data());

			}

			//}
		}
	}
}