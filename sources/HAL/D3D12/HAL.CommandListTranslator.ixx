export module HAL:Private.CommandListTranslator;
import Core; 
import d3d12;

import :API.Device;
import :Private.CommandListCompiler;

import :ResourceStates;
import :Resource;
import :DescriptorHeap;
import :Fence;
import :FrameManager;
import :PipelineState;
import :API.IndirectCommand;

import :QueryHeap;

import :Types;
import :Debug;

import d3d12;

export namespace HAL
{

	namespace Private
	{

		template<class T>
		class CommandListTranslator
		{
			T compiler;

			D3D_PRIMITIVE_TOPOLOGY  native_topology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;

		public:
			CommandListTranslator()
			{

			}
			CommandListTranslator(CommandListCompiled& t):compiler(t)
			{

			}
			void create(HAL::CommandListType type)
			{
				compiler.create(type);
			}

			void reset()
			{
				native_topology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
				compiler.reset();
			}
			CommandListCompiled compile()
			{
				return compiler.compile();
			}


			void func(std::function<void(CommandListTranslator<CommandListCompiler>&)> f)
			{
				

				compiler.func([f](CommandListCompiled& c)
				{

						CommandListTranslator<CommandListCompiler> translator(c);

						f(translator);

				});
			}
			void clear_uav(const Handle& h, vec4 ClearColor)
			{
				auto v= h.get_resource_info().view;
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
				compiler.ClearUnorderedAccessViewFloat(h.get_gpu(), h.get_cpu(), dx_resource, reinterpret_cast<FLOAT*>(ClearColor.data()), 0, nullptr);
			}


			void clear_rtv(const Handle& h, vec4 ClearColor)
			{
				compiler.ClearRenderTargetView(h.get_cpu(), ClearColor.data(), 0, nullptr);
			}

			void clear_stencil(const Handle& dsv, UINT8 stencil)
			{
				compiler.ClearDepthStencilView(dsv.get_cpu(), D3D12_CLEAR_FLAG_STENCIL, 0, stencil, 0, nullptr);
			}

			void clear_depth(const Handle& dsv, float depth )
			{
				compiler.ClearDepthStencilView(dsv.get_cpu(), D3D12_CLEAR_FLAG_DEPTH, depth, 0, 0, nullptr);
			}

			void set_topology(HAL::PrimitiveTopologyType topology, HAL::PrimitiveTopologyFeed feedType = HAL::PrimitiveTopologyFeed::LIST, bool adjusted = false, uint controlpoints = 0)
			{
				auto native = to_native(topology, feedType, adjusted, controlpoints);
				if (this->native_topology != native)
					compiler.IASetPrimitiveTopology(native);

				this->native_topology = native;
			}

			void set_stencil_ref(UINT ref)
			{
				compiler.OMSetStencilRef(ref);
			}
			template<class Hit, class Miss, class Raygen>
			void dispatch_rays(ivec2 size, HAL::ResourceAddress hit_buffer, UINT hit_count, HAL::ResourceAddress miss_buffer, UINT miss_count, HAL::ResourceAddress raygen_buffer)
			{
				D3D12_DISPATCH_RAYS_DESC dispatchDesc = {};
				// Since each shader table has only one shader record, the stride is same as the size.
				dispatchDesc.HitGroupTable.StartAddress = to_native(hit_buffer);
				dispatchDesc.HitGroupTable.SizeInBytes = sizeof(Hit) * hit_count;
				dispatchDesc.HitGroupTable.StrideInBytes = sizeof(Hit);

				dispatchDesc.MissShaderTable.StartAddress = to_native(miss_buffer);
				dispatchDesc.MissShaderTable.SizeInBytes = sizeof(Miss) * miss_count;
				dispatchDesc.MissShaderTable.StrideInBytes = sizeof(Miss);

				dispatchDesc.RayGenerationShaderRecord.StartAddress = to_native(raygen_buffer);
				dispatchDesc.RayGenerationShaderRecord.SizeInBytes = sizeof(Raygen);
				dispatchDesc.Width = size.x;
				dispatchDesc.Height = size.y;
				dispatchDesc.Depth = 1;

				compiler.DispatchRays(&dispatchDesc);
			}



