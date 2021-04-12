#pragma once
#include "../../3rdparty/Aftermath/NsightAftermathGpuCrashTracker.h"

namespace DX12
{
	class Device : public Singleton<Device>
	{
		ComPtr<ID3D12Device5> m_device;
		ComPtr<IDXGIFactory4> factory;
		std::vector<std::shared_ptr<SwapChain>> swap_chains;
		std::vector <ComPtr<IDXGIAdapter3> > vAdapters;

		//   std::shared_ptr<CommandList> upload_list;
		enum_array<CommandListType, Queue::ptr> queues;
		IdGenerator<Thread::Lockable> id_generator;
		friend class CommandList;
		bool rtx = false;
		GpuCrashTracker crasher;
	public:
		void stop_all();
		virtual ~Device();
		ContextGenerator context_generator;
		std::shared_ptr<CommandList> get_upload_list();
		ComPtr<ID3D12Device5> get_native_device();

		Queue::ptr& get_queue(CommandListType type);

		size_t get_vram();

		Device();

		ResourceAllocationInfo get_alloc_info(CD3DX12_RESOURCE_DESC& desc);
		std::shared_ptr<SwapChain> create_swap_chain(const DX12::swap_chain_desc& desc);

		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS to_native(const RaytracingBuildDescBottomInputs& desc);
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS to_native(const RaytracingBuildDescTopInputs& desc);
		
			
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO calculateBuffers(const RaytracingBuildDescBottomInputs& desc);
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO calculateBuffers(const RaytracingBuildDescTopInputs& desc);

		/*
		TrackedResource::ptr create_resource(CD3DX12_RESOURCE_DESC desc, ResourceHandle handle, ResourceState state = ResourceState::UNKNOWN, float4 clear_value = {0,0,0,0})
		{
			auto result = std::make_shared<TrackedResource>();
			D3D12_CLEAR_VALUE value;
			value.Format = to_srv(desc.Format);
			value.Color[0] = clear_value.x;
			value.Color[1] = clear_value.y;
			value.Color[2] = clear_value.z;
			value.Color[3] = clear_value.w;

			if (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
			{
				value.Format = to_dsv(desc.Format);
				value.DepthStencil.Depth = 1.0f;
				value.DepthStencil.Stencil = 0;
			}

			if (!is_shader_visible(desc.Format))
				desc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;

			if (state == ResourceState::UNKNOWN)
			{
				if (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
					state = ResourceState::DEPTH_WRITE;
				else if (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET)
					state = ResourceState::RENDER_TARGET;
				else if (!(desc.Flags & D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE))
					state = ResourceState::PIXEL_SHADER_RESOURCE;

				else
					state = ResourceState::COMMON;
			}

			TEST(Device::get().get_native_device()->CreatePlacedResource(
				handle.get_heap()->heap.Get(),
				handle.get_offset(),
				&desc,
				static_cast<D3D12_RESOURCE_STATES>(state),
				(desc.Dimension == D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE2D && (desc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL))) ? &value : nullptr,
				IID_PPV_ARGS(&result.resource)));

			result.alloc_handle = handle;
			return result;

		}

		*/
		void create_sampler(D3D12_SAMPLER_DESC desc, CD3DX12_CPU_DESCRIPTOR_HANDLE handle);

		bool is_rtx_supported() { return rtx; }
		void create_rtv(Handle h, Resource* resource, D3D12_RENDER_TARGET_VIEW_DESC rtv);
		void create_srv(Handle h, Resource* resource, D3D12_SHADER_RESOURCE_VIEW_DESC srv);
		void create_uav(Handle h, Resource* resource, D3D12_UNORDERED_ACCESS_VIEW_DESC uav, Resource* counter = nullptr);
		void create_cbv(Handle h, Resource* resource, D3D12_CONSTANT_BUFFER_VIEW_DESC cbv);
		void create_dsv(Handle h, Resource* resource, D3D12_DEPTH_STENCIL_VIEW_DESC dsv);

	};


}