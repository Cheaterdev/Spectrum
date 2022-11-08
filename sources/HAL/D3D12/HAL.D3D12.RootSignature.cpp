module HAL:API.RootSignature;

import Core;
import d3d12;
import :Types;
import :Sampler;

import :RootSignature;
import :API.Device;
import :Utils;


namespace HAL
{

	void init_parameters(std::vector<CD3DX12_ROOT_PARAMETER1>& parameters, std::list<CD3DX12_DESCRIPTOR_RANGE1>& ranges, const  RootSignatureDesc& desc)
	{
		for (auto e : desc.parameters)
		{
			parameters.emplace_back();
			auto& param = parameters.back();

			auto& v = e.second;

			std::visit(overloaded{
				[&](const DescriptorConstants& contants) {
					param.InitAsConstants(contants.count, contants.buffer_offset, 0, static_cast<D3D12_SHADER_VISIBILITY>(contants.visibility));
				},
				[&](const DescriptorTable& table) {
					ranges.emplace_back();
					auto& range = ranges.back();

					range.Init(static_cast<D3D12_DESCRIPTOR_RANGE_TYPE>(table.range), table.count, table.offset, table.space, D3D12_DESCRIPTOR_RANGE_FLAGS::D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);
					param.InitAsDescriptorTable(1, &range, static_cast<D3D12_SHADER_VISIBILITY>(table.visibility));

				},
				[&](const DescriptorConstBuffer& cbv) {
					param.InitAsConstantBufferView(cbv.offset, cbv.space, D3D12_ROOT_DESCRIPTOR_FLAGS::D3D12_ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE, static_cast<D3D12_SHADER_VISIBILITY>(cbv.visibility));
				},
				[&](const DescriptorSRV& srv) {
					param.InitAsShaderResourceView(srv.offset, srv.space, D3D12_ROOT_DESCRIPTOR_FLAGS::D3D12_ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE, static_cast<D3D12_SHADER_VISIBILITY>(srv.visibility));
				},
				[&](const DescriptorUAV& uav) {
					param.InitAsUnorderedAccessView(uav.offset, uav.space, D3D12_ROOT_DESCRIPTOR_FLAGS::D3D12_ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE, static_cast<D3D12_SHADER_VISIBILITY>(uav.visibility));

				},
				}, v);


		}

	}

	RootSignature::RootSignature(Device& device, const RootSignatureDesc& desc)
	{
		std::vector<CD3DX12_ROOT_PARAMETER1> parameters;
		std::list<CD3DX12_DESCRIPTOR_RANGE1> ranges;

		init_parameters(parameters, ranges, desc);

		std::vector<D3D12_STATIC_SAMPLER_DESC> descs;

		auto& samplers = desc.samplers();


		for (const auto& [position, desc] : samplers)
		{
			descs.emplace_back();
			auto& sampler = descs.back();
			sampler.AddressU = to_native(desc.AddressU);
			sampler.AddressV = to_native(desc.AddressV);
			sampler.AddressW = to_native(desc.AddressW);

			if (desc.BorderColor[0] == 0)
				sampler.BorderColor = desc.BorderColor[3] == 0 ? D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK : D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
			else
				sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;

			sampler.ComparisonFunc = to_native(desc.ComparisonFunc);
			sampler.Filter = to_native(desc.MinFilter, desc.MagFilter, desc.MipFilter, desc.ComparisonFunc);
			sampler.MaxAnisotropy = desc.MaxAnisotropy;
			sampler.MaxLOD = desc.MaxLOD;
			sampler.MinLOD = desc.MinLOD;
			sampler.MipLODBias = desc.MipLODBias;
			sampler.RegisterSpace = position.space;
			sampler.ShaderRegister = position.offset;

		}

		D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED;


		if (desc.type == RootSignatureType::Local)
		{
			flags = D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
		}
		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init_1_1(static_cast<UINT>(parameters.size()), parameters.data(), static_cast<UINT>(descs.size()), descs.data(), flags);
		this->desc = desc;

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		auto hr = (D3D12SerializeVersionedRootSignature(&rootSignatureDesc, &signature, &error));

		if (error)
		{
			Log::get().crash_error(static_cast<char*>(error->GetBufferPointer()));
		}

		TEST(device, hr);

		TEST(device, device.native_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
	}


	namespace API
	{
		D3D::RootSignature RootSignature::get_native()
		{
			return m_rootSignature;
		}
	}
	
}