		void SetName(
				LPCWSTR Name)
			{
			compiler.SetName(Name);
			};

			void set_descriptor_heaps(DescriptorHeap* cbv, DescriptorHeap* sampler)
			{
				std::array<ID3D12DescriptorHeap*, 2> heaps;

				heaps[0] = sampler? sampler->get_dx():nullptr;
				heaps[1] = cbv ? cbv->get_dx() : nullptr;
				compiler.SetDescriptorHeaps(2, heaps.data());
			}


			void insert_time(const QueryHandle& handle, uint offset)
			{
				compiler.EndQuery(handle.get_heap()->get_native().Get(), D3D12_QUERY_TYPE_TIMESTAMP, handle.get_offset() + offset);
			}

			void resolve_times(QueryHeap& pQueryHeap, uint32_t NumQueries, ResourceAddress destination)
			{
				compiler.ResolveQueryData(pQueryHeap.get_native().Get(), D3D12_QUERY_TYPE_TIMESTAMP, 0, NumQueries, destination.resource->get_dx(), destination.resource_offset);
			}


			void set_graphics_signature(const RootSignature::ptr& s)
			{
				compiler.SetGraphicsRootSignature(s->get_native().Get());
			}

			void set_compute_signature(const RootSignature::ptr& s)
			{
				compiler.SetComputeRootSignature(s->get_native().Get());
			}

			void draw(UINT vertex_count, UINT vertex_offset, UINT instance_count, UINT instance_offset)
			{
				compiler.DrawInstanced(vertex_count, instance_count, vertex_offset, instance_offset);
			}
			void draw_indexed(UINT index_count, UINT index_offset, UINT vertex_offset, UINT instance_count, UINT instance_offset)
			{

				compiler.DrawIndexedInstanced(index_count, instance_count, index_offset, vertex_offset, instance_offset);
			}

			void set_index_buffer(HAL::Views::IndexBuffer index)
			{
				D3D12_INDEX_BUFFER_VIEW native;
				native.SizeInBytes = index.Resource ? index.SizeInBytes : 0;
				native.Format = ::to_native(index.Format);
				native.BufferLocation = index.Resource ? to_native(index.Resource->get_resource_address().offset(index.OffsetInBytes)) : 0;// index.Resource ? static_cast<HAL::Resource*>(index.Resource)->get_resource_address() + index.OffsetInBytes : 0;
				compiler.IASetIndexBuffer(&native);
			}

			void  graphics_set_const_buffer(UINT i, const ResourceAddress& adress)
			{
				compiler.SetGraphicsRootConstantBufferView(i, to_native(adress));
			}

			void  compute_set_const_buffer(UINT i, const ResourceAddress& adress)
			{
				compiler.SetComputeRootConstantBufferView(i, to_native(adress));
			}


			void  graphics_set_constant(UINT i, UINT offset, UINT value)
			{
				compiler.SetGraphicsRoot32BitConstant(i, value, offset);
			}

			void  compute_set_constant(UINT i, UINT offset, UINT value)
			{
				compiler.SetComputeRoot32BitConstant(i, value, offset);
			}

			void dispatch_mesh(ivec3 v)
			{

			
				compiler.DispatchMesh(v.x, v.y, v.z);
				}

			void dispatch(ivec3 v)
			{
				compiler.Dispatch(v.x, v.y, v.z);
			}
			void set_scissors(sizer_long rect)
			{
				compiler.RSSetScissorRects(1, reinterpret_cast<D3D12_RECT*>(&rect));
			}

			void set_viewports(std::vector<Viewport> viewports)
			{

				std::vector<D3D12_VIEWPORT> vps(viewports.size());

				for (uint i = 0; i < vps.size(); i++)
				{
					vps[i] = to_native(viewports[i]);
				}
				compiler.RSSetViewports(static_cast<UINT>(vps.size()), vps.data());
			}

