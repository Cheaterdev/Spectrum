#include "pch.h"
#include "RootSignature.h"
#include "Device12.h"

namespace DX12
{

    void init_parameters(std::vector<CD3DX12_ROOT_PARAMETER>& parameters, std::list<CD3DX12_DESCRIPTOR_RANGE>& ranges,const  RootSignatureDesc& desc)
    {
		for (auto e : desc.parameters)
		{
            parameters.emplace_back();
			auto& param = parameters.back();

			MyVariant v = e.second;

			auto contants = v.try_get<DescriptorConstants>();
			auto table = v.try_get<DescriptorTable>();
			auto cbv = v.try_get<DescriptorConstBuffer>();
			auto srv = v.try_get<DescriptorSRV>();
			auto uav = v.try_get<DescriptorUAV>();


			if (contants)
			{
				param.InitAsConstants(contants->count, contants->buffer_offset, 0, static_cast<D3D12_SHADER_VISIBILITY>(contants->visibility));
			}

			if (cbv)
			{
				param.InitAsConstantBufferView(cbv->offset, cbv->space, static_cast<D3D12_SHADER_VISIBILITY>(cbv->visibility));
			}

			if (srv)
			{
				param.InitAsShaderResourceView(srv->offset, srv->space, static_cast<D3D12_SHADER_VISIBILITY>(srv->visibility));
			}

			if (uav)
			{
				param.InitAsUnorderedAccessView(uav->offset, uav->space, static_cast<D3D12_SHADER_VISIBILITY>(uav->visibility));
			}

			if (table)
			{
				ranges.emplace_back();
				auto& range = ranges.back();

				range.Init(static_cast<D3D12_DESCRIPTOR_RANGE_TYPE>(table->range), table->count, table->offset, table->space);
				param.InitAsDescriptorTable(1, &range, static_cast<D3D12_SHADER_VISIBILITY>(table->visibility));
			}

			}

    }

    RootSignature::RootSignature(std::initializer_list<DescriptorTable> table)
    {
        UINT i = 0;

        for (auto && e : table)
        {
            desc[i++] = e;
        }

        finalize();
    }

    void  RootSignature::finalize()
    {


      //  desc.parameters[]
        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;

        std::vector<CD3DX12_ROOT_PARAMETER> parameters;
		std::list<CD3DX12_DESCRIPTOR_RANGE> ranges;

        init_parameters(parameters, ranges, desc);
        rootSignatureDesc.Init(static_cast<UINT>(parameters.size()), parameters.data(), 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	
        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        TEST(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
        TEST(Device::get().get_native_device()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
    }

    RootSignature::RootSignature(const RootSignatureDesc& desc, D3D12_ROOT_SIGNATURE_FLAGS flags)
    {
		std::vector<CD3DX12_ROOT_PARAMETER> parameters;
		std::list<CD3DX12_DESCRIPTOR_RANGE> ranges;

		init_parameters(parameters, ranges, desc);


        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init(static_cast<UINT>(parameters.size()), parameters.data(), 0, nullptr, flags);
        rootSignatureDesc.pStaticSamplers = desc.samplers().data();
        rootSignatureDesc.NumStaticSamplers = static_cast<UINT>(desc.samplers().size());
        this->desc = desc;


		
        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        TEST(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));

        if (error)
        {
            Log::get().crash_error(static_cast<char*>(error->GetBufferPointer()));
        }

        TEST(Device::get().get_native_device()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
    }

    ComPtr<ID3D12RootSignature> RootSignature::get_native()
    {
        return m_rootSignature;
    }
}