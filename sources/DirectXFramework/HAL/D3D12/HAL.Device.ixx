export module HAL:Device;
import d3d12;
import Math;
import Utils;

import :Types;
import :Sampler;
import :Utils;

using namespace HAL;


export namespace HAL
{

	struct DeviceDesc
	{
		DXGI::Adapter adapter;
	};
	class Device
	{

	public:
		using ptr = std::shared_ptr<Device>;
		Device(DeviceDesc& desc);
		virtual ~Device()
		{
		}

	public:
		D3D::Device native_device;
		enum_array<DescriptorHeapType, uint> descriptor_sizes;


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


	};
}

//module:private;


namespace HAL
{
	Device::Device(DeviceDesc& desc)
	{
		D3D12CreateDevice(
			desc.adapter.Get(),
			D3D_FEATURE_LEVEL_12_1,
			IID_PPV_ARGS(&native_device)
		);

		for (auto type : magic_enum::enum_values<DescriptorHeapType>())
		{
			descriptor_sizes[type] = native_device->GetDescriptorHandleIncrementSize(to_native(type));
		}
	}
}