			void copy_resource(Resource* dest, Resource* source)
			{
				compiler.CopyResource(dest->get_dx(), source->get_dx());
			}

			void  copy_buffer(Resource* dest, UINT dest_offset, Resource* source ,UINT source_offset,UINT size )
			{
				
		if constexpr (Debug::CheckErrors)
		{
			auto source_size = source->get_size();
			auto dest_size = dest->get_size();

			assert(dest_offset+size<=dest_size);
			assert(source_offset+size<=source_size);

		}
				compiler.CopyBufferRegion(dest->get_dx(), dest_offset, source->get_dx(), source_offset, size);
			}

			void set_pipeline(PipelineStateBase* pipeline)
			{

						auto pso = pipeline->get_native();
						if (pso)	compiler.SetPipelineState(pso.Get());
						else
						{
							auto state = pipeline->get_native_state();
							compiler.SetPipelineState1(state.Get());
						}

					
			}
			void execute_indirect(IndirectCommand& command_types, UINT max_commands, Resource* command_buffer, UINT64 command_offset, Resource* counter_buffer, UINT64 counter_offset)
			{
				compiler.ExecuteIndirect(
					command_types.command_signature.Get(),
					max_commands,
					command_buffer ? command_buffer->get_dx() : nullptr,
					command_offset,
					counter_buffer ? counter_buffer->get_dx() : nullptr,
					counter_offset);
			}


			void set_rtv(int c, Handle rt, Handle h)
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

				compiler.OMSetRenderTargets(c, rtv_ptr, true, dsv_ptr);
			}


			void start_event(std::wstring str)
			{
					compiler.start_event(str);
			}

			void end_event()
			{
				compiler.end_event();
			}


			void build_ras(const HAL::RaytracingBuildDescStructure& build_desc, const HAL::RaytracingBuildDescBottomInputs& bottom)
			{

				auto _bottom = to_native(bottom);
				D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC desc = {};

				desc.DestAccelerationStructureData = to_native(build_desc.DestAccelerationStructureData);
				desc.SourceAccelerationStructureData = to_native(build_desc.SourceAccelerationStructureData);
				desc.ScratchAccelerationStructureData = to_native(build_desc.ScratchAccelerationStructureData);

				desc.Inputs = _bottom;

				compiler.BuildRaytracingAccelerationStructure(&desc, 0, nullptr);

			}

			void build_ras(const HAL::RaytracingBuildDescStructure& build_desc, const HAL::RaytracingBuildDescTopInputs& top)
			{
				D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC desc = {};

				desc.DestAccelerationStructureData = to_native(build_desc.DestAccelerationStructureData);
				desc.SourceAccelerationStructureData = to_native(build_desc.SourceAccelerationStructureData);
				desc.ScratchAccelerationStructureData = to_native(build_desc.ScratchAccelerationStructureData);

				desc.Inputs = to_native(top);
	
				compiler.BuildRaytracingAccelerationStructure(&desc, 0, nullptr);

			}


			void copy_texture(const Resource::ptr& dest, int dest_subres, const Resource::ptr& source, int source_subres)
			{
				CD3DX12_TEXTURE_COPY_LOCATION Dst(dest->get_dx(), dest_subres);
				CD3DX12_TEXTURE_COPY_LOCATION Src(source->get_dx(), source_subres);
				compiler.CopyTextureRegion(&Dst, 0, 0, 0, &Src, nullptr);
			}

void copy_texture(const Resource::ptr& to, ivec3 to_pos, const Resource::ptr& from, ivec3 from_pos, ivec3 size)
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
	compiler.CopyTextureRegion(&Dst, to_pos.x, to_pos.y, to_pos.z, &Src, &box);
			}


struct texture_layout
{
	uint64 size;
	uint rows_count;
	uint64 row_stride;
	uint64 slice_stride;

	uint alignment;

