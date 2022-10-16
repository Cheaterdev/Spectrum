export module HAL:API.Device;
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

	protected:
		void init(DeviceDesc& desc);
		virtual ~Device() = default;


	public:
		using ptr = std::shared_ptr<Device>;
	

	
	
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
		TEST(*this, native_device->QueryInterface(IID_PPV_ARGS(&pDred)));

		D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT DredAutoBreadcrumbsOutput = {};
		D3D12_DRED_PAGE_FAULT_OUTPUT DredPageFaultOutput = {};
		TEST(*this, pDred->GetAutoBreadcrumbsOutput(&DredAutoBreadcrumbsOutput));
		TEST(*this, pDred->GetPageFaultAllocationOutput(&DredPageFaultOutput));

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