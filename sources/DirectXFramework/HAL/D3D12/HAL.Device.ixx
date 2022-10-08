export module HAL:Device;
import d3d12;
import Math;
import Utils;

import :Types;
import :Sampler;
import :Utils;
import :Adapter;

using namespace HAL;


export namespace HAL
{

	struct DeviceDesc
	{
		HAL::Adapter::ptr adapter;
	};

	struct DeviceProperties
	{
		std::string name;
		bool rtx = false;
		bool mesh_shader = false;
		bool full_bindless = false;
	};
	class Device
	{
		DeviceProperties properties;
	public:
		using ptr = std::shared_ptr<Device>;
		Device(DeviceDesc& desc);

		const DeviceProperties& get_properties() const
		{
			return properties;
		}
		virtual ~Device()
		{
		}

	public:
		D3D::Device native_device;
		enum_array<DescriptorHeapType, uint> descriptor_sizes;
		void process_result(HRESULT hr, std::string_view line) const;
	private:
		void dump_dred();
	public:
		uint get_descriptor_size(DescriptorHeapType type) const
		{
			return descriptor_sizes[type];
		}

		uint Subresources(const ResourceDesc& desc) const
		{
			if (desc.is_buffer())
				return 1;

			auto texture_desc = desc.as_texture();
			uint count = D3D12GetFormatPlaneCount(native_device.Get(), to_native(texture_desc.Format));
			return texture_desc.MipLevels * texture_desc.ArraySize * count;
		}

		size_t get_vram();
	};
}

//module:private;


namespace HAL
{
	Device::Device(DeviceDesc& desc)
	{
		D3D12CreateDevice(
			desc.adapter->native_adapter.Get(),
			D3D_FEATURE_LEVEL_12_1,
			IID_PPV_ARGS(&native_device)
		);

		if (!native_device) return;

		for (auto type : magic_enum::enum_values<DescriptorHeapType>())
		{
			if(type!= DescriptorHeapType::__GENERATE_OPS__) // TODO: ooooooooooooooooooooops
			descriptor_sizes[type] = native_device->GetDescriptorHandleIncrementSize(to_native(type));
		}


		D3D12_FEATURE_DATA_D3D12_OPTIONS options = {};
		D3D12_FEATURE_DATA_D3D12_OPTIONS5 options5 = {};
		D3D12_FEATURE_DATA_D3D12_OPTIONS7 options7 = {};
		D3D12_FEATURE_DATA_SHADER_MODEL supportedShaderModel = { D3D_SHADER_MODEL_6_7 };

		TEST(this,native_device->CheckFeatureSupport(D3D12_FEATURE::D3D12_FEATURE_D3D12_OPTIONS7, &options7, sizeof(options7)));
		TEST(this, native_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options5, sizeof(options5)));
		TEST(this, native_device->CheckFeatureSupport(D3D12_FEATURE::D3D12_FEATURE_D3D12_OPTIONS, &options, sizeof(options)));
		TEST(this, native_device->CheckFeatureSupport(D3D12_FEATURE::D3D12_FEATURE_SHADER_MODEL, &supportedShaderModel, sizeof(supportedShaderModel)));

	
		properties.rtx = options5.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED;	
		properties.full_bindless = supportedShaderModel.HighestShaderModel >= D3D_SHADER_MODEL_6_6;
		properties.mesh_shader = options7.MeshShaderTier >= D3D12_MESH_SHADER_TIER::D3D12_MESH_SHADER_TIER_1;
	}

	size_t Device::get_vram()
	{
			DXGI_QUERY_VIDEO_MEMORY_INFO videoMemoryInfo;
			//	adapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &videoMemoryInfo);
			size_t usedVRAM = videoMemoryInfo.CurrentUsage / 1024 / 1024;
			return usedVRAM;
	}

	void Device::dump_dred()
	{
		ComPtr<ID3D12DeviceRemovedExtendedData>  pDred;
		TEST(this, native_device->QueryInterface(IID_PPV_ARGS(&pDred)));

		D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT DredAutoBreadcrumbsOutput = {};
		D3D12_DRED_PAGE_FAULT_OUTPUT DredPageFaultOutput = {};
		TEST(this, pDred->GetAutoBreadcrumbsOutput(&DredAutoBreadcrumbsOutput));
		TEST(this, pDred->GetPageFaultAllocationOutput(&DredPageFaultOutput));

		auto parse_node = [](const D3D12_AUTO_BREADCRUMB_NODE& node)
		{
			Log::get() << node << Log::endl;
		};


		auto node = DredAutoBreadcrumbsOutput.pHeadAutoBreadcrumbNode;

		while (node)
		{
			parse_node(*node);
			node = node->pNext;
		}

	}



	void Device::process_result(HRESULT hr, std::string_view line) const
	{
		//if (hr == 0x887a0005) device_fail();

		std::string message = std::system_category().message(hr);

		if (FAILED(hr))
			Log::get().crash_error(hr, line);

		if (FAILED(hr))
		{
			__debugbreak();
			assert(false);
		}
	
	}

}