	Format format;
};

texture_layout get_texture_layout(const Resource* resource, UINT sub_resource, ivec3 box)
{
	UINT rows_count = box.y;

	auto desc = resource->get_desc().as_texture();
	if (desc.Format ==Format::BC7_UNORM_SRGB || desc.Format == Format::BC7_UNORM)
		rows_count /= 4;
	UINT64 RequiredSize = 0;
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT Layouts;
	UINT NumRows;
	UINT64 RowSizesInBytes;
	D3D12_RESOURCE_DESC Desc = ::to_native(resource->get_desc());
	HAL::Device::get().get_native_device()->GetCopyableFootprints(&Desc, sub_resource, 1, 0, &Layouts, &NumRows, &RowSizesInBytes, &RequiredSize);
	UINT64 res_stride = Math::AlignUp(RowSizesInBytes, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
	UINT64 size = res_stride * rows_count * box.z;

	return { size , rows_count , res_stride , res_stride * rows_count,D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT, from_native(Layouts.Footprint.Format) };
}
			void update_texture(Resource* resource, ivec3 offset, ivec3 box, UINT sub_resource, ResourceAddress address, texture_layout layout)
			{

			
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
				compiler.CopyTextureRegion(&Dst, offset.x, offset.y, offset.z, &Src, nullptr);
			}

			
			void read_texture(const Resource* resource, ivec3 offset, ivec3 box, UINT sub_resource, ResourceAddress target, texture_layout layout)
			{
				CD3DX12_TEXTURE_COPY_LOCATION source(resource->get_dx(), sub_resource);
				CD3DX12_TEXTURE_COPY_LOCATION dest;

				assert(box.z > 0);
				dest.pResource = target.resource->get_dx();
				dest.Type = D3D12_TEXTURE_COPY_TYPE::D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
				dest.PlacedFootprint.Offset = target.resource_offset;
				dest.PlacedFootprint.Footprint.Width = box.x;
				dest.PlacedFootprint.Footprint.Height = box.y;
				dest.PlacedFootprint.Footprint.Depth = box.z;
				dest.PlacedFootprint.Footprint.RowPitch = static_cast<UINT>(layout.row_stride);
				dest.PlacedFootprint.Footprint.Format = ::to_native(layout.format.to_srv());
				compiler.CopyTextureRegion(&dest, offset.x, offset.y, offset.z, &source, nullptr);
			}


			void transitions(HAL::Barriers& _barriers)
{
				auto& barriers = _barriers.get_barriers();
				if (!barriers.empty())
				{

					std::vector<D3D12_RESOURCE_BARRIER> native(barriers.size());

					for (uint i = 0; i < native.size(); i++)
					{

						auto& e=barriers[i];
						std::visit(overloaded{
					[&](const BarrierUAV& barrier) {
					native[i]=(CD3DX12_RESOURCE_BARRIER::UAV((barrier.resource)->get_dx()));
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
												barrier.subres== ALL_SUBRESOURCES? D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES:barrier.subres,
												native_flags));
					},
					[&](auto other) {
						assert(false);
					}
							}, e);



					}
				

					compiler.ResourceBarrier((UINT)native.size(), native.data());
				}
}
		};

		//	virtual HRESULT  SetName(
		//		LPCWSTR Name)
		//	{
		//		return 0;
		//	};

		//	virtual void  BuildRaytracingAccelerationStructure(
		//		const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC* pDesc,
		//		UINT NumPostbuildInfoDescs,
		//		const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_DESC* pPostbuildInfoDescs)
		//	{
		//		compiled.m_commandList->BuildRaytracingAccelerationStructure(pDesc, NumPostbuildInfoDescs, pPostbuildInfoDescs);
		//	};


		//	virtual void  SetPipelineState1(
		//		ID3D12StateObject* pStateObject)
		//	{
		//		compiled.m_commandList->SetPipelineState1(pStateObject);
		//	};

		//	virtual void  DispatchRays(
		//		const D3D12_DISPATCH_RAYS_DESC* pDesc) {
		//		compiled.m_commandList->DispatchRays(pDesc);
		//	};

		//	void  ClearState(
		//		ID3D12PipelineState* pPipelineState)
		//	{
		//		compiled.m_commandList->ClearState(pPipelineState);
		//	}

		//	void  DrawInstanced(
		//		UINT VertexCountPerInstance,
		//		UINT InstanceCount,
		//		UINT StartVertexLocation,
		//		UINT StartInstanceLocation)
		//	{
		//		compiled.m_commandList->DrawInstanced(VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
		//	}

		//	void  DrawIndexedInstanced(
		//		UINT IndexCountPerInstance,
		//		UINT InstanceCount,
		//		UINT StartIndexLocation,
		//		INT BaseVertexLocation,
		//		UINT StartInstanceLocation)
		//	{
		//		compiled.m_commandList->DrawIndexedInstanced(IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
		//	}

		//	void  Dispatch(
		//		UINT ThreadGroupCountX,
		//		UINT ThreadGroupCountY,
		//		UINT ThreadGroupCountZ)
		//	{
		//		compiled.m_commandList->Dispatch(ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
		//	}

		//	void  CopyBufferRegion(
		//		ID3D12Resource* pDstBuffer,
		//		UINT64 DstOffset,
		//		ID3D12Resource* pSrcBuffer,
		//		UINT64 SrcOffset,
		//		UINT64 NumBytes)
		//	{
		//		compiled.m_commandList->CopyBufferRegion(pDstBuffer, DstOffset, pSrcBuffer, SrcOffset, NumBytes);
		//	}

		//	void  CopyTextureRegion(
		//		const D3D12_TEXTURE_COPY_LOCATION* pDst,
		//		UINT DstX,
		//		UINT DstY,
		//		UINT DstZ,
		//		const D3D12_TEXTURE_COPY_LOCATION* pSrc,
		//		const D3D12_BOX* pSrcBox)
		//	{
		//		compiled.m_commandList->CopyTextureRegion(pDst, DstX, DstY, DstZ, pSrc, pSrcBox);
		//	}

		//	void  CopyResource(
		//		ID3D12Resource* pDstResource,
		//		ID3D12Resource* pSrcResource)
		//	{
		//		compiled.m_commandList->CopyResource(pDstResource, pSrcResource);
		//	}

		//	void  IASetPrimitiveTopology(
		//		D3D12_PRIMITIVE_TOPOLOGY PrimitiveTopologyType)
		//	{
		//		compiled.m_commandList->IASetPrimitiveTopology(PrimitiveTopologyType);
		//	}

		//	void  RSSetViewports(
		//		UINT NumViewports,
		//		const D3D12_VIEWPORT* pViewports) {
		//		compiled.m_commandList->RSSetViewports(NumViewports, pViewports);
		//	}

		//	void  RSSetScissorRects(
		//		UINT NumRects,
		//		const D3D12_RECT* pRects)
		//	{
		//		compiled.m_commandList->RSSetScissorRects(NumRects, pRects);
		//	}

		//	void  OMSetBlendFactor(
		//		const FLOAT BlendFactor[4]) {
		//		compiled.m_commandList->OMSetBlendFactor(BlendFactor);
		//	}

		//	void  OMSetStencilRef(
		//		UINT StencilRef) {
		//		compiled.m_commandList->OMSetStencilRef(StencilRef);
		//	}

		//	void  SetPipelineState(
		//		ID3D12PipelineState* pPipelineState) {
		//		compiled.m_commandList->SetPipelineState(pPipelineState);
		//	}

		//	void  ResourceBarrier(
		//		UINT NumBarriers,
		//		const D3D12_RESOURCE_BARRIER* pBarriers)
		//	{
		//		compiled.m_commandList->ResourceBarrier(NumBarriers, pBarriers);
		//	}

		//	void  ExecuteBundle(
		//		ID3D12GraphicsCommandList* pCommandList)
		//	{
		//		compiled.m_commandList->ExecuteBundle(pCommandList);
		//	}

		//	void  SetDescriptorHeaps(
		//		UINT NumDescriptorHeaps,
		//		ID3D12DescriptorHeap* const* ppDescriptorHeaps)
		//	{
		//		compiled.m_commandList->SetDescriptorHeaps(NumDescriptorHeaps, ppDescriptorHeaps);
		//	}

		//	void  SetComputeRootSignature(
		//		ID3D12RootSignature* pRootSignature)
		//	{
		//		compiled.m_commandList->SetComputeRootSignature(pRootSignature);
		//	}

		//	void  SetGraphicsRootSignature(
		//		ID3D12RootSignature* pRootSignature)
		//	{
		//		compiled.m_commandList->SetGraphicsRootSignature(pRootSignature);
		//	}

		//	void  SetComputeRootDescriptorTable(
		//		UINT RootParameterIndex,
		//		D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor)
		//	{
		//		compiled.m_commandList->SetComputeRootDescriptorTable(RootParameterIndex, BaseDescriptor);
		//	}

		//	void  SetGraphicsRootDescriptorTable(
		//		UINT RootParameterIndex,
		//		D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor)
		//	{
		//		compiled.m_commandList->SetGraphicsRootDescriptorTable(RootParameterIndex, BaseDescriptor);
		//	}

		//	virtual void  SetComputeRootConstantBufferView(
		//		UINT RootParameterIndex,
		//		D3D12_GPU_VIRTUAL_ADDRESS BufferLocation)
		//	{
		//		compiled.m_commandList->SetComputeRootConstantBufferView(RootParameterIndex, BufferLocation);
		//	}


		//	virtual void  SetGraphicsRootConstantBufferView(
		//		UINT RootParameterIndex,
		//		D3D12_GPU_VIRTUAL_ADDRESS BufferLocation)
		//	{
		//		compiled.m_commandList->SetGraphicsRootConstantBufferView(RootParameterIndex, BufferLocation);
		//	}


		//	void  IASetIndexBuffer(
		//		const D3D12_INDEX_BUFFER_VIEW* pView)
		//	{
		//		compiled.m_commandList->IASetIndexBuffer(pView);
		//	}

		//	void  IASetVertexBuffers(
		//		UINT StartSlot,
		//		UINT NumViews,
		//		const D3D12_VERTEX_BUFFER_VIEW* pViews)
		//	{
		//		compiled.m_commandList->IASetVertexBuffers(StartSlot, NumViews, pViews);
		//	}

		//	void  SOSetTargets(
		//		UINT StartSlot,
		//		UINT NumViews,
		//		const D3D12_STREAM_OUTPUT_BUFFER_VIEW* pViews)
		//	{
		//		compiled.m_commandList->SOSetTargets(StartSlot, NumViews, pViews);
		//	}

		//	void  OMSetRenderTargets(
		//		UINT NumRenderTargetDescriptors,
		//		const D3D12_CPU_DESCRIPTOR_HANDLE* pRenderTargetDescriptors,
		//		BOOL RTsSingleHandleToDescriptorRange,
		//		const D3D12_CPU_DESCRIPTOR_HANDLE* pDepthStencilDescriptor)
		//	{
		//		compiled.m_commandList->OMSetRenderTargets(NumRenderTargetDescriptors, pRenderTargetDescriptors, RTsSingleHandleToDescriptorRange, pDepthStencilDescriptor);
		//	}

		//	void  ClearDepthStencilView(
		//		D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView,
		//		D3D12_CLEAR_FLAGS ClearFlags,
		//		FLOAT Depth,
		//		UINT8 Stencil,
		//		UINT NumRects,
		//		const D3D12_RECT* pRects)
		//	{
		//		compiled.m_commandList->ClearDepthStencilView(DepthStencilView, ClearFlags, Depth, Stencil, NumRects, pRects);
		//	}

		//	void  ClearRenderTargetView(
		//		D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetView,
		//		const FLOAT ColorRGBA[4],
		//		UINT NumRects,
		//		const D3D12_RECT* pRects)
		//	{
		//		compiled.m_commandList->ClearRenderTargetView(RenderTargetView, ColorRGBA, NumRects, pRects);
		//	}

		//	void  ClearUnorderedAccessViewUint(
		//		D3D12_GPU_DESCRIPTOR_HANDLE ViewGPUHandleInCurrentHeap,
		//		D3D12_CPU_DESCRIPTOR_HANDLE ViewCPUHandle,
		//		ID3D12Resource* pResource,
		//		const UINT Values[4],
		//		UINT NumRects,
		//		const D3D12_RECT* pRects)
		//	{
		//		compiled.m_commandList->ClearUnorderedAccessViewUint(ViewGPUHandleInCurrentHeap, ViewCPUHandle, pResource, Values, NumRects, pRects);
		//	}

		//	void  ClearUnorderedAccessViewFloat(
		//		D3D12_GPU_DESCRIPTOR_HANDLE ViewGPUHandleInCurrentHeap,
		//		D3D12_CPU_DESCRIPTOR_HANDLE ViewCPUHandle,
		//		ID3D12Resource* pResource,
		//		const FLOAT Values[4],
		//		UINT NumRects,
		//		const D3D12_RECT* pRects)
		//	{
		//		compiled.m_commandList->ClearUnorderedAccessViewFloat(ViewGPUHandleInCurrentHeap, ViewCPUHandle, pResource, Values, NumRects, pRects);
		//	}

		//	void  DiscardResource(
		//		ID3D12Resource* pResource,
		//		const D3D12_DISCARD_REGION* pRegion)
		//	{
		//		compiled.m_commandList->DiscardResource(pResource, pRegion);
		//	}

		//	void  BeginQuery(
		//		ID3D12QueryHeap* pQueryHeap,
		//		D3D12_QUERY_TYPE Type,
		//		UINT Index)
		//	{
		//		compiled.m_commandList->BeginQuery(pQueryHeap, Type, Index);
		//	}

		//	void  EndQuery(
		//		ID3D12QueryHeap* pQueryHeap,
		//		D3D12_QUERY_TYPE Type,
		//		UINT Index)
		//	{
		//		compiled.m_commandList->EndQuery(pQueryHeap, Type, Index);
		//	}

		//	void  ResolveQueryData(
		//		ID3D12QueryHeap* pQueryHeap,
		//		D3D12_QUERY_TYPE Type,
		//		UINT StartIndex,
		//		UINT NumQueries,
		//		ID3D12Resource* pDestinationBuffer,
		//		UINT64 AlignedDestinationBufferOffset)
		//	{
		//		compiled.m_commandList->ResolveQueryData(pQueryHeap, Type, StartIndex, NumQueries, pDestinationBuffer, AlignedDestinationBufferOffset);
		//	}

		//	void  SetPredication(
		//		ID3D12Resource* pBuffer,
		//		UINT64 AlignedBufferOffset,
		//		D3D12_PREDICATION_OP Operation)
		//	{
		//		compiled.m_commandList->SetPredication(pBuffer, AlignedBufferOffset, Operation);
		//	}

		//	void  SetMarker(
		//		UINT Metadata,
		//		const void* pData,
		//		UINT Size)
		//	{
		//		compiled.m_commandList->SetMarker(Metadata, pData, Size);
		//	}

		//	void  BeginEvent(
		//		UINT Metadata,
		//		const void* pData,
		//		UINT Size)
		//	{
		//		compiled.m_commandList->BeginEvent(Metadata, pData, Size);
		//	}

		//	void  EndEvent(void)
		//	{
		//		compiled.m_commandList->EndEvent();
		//	}

		//	void  ExecuteIndirect(
		//		ID3D12CommandSignature* pCommandSignature,
		//		UINT MaxCommandCount,
		//		ID3D12Resource* pArgumentBuffer,
		//		UINT64 ArgumentBufferOffset,
		//		ID3D12Resource* pCountBuffer,
		//		UINT64 CountBufferOffset)
		//	{
		//		compiled.m_commandList->ExecuteIndirect(pCommandSignature, MaxCommandCount, pArgumentBuffer, ArgumentBufferOffset, pCountBuffer, CountBufferOffset);
		//	}

		//};


	}

}