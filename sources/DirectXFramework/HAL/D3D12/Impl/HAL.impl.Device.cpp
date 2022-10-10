module Graphics:Device;

import HAL;
import d3d12;
import Math;
import Utils;


namespace HAL
{
	void Device::init(DeviceDesc& desc)
	{
		D3D12CreateDevice(
			desc.adapter->native_adapter.Get(),
			D3D_FEATURE_LEVEL_12_1,
			IID_PPV_ARGS(&native_device)
		);

		if (!native_device) return;

		for (auto type : magic_enum::enum_values<DescriptorHeapType>())
		{
			if (type != DescriptorHeapType::__GENERATE_OPS__) // TODO: ooooooooooooooooooooops
				descriptor_sizes[type] = native_device->GetDescriptorHandleIncrementSize(to_native(type));
		}


		D3D12_FEATURE_DATA_D3D12_OPTIONS options = {};
		D3D12_FEATURE_DATA_D3D12_OPTIONS5 options5 = {};
		D3D12_FEATURE_DATA_D3D12_OPTIONS7 options7 = {};
		D3D12_FEATURE_DATA_SHADER_MODEL supportedShaderModel = { D3D_SHADER_MODEL_6_7 };

		TEST(*this, native_device->CheckFeatureSupport(D3D12_FEATURE::D3D12_FEATURE_D3D12_OPTIONS7, &options7, sizeof(options7)));
		TEST(*this, native_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options5, sizeof(options5)));
		TEST(*this, native_device->CheckFeatureSupport(D3D12_FEATURE::D3D12_FEATURE_D3D12_OPTIONS, &options, sizeof(options)));
		TEST(*this, native_device->CheckFeatureSupport(D3D12_FEATURE::D3D12_FEATURE_SHADER_MODEL, &supportedShaderModel, sizeof(supportedShaderModel)));

		auto &properties = static_cast<Graphics::Device*>(this)->properties;
		properties.rtx = options5.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED;
		properties.full_bindless = supportedShaderModel.HighestShaderModel >= D3D_SHADER_MODEL_6_6;
		properties.mesh_shader = options7.MeshShaderTier >= D3D12_MESH_SHADER_TIER::D3D12_MESH_SHADER_TIER_1;
	}
}