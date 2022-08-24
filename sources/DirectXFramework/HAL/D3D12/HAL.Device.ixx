export module HAL.Device;
import d3d12;
import Math;
import Utils;

import HAL.Types;
import HAL.Sampler;

import D3D12.Utils;

using namespace HAL;


export namespace HAL
{
	
	struct DeviceDesc
	{
		D3D::Adapter adapter;
	};
	class Device
	{		

	public:
		using ptr = std::shared_ptr<Device>;
		Device(DeviceDesc &desc);


	public:
		D3D::Device native_device;
	};
}

module:private;


namespace HAL
{
	Device::Device(DeviceDesc& desc)
	{
		D3D12CreateDevice(
			desc.adapter.Get(),
			D3D_FEATURE_LEVEL_12_1,
			IID_PPV_ARGS(&native_device)
		);
	